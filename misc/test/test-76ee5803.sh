#!/bin/sh
#
# Test for commit 76ee5803: "Take care of other shell escape issues"
#
# This commit fixes quoting of temp file paths (%s -> '%s') in shell
# commands used by batch_rename() and cpmv_rename().  The bug only
# manifests when the temp-file directory contains spaces.  Since nnn
# prefers /tmp (which never has spaces), we use Linux user+mount
# namespaces (unshare -mr) to make /tmp inaccessible, forcing nnn's
# set_tmp_path() to fall back to $TMPDIR — which we point at a path
# that contains spaces.
#
# Requirements: Linux with user-namespace support, unshare(1)
#
# Strategy:
#   1. Create $TMPDIR with spaces; make /tmp inaccessible via namespace
#   2. Create test files and trigger batch rename (r -> c)
#   3. A mock EDITOR renames "aaa.txt" -> "zzz.txt" in the temp file
#   4. Verify the rename happened on the filesystem
#
# Expected: PASS with the fix, FAIL without (the unquoted paste/sed
# commands choke on the spaced temp path).

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
NNN_TEST="$SCRIPT_DIR/nnn_runner.sh"

# Locate nnn binary
NNN_BIN="${1:-./nnn}"
case "$NNN_BIN" in
    /*) ;;
    *)  NNN_BIN="$(cd "$(dirname "$NNN_BIN")" && pwd)/$(basename "$NNN_BIN")" ;;
esac

[ -x "$NNN_BIN" ] || { echo "error: nnn not found at $NNN_BIN" >&2; exit 1; }

# ---------- check for namespace support ----------
if ! unshare -mr true 2>/dev/null; then
    printf "SKIP: user/mount namespaces not available\n"
    exit 0
fi

# ---------- set up test environment ----------
# All temp data lives under TESTROOT.  We place it under $HOME rather
# than /tmp because /tmp will be masked inside the namespace.
TESTROOT="$(mktemp -d "${HOME}/nnn-shell-esc.XXXXXX")"
trap 'rm -rf "$TESTROOT"' EXIT

# TMPDIR for nnn — path deliberately contains a space
SPACE_TMP="$TESTROOT/tmp dir"
mkdir -p "$SPACE_TMP"

# NNN_SEL — also under a path with spaces
SELDIR="$TESTROOT/path with spaces"
mkdir -p "$SELDIR"

# Clean XDG_CONFIG_HOME so the .nmv plugin is NOT found,
# forcing nnn to use the built-in batch_rename()
FAKECFG="$TESTROOT/config"
mkdir -p "$FAKECFG/nnn/plugins"

# Working directory with test files
WORKDIR="$TESTROOT/workdir"
mkdir -p "$WORKDIR"
printf 'content_a\n' > "$WORKDIR/aaa.txt"
printf 'content_b\n' > "$WORKDIR/bbb.txt"

# Mock EDITOR: renames aaa.txt -> zzz.txt in the batch-rename temp file
MOCK_EDITOR="$TESTROOT/mock-editor.sh"
cat > "$MOCK_EDITOR" << 'EDEOF'
#!/bin/sh
sed -i 's/^aaa\.txt$/zzz.txt/' "$1"
EDEOF
chmod +x "$MOCK_EDITOR"

# nnn-test input file
INPUT_FILE="$TESTROOT/test-input.txt"
cat > "$INPUT_FILE" << 'INPUTEOF'
# Wait for nnn to render
@sleep 300
@assert aaa.txt
@assert bbb.txt

# Trigger batch rename: r key then c (current dir)
r
@sleep 300
c
@sleep 800

# After rename, nnn should refresh
@sleep 500
@mark
@snapshot after-rename

# Quit
q
INPUTEOF

# ---------- inner script that runs inside the namespace ----------
# Two levels of namespace nesting:
#   Outer (unshare -mr): gives root to mount a mode-000 tmpfs on /tmp
#   Inner (unshare --user): drops back to nobody — /tmp is now blocked,
#     but TESTROOT dirs (owned by the real user) map to nobody and remain
#     writable. nnn's xdiraccess("/tmp") fails → falls back to $TMPDIR.
INNER="$TESTROOT/inner.sh"
cat > "$INNER" << INNEREOF
#!/bin/sh
set -e

# Make /tmp inaccessible so nnn falls back to \$TMPDIR
mount -t tmpfs -o size=1M,mode=000 tmpfs /tmp

# Run the actual test in a nested user namespace (drops to nobody)
exec unshare --user sh -c '
  export TMPDIR="$SPACE_TMP"
  export NNN_SEL="$SELDIR/.selection"
  export XDG_CONFIG_HOME="$FAKECFG"
  export EDITOR="$MOCK_EDITOR"
  export VISUAL="$MOCK_EDITOR"

  OUT_DIR="$TESTROOT/output"
  "$NNN_TEST" -v -e "$NNN_BIN" -d "$WORKDIR" -o "\$OUT_DIR" "$INPUT_FILE"
'
INNEREOF
chmod +x "$INNER"

# ---------- run test ----------
printf "Testing commit 76ee5803: shell escape with spaced TMPDIR...\n"
printf "  NNN binary: %s\n" "$NNN_BIN"
printf "  TMPDIR:     %s\n" "$SPACE_TMP"
printf "  WORKDIR:    %s\n" "$WORKDIR"

unshare -mr "$INNER"
TEST_EXIT=$?

# ---------- verify filesystem ----------
printf "\n--- Filesystem verification ---\n"
PASS=0
FAIL=0

if [ -f "$WORKDIR/zzz.txt" ]; then
    printf "  PASS: zzz.txt exists (renamed from aaa.txt)\n"
    PASS=$((PASS + 1))
else
    printf "  FAIL: zzz.txt not found (batch rename failed)\n"
    FAIL=$((FAIL + 1))
fi

if [ ! -f "$WORKDIR/aaa.txt" ]; then
    printf "  PASS: aaa.txt no longer exists\n"
    PASS=$((PASS + 1))
else
    printf "  FAIL: aaa.txt still exists (rename didn't happen)\n"
    FAIL=$((FAIL + 1))
fi

if [ -f "$WORKDIR/zzz.txt" ]; then
    CONTENT="$(cat "$WORKDIR/zzz.txt")"
    if [ "$CONTENT" = "content_a" ]; then
        printf "  PASS: zzz.txt has correct content\n"
        PASS=$((PASS + 1))
    else
        printf "  FAIL: zzz.txt has wrong content: %s\n" "$CONTENT"
        FAIL=$((FAIL + 1))
    fi
fi

if [ -f "$WORKDIR/bbb.txt" ]; then
    printf "  PASS: bbb.txt unchanged\n"
    PASS=$((PASS + 1))
else
    printf "  FAIL: bbb.txt missing\n"
    FAIL=$((FAIL + 1))
fi

printf "\n=== Shell escape test: %d passed, %d failed ===\n" "$PASS" "$FAIL"

if [ "$FAIL" -gt 0 ] || [ "$TEST_EXIT" -ne 0 ]; then
    printf "Snapshots: %s/output\n" "$TESTROOT"
    ls -la "$WORKDIR"/
    exit 2
fi

exit 0
