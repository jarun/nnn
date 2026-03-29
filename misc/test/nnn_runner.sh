#!/bin/sh
#
# POSIX test wrapper for nnn (no tmux dependency)
#
# Runs nnn inside a pty (via the `script` command) and feeds it keystrokes
# from an input file, one input per line. Captures terminal output for
# verification.
#
# Dependencies: script (from util-linux / POSIX), mkfifo
#
# Usage:
#   ./misc/test/nnn_runner.sh [OPTIONS] <input-file> [nnn-args ...]
#
# Options:
#   -e <path>      Path to nnn binary (default: ./nnn)
#   -d <dir>       Working directory for nnn (default: temporary dir)
#   -o <dir>       Output directory for snapshots (default: /tmp/nnn-test.<pid>)
#   -t <ms>        Delay in milliseconds between inputs (default: 200)
#   -v             Verbose: print each input as it is sent
#   -h             Show this help
#
# Input file format:
#   One input per line. Blank lines and lines starting with '#' are skipped.
#   Lines starting with '@' are directives:
#     @sleep <ms>         Extra delay in milliseconds
#     @snapshot <label>   Capture a named snapshot
#     @assert <string>    Fail if <string> is NOT in terminal output (full history)
#     @refute <string>    Fail if <string> IS in output since last @mark
#     @mark               Set checkpoint; subsequent @refute checks only output
#                         produced after this point
#
#   Key encoding:
#     >text         Send literal text (no newline appended)
#     Enter         Send carriage return (\r)
#     Escape        Send ESC (\033)
#     Tab           Send tab (\t)
#     Space         Send space
#     Up            Send arrow up    (\033[A)
#     Down          Send arrow down  (\033[B)
#     Right         Send arrow right (\033[C)
#     Left          Send arrow left  (\033[D)
#     BSpace        Send backspace (\177)
#     C-<x>         Send Ctrl+<x>  (e.g. C-a sends \001)
#     Any other single character is sent as-is (e.g. j, k, q, /, d)
#
# Example input file:
#   # Navigate down and open a file
#   j
#   j
#   Enter
#   @sleep 500
#   @snapshot after-open
#   q
#
# Exit codes:
#   0   All assertions passed
#   1   Usage error / missing dependencies
#   2   Assertion failure

set -e

# ---------- defaults ----------
NNN_BIN="./nnn"
WORK_DIR=""
OUT_DIR=""
DELAY_MS=200
VERBOSE=0
PASS_COUNT=0
FAIL_COUNT=0

# ---------- helpers ----------
usage() {
    sed -n '2,/^$/{ s/^#//; s/^ //; p; }' "$0"
    exit 1
}

die() {
    printf "error: %s\n" "$1" >&2
    exit 1
}

cleanup() {
    # Kill nnn/script if still running
    if [ -n "$SCRIPT_PID" ] && kill -0 "$SCRIPT_PID" 2>/dev/null; then
        kill "$SCRIPT_PID" 2>/dev/null || true
        wait "$SCRIPT_PID" 2>/dev/null || true
    fi
    # Close the input pipe fd
    exec 3>&- 2>/dev/null || true
    # Remove temporary working directory
    if [ -n "$TMPWORK" ] && [ -d "$TMPWORK" ]; then
        rm -rf "$TMPWORK"
    fi
    # Remove the FIFO
    if [ -n "$INPUT_FIFO" ] && [ -p "$INPUT_FIFO" ]; then
        rm -f "$INPUT_FIFO"
    fi
}

msleep() {
    # Portable millisecond sleep: try fractional seconds, fall back to 1s.
    _s=$(( $1 / 1000 ))
    _ms=$(( $1 % 1000 ))
    if [ "$_ms" -gt 0 ]; then
        sleep "${_s}.$(printf '%03d' "$_ms")" 2>/dev/null || sleep $(( _s + 1 ))
    elif [ "$_s" -gt 0 ]; then
        sleep "$_s"
    fi
}

# Map a key name to a raw byte sequence
map_key() {
    case "$1" in
        Enter)      printf '\r' ;;
        Escape)     printf '\033' ;;
        Tab)        printf '\t' ;;
        Space)      printf ' ' ;;
        BSpace)     printf '\177' ;;
        Up)         printf '\033[A' ;;
        Down)       printf '\033[B' ;;
        Right)      printf '\033[C' ;;
        Left)       printf '\033[D' ;;
        Home)       printf '\033[H' ;;
        End)        printf '\033[F' ;;
        PgUp)       printf '\033[5~' ;;
        PgDn)       printf '\033[6~' ;;
        Delete)     printf '\033[3~' ;;
        Insert)     printf '\033[2~' ;;
        F1)         printf '\033OP' ;;
        F2)         printf '\033OQ' ;;
        F3)         printf '\033OR' ;;
        F4)         printf '\033OS' ;;
        F5)         printf '\033[15~' ;;
        F6)         printf '\033[17~' ;;
        F7)         printf '\033[18~' ;;
        F8)         printf '\033[19~' ;;
        F9)         printf '\033[20~' ;;
        F10)        printf '\033[21~' ;;
        F11)        printf '\033[23~' ;;
        F12)        printf '\033[24~' ;;
        C-?)
            # Ctrl+letter: convert to control character (1-26 for a-z)
            _letter="$(printf '%s' "$1" | cut -c3)"
            _upper="$(printf '%s' "$_letter" | tr 'a-z' 'A-Z')"
            _ord=$(printf '%d' "'$_upper")
            _ctrl=$(( _ord - 64 ))
            printf "\\$(printf '%03o' "$_ctrl")"
            ;;
        ?)
            # Single character — send as-is
            printf '%s' "$1"
            ;;
        *)
            # Multi-char: send as literal keystrokes
            printf '%s' "$1"
            ;;
    esac
}

# Send raw bytes into the nnn pty via the FIFO
send_key() {
    map_key "$1" >&3
}

send_literal() {
    printf '%s' "$1" >&3
}

# Strip ANSI escape sequences and control chars to get plain content
strip_ansi() {
    # Use printf to get a literal ESC character for portable sed
    _esc="$(printf '\033')"
    _cr="$(printf '\r')"
    _bs="$(printf '\010')"
    # 1) CSI sequences:  ESC [ <params> <letter>
    # 2) OSC sequences:  ESC ] ... (until ESC or end)
    # 3) Charset select: ESC ( <char>, ESC ) <char>
    # 4) Other 2-byte:   ESC <char> (for =, >, <, etc.)
    # 5) Lone ESC
    # 6) char + backspace pairs (tty echo artifacts)
    # 7) Carriage returns
    sed "s/${_esc}\[[^a-zA-Z]*[a-zA-Z]//g
         s/${_esc}\][^${_esc}]*//g
         s/${_esc}[()].\{0,1\}//g
         s/${_esc}.//g
         s/${_esc}//g
         s/.${_bs}//g
         s/${_cr}//g"
}

# Get a clean snapshot of the current terminal output (full history)
get_screen() {
    if [ -f "$TYPESCRIPT" ]; then
        tail -c 16384 "$TYPESCRIPT" | strip_ansi
    fi
}

# Get only the terminal output produced since the last checkpoint
# This is what assertions run against, so they test the current screen.
LAST_OFFSET=0
get_recent_screen() {
    if [ -f "$TYPESCRIPT" ]; then
        _size="$(wc -c < "$TYPESCRIPT")"
        _new=$(( _size - LAST_OFFSET ))
        if [ "$_new" -gt 0 ]; then
            tail -c "$_new" "$TYPESCRIPT" | strip_ansi
        fi
    fi
}

# Advance the checkpoint to the current end of the typescript
advance_offset() {
    if [ -f "$TYPESCRIPT" ]; then
        LAST_OFFSET="$(wc -c < "$TYPESCRIPT")"
    fi
}

snapshot() {
    _label="$1"
    _file="${OUT_DIR}/${_label}.txt"
    get_screen > "$_file"
    [ "$VERBOSE" -eq 1 ] && printf "  snapshot: %s\n" "$_file"
}

assert_screen() {
    _pattern="$1"
    _content="$(get_screen)"
    if printf '%s' "$_content" | grep -qF "$_pattern"; then
        PASS_COUNT=$(( PASS_COUNT + 1 ))
        [ "$VERBOSE" -eq 1 ] && printf "  PASS: assert '%s'\n" "$_pattern"
    else
        FAIL_COUNT=$(( FAIL_COUNT + 1 ))
        printf "  FAIL: expected '%s' on screen\n" "$_pattern" >&2
        printf "  --- screen (last 2000 chars) ---\n" >&2
        printf '%s' "$_content" | tail -c 2000 >&2
        printf "\n  --- end ---\n" >&2
    fi
}

refute_screen() {
    _pattern="$1"
    _content="$(get_recent_screen)"
    if printf '%s' "$_content" | grep -qF "$_pattern"; then
        FAIL_COUNT=$(( FAIL_COUNT + 1 ))
        printf "  FAIL: unexpected '%s' on screen\n" "$_pattern" >&2
        printf "  --- screen (last 2000 chars) ---\n" >&2
        printf '%s' "$_content" | tail -c 2000 >&2
        printf "\n  --- end ---\n" >&2
    else
        PASS_COUNT=$(( PASS_COUNT + 1 ))
        [ "$VERBOSE" -eq 1 ] && printf "  PASS: refute '%s'\n" "$_pattern"
    fi
}

# ---------- parse options ----------
while getopts "e:d:o:t:vh" opt; do
    case "$opt" in
        e) NNN_BIN="$OPTARG" ;;
        d) WORK_DIR="$OPTARG" ;;
        o) OUT_DIR="$OPTARG" ;;
        t) DELAY_MS="$OPTARG" ;;
        v) VERBOSE=1 ;;
        h) usage ;;
        *) usage ;;
    esac
done
shift $(( OPTIND - 1 ))

INPUT_FILE="${1:?input file required}"
shift
NNN_EXTRA_ARGS="$*"

[ -f "$INPUT_FILE" ] || die "input file not found: $INPUT_FILE"
command -v script >/dev/null 2>&1 || die "'script' command is required but not found"

# Resolve nnn binary
case "$NNN_BIN" in
    /*) ;;  # absolute
    *)  NNN_BIN="$(cd "$(dirname "$NNN_BIN")" && pwd)/$(basename "$NNN_BIN")" ;;
esac
[ -x "$NNN_BIN" ] || die "nnn binary not found or not executable: $NNN_BIN"

# ---------- set up environment ----------
TMPWORK=""
if [ -z "$WORK_DIR" ]; then
    TMPWORK="$(mktemp -d "${TMPDIR:-/tmp}/nnn-test-work.XXXXXX")"
    WORK_DIR="$TMPWORK"
    # populate with a few test entries
    mkdir -p "$WORK_DIR/dir1" "$WORK_DIR/dir2"
    printf 'hello\n' > "$WORK_DIR/file1.txt"
    printf 'world\n' > "$WORK_DIR/file2.txt"
fi

if [ -z "$OUT_DIR" ]; then
    OUT_DIR="/tmp/nnn-test.$$"
fi
mkdir -p "$OUT_DIR"

TYPESCRIPT="${OUT_DIR}/typescript.raw"
INPUT_FIFO="${OUT_DIR}/input.fifo"
SCRIPT_PID=""

trap cleanup EXIT INT TERM

# Create a FIFO for feeding input to nnn
rm -f "$INPUT_FIFO"
mkfifo "$INPUT_FIFO"

# ---------- start nnn inside script(1) pty ----------
# The `script` command allocates a real pty so nnn's isatty() check passes.
# We feed keystrokes through the FIFO connected to script's stdin.
# Terminal output is captured to the typescript file.

# Detect script flavour (GNU coreutils vs BSD)
if script --version 2>&1 | grep -q 'util-linux'; then
    # GNU/Linux: script -q -f -c <cmd> <outfile>
    script -q -f -c "cd '${WORK_DIR}' && exec '${NNN_BIN}' ${NNN_EXTRA_ARGS}" "$TYPESCRIPT" \
        < "$INPUT_FIFO" > /dev/null 2>&1 &
else
    # BSD / macOS: script -q -F <outfile> <cmd>  (-F = flush on BSD)
    script -q -F "$TYPESCRIPT" sh -c "cd '${WORK_DIR}' && exec '${NNN_BIN}' ${NNN_EXTRA_ARGS}" \
        < "$INPUT_FIFO" > /dev/null 2>&1 &
fi
SCRIPT_PID=$!

# Open the FIFO for writing on fd 3 (keeps the pipe open so nnn doesn't get EOF)
exec 3>"$INPUT_FIFO"

# Give nnn time to start and render its first frame
msleep 500

[ "$VERBOSE" -eq 1 ] && printf "nnn started (pid %s)\n" "$SCRIPT_PID"

# Take an initial snapshot
snapshot "00-init"

# ---------- process input file ----------
STEP=1
while IFS= read -r line || [ -n "$line" ]; do
    # Skip blank lines and comments
    case "$line" in
        ""|\#*) continue ;;
    esac

    PADSTEP="$(printf '%02d' "$STEP")"

    # Handle directives
    case "$line" in
        @sleep\ *)
            _val="${line#@sleep }"
            [ "$VERBOSE" -eq 1 ] && printf "[%s] sleep %s ms\n" "$PADSTEP" "$_val"
            msleep "$_val"
            ;;
        @snapshot\ *)
            _label="${line#@snapshot }"
            [ "$VERBOSE" -eq 1 ] && printf "[%s] snapshot '%s'\n" "$PADSTEP" "$_label"
            snapshot "${PADSTEP}-${_label}"
            ;;
        @assert\ *)
            _pat="${line#@assert }"
            [ "$VERBOSE" -eq 1 ] && printf "[%s] assert '%s'\n" "$PADSTEP" "$_pat"
            assert_screen "$_pat"
            ;;
        @refute\ *)
            _pat="${line#@refute }"
            [ "$VERBOSE" -eq 1 ] && printf "[%s] refute '%s'\n" "$PADSTEP" "$_pat"
            refute_screen "$_pat"
            ;;
        @mark)
            [ "$VERBOSE" -eq 1 ] && printf "[%s] mark offset\n" "$PADSTEP"
            advance_offset
            ;;
        \>*)
            # Literal text (strip leading '>')
            _text="${line#>}"
            [ "$VERBOSE" -eq 1 ] && printf "[%s] literal: %s\n" "$PADSTEP" "$_text"
            send_literal "$_text"
            msleep "$DELAY_MS"
            ;;
        *)
            # Key name — translate and send
            [ "$VERBOSE" -eq 1 ] && printf "[%s] key: %s\n" "$PADSTEP" "$line"
            send_key "$line"
            msleep "$DELAY_MS"
            ;;
    esac

    # Auto-snapshot after each step
    snapshot "${PADSTEP}-auto"

    STEP=$(( STEP + 1 ))
done < "$INPUT_FILE"

# Final snapshot
snapshot "final"

# Close the input fd — this causes nnn to see EOF and exit
exec 3>&-

# Wait for script/nnn to finish (with a timeout)
_waited=0
while kill -0 "$SCRIPT_PID" 2>/dev/null && [ "$_waited" -lt 3 ]; do
    msleep 500
    _waited=$(( _waited + 1 ))
done
if kill -0 "$SCRIPT_PID" 2>/dev/null; then
    kill "$SCRIPT_PID" 2>/dev/null || true
fi
wait "$SCRIPT_PID" 2>/dev/null || true
SCRIPT_PID=""

# ---------- summary ----------
TOTAL=$(( PASS_COUNT + FAIL_COUNT ))
if [ "$TOTAL" -gt 0 ]; then
    printf "\n=== Results: %d passed, %d failed (of %d) ===\n" \
        "$PASS_COUNT" "$FAIL_COUNT" "$TOTAL"
fi

printf "Snapshots saved to: %s\n" "$OUT_DIR"
printf "Raw typescript: %s\n" "$TYPESCRIPT"

if [ "$FAIL_COUNT" -gt 0 ]; then
    exit 2
fi

exit 0
