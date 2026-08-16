#!/bin/sh

# Interactively configure nnn's quitcd helper and common beginner options.
# Optionally install plugins and configure nuke as the opener.
#
# Shell: POSIX compliant
# Author: Arun Prakash Jana

REPO_URL='https://raw.githubusercontent.com/jarun/nnn/master'
TMPFILE=''

cleanup()
{
    status=$?
    [ -z "$TMPFILE" ] || rm -f -- "$TMPFILE"
    return "$status"
}

trap cleanup EXIT HUP INT TERM

check_nnn()
{
    if ! command -v nnn >/dev/null 2>&1; then
        printf 'nnn is not installed or is not available on PATH.\n' >&2
        printf 'Installation: https://github.com/jarun/nnn/wiki/Usage#installation\n' >&2
        exit 1
    fi
}

ask_yes_no()
{
    prompt=$1

    while :; do
        printf '%s [y/N] ' "$prompt"
        IFS= read -r answer || exit 1
        case $answer in
            y|Y|yes|YES|Yes) return 0 ;;
            ''|n|N|no|NO|No) return 1 ;;
            *) printf 'Please answer y or n.\n' ;;
        esac
    done
}

print_configuration_help()
{
    cat <<'EOF'

Optional environment variables (add selected exports to your shell rc file):
  NNN_BMS      Key-to-directory bookmarks, e.g. 'd:$HOME/Documents'
  NNN_PLUG     Key-to-plugin pairs, e.g. 'o:fzopen;v:imgview'
  NNN_OPENER   Custom file opener; useful for CLI-only setups
  NNN_COLORS   Context colours, e.g. '1234'
  NNN_FCOLORS  File-type colours for xterm-256color terminals
  NNN_TRASH    Custom trash command instead of rm -rf
  NNN_ARCHIVE  Extra archive filename extensions
  NNN_FIFO     FIFO path for live previews or integrations
  NNN_SEL      Custom selection file path

See https://github.com/jarun/nnn/wiki/Usage#configuration for details.

EOF
}

detect_shell()
{
    shell_name=$(ps -p "$PPID" -o comm= 2>/dev/null)
    shell_name=${shell_name##*/}

    if [ -z "$shell_name" ]; then
        shell_name=${SHELL##*/}
    fi

    case $shell_name in
        ash|dash|ksh|mksh|yash) shell_name='sh' ;;
        nu) shell_name='nushell' ;;
    esac

    printf 'Detected shell: %s\n' "$shell_name"

    case $shell_name in
        bash)
            helper='quitcd.bash_sh_zsh'
            rc_file=${BASHRC:-"$HOME/.bashrc"}
            format='bash'
            ;;
        sh)
            helper='quitcd.bash_sh_zsh'
            rc_file="$HOME/.profile"
            format='posix'
            ;;
        zsh)
            helper='quitcd.bash_sh_zsh'
            rc_file="${ZDOTDIR:-$HOME}/.zshrc"
            format='zsh'
            ;;
        fish)
            helper='quitcd.fish'
            rc_file="${XDG_CONFIG_HOME:-$HOME/.config}/fish/config.fish"
            format='fish'
            ;;
        csh)
            helper='quitcd.csh'
            rc_file="$HOME/.cshrc"
            format='csh'
            ;;
        tcsh)
            helper='quitcd.csh'
            rc_file="$HOME/.tcshrc"
            format='csh'
            ;;
        elvish)
            helper='quitcd.elv'
            rc_file="$HOME/.elvish/rc.elv"
            format='elvish'
            ;;
        nushell|nu)
            helper='quitcd.nu'
            rc_file="${XDG_CONFIG_HOME:-$HOME/.config}/nushell/config.nu"
            format='nushell'
            ;;
        *)
            printf 'Unsupported shell: %s\n' "$shell_name" >&2
            exit 1
            ;;
    esac
}

add_option()
{
    option=$1
    description=$2

    if ask_yes_no "$option: $description"; then
        options="$options $option"
    fi
}

function_exists()
{
    [ -f "$rc_file" ] || return 1

    case $format in
        bash|posix|zsh)
            grep -Eq "^[[:space:]]*${function_name}[[:space:]]*\\(\\)" "$rc_file"
            ;;
        fish)
            grep -Eq "^[[:space:]]*function[[:space:]]+$function_name([[:space:]]|$)" "$rc_file"
            ;;
        csh)
            grep -Eq "^[[:space:]]*alias[[:space:]]+$function_name([[:space:]]|$)" "$rc_file"
            ;;
        elvish)
            grep -Eq "^[[:space:]]*fn[[:space:]]+$function_name([[:space:]]|$)" "$rc_file"
            ;;
        nushell)
            grep -Eq "^[[:space:]]*export[[:space:]]+def([[:space:]]+--env)?[[:space:]]+$function_name([[:space:]]|$)" "$rc_file"
            ;;
    esac
}

select_function_name()
{
    while :; do
        printf 'Function name [n]: '
        IFS= read -r function_name || exit 1
        function_name=${function_name:-n}

        case $function_name in
            [!A-Za-z_]*|*[!A-Za-z0-9_]* )
                printf 'Use a name containing only letters, numbers, and underscores.\n'
                continue
                ;;
        esac

        if function_exists; then
            if ask_yes_no "Function $function_name already exists in $rc_file. Append another definition?"; then
                return
            fi
            continue
        fi
        return
    done
}

download_helper()
{
    TMPFILE=$(mktemp "${TMPDIR:-/tmp}/nnn-quitcd.XXXXXX") || exit 1

    if command -v curl >/dev/null 2>&1; then
        curl --connect-timeout 10 --max-time 60 -fsSL "$REPO_URL/misc/quitcd/$helper" > "$TMPFILE"
    elif command -v wget >/dev/null 2>&1; then
        wget --timeout=60 -qO "$TMPFILE" "$REPO_URL/misc/quitcd/$helper"
    else
        printf 'curl or wget is required to download the %s helper.\n' "$shell_name" >&2
        exit 1
    fi

    if [ ! -s "$TMPFILE" ]; then
        printf 'Could not download %s from the nnn repository.\n' "$helper" >&2
        exit 1
    fi
}

configure_helper()
{
    case $format in
        bash|posix|zsh)
            sed "s|command nnn \"\$@\"|command nnn$options \"\$@\"|" "$TMPFILE"
            ;;
        fish)
            sed "s|command nnn \$argv|command nnn$options \$argv|" "$TMPFILE"
            ;;
        csh)
            sed "s|\\\\nnn; source|\\\\nnn$options; source|" "$TMPFILE"
            ;;
        elvish)
            sed "s|e:nnn \$@a|e:nnn$options \$@a|" "$TMPFILE"
            ;;
        nushell)
            sed "s|\^nnn \.\.\.\$args|^nnn$options ...\$args|g" "$TMPFILE"
            ;;
    esac
}

rename_function()
{
    case $format in
        bash|posix|zsh)
            sed "s|^n ()|$function_name ()|";
            ;;
        fish)
            sed "s|^function n\\([[:space:]]\\)|function $function_name\\1|";
            ;;
        csh)
            sed "s|^alias n\\([[:space:]]\\)|alias $function_name\\1|";
            ;;
        elvish)
            sed "s|^fn n |fn $function_name |";
            ;;
        nushell)
            sed "s|^export def --env n |export def --env $function_name |";
            ;;
    esac
}

configure_nuke()
{
    if [ "$use_nuke" -eq 0 ]; then
        cat
        return
    fi

    case $format in
        bash|posix|zsh)
            # shellcheck disable=SC2016
            sed '/^[[:space:]]*export NNN_TMPFILE=/a\    export NNN_OPENER="${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins/nuke"'
            ;;
        fish)
            # shellcheck disable=SC2016
            sed '/^function /a\    set -x NNN_OPENER (path join (or $XDG_CONFIG_HOME "$HOME/.config") nnn/plugins/nuke)'
            ;;
        csh)
            # shellcheck disable=SC2016
            sed '/^set NNN_TMPFILE=/a\setenv NNN_OPENER "$HOME/.config/nnn/plugins/nuke"'
            ;;
        elvish)
            # shellcheck disable=SC2016
            sed '/^fn /a\    set-env NNN_OPENER (path:join (or $E:XDG_CONFIG_HOME (path:join $E:HOME .config)) nnn/plugins/nuke)'
            ;;
        nushell)
            # shellcheck disable=SC2016
            sed '/^export def --env /a\    let-env NNN_OPENER = ($env.XDG_CONFIG_HOME | default ($env.HOME | path join .config) | path join nnn/plugins/nuke)'
            ;;
    esac
}

install_plugins()
{
    if ! ask_yes_no "$(printf '\n%s' 'Install (or update) all nnn plugins now?')"; then
        return
    fi

    if command -v curl >/dev/null 2>&1; then
        if sh -c "$(curl --connect-timeout 10 --max-time 60 -fsSL https://raw.githubusercontent.com/jarun/nnn/master/plugins/getplugs)"; then
            if ask_yes_no 'Set NNN_OPENER to the installed nuke plugin?'; then
                use_nuke=1
                printf 'The generated function will use nuke to open files.\n'
            fi
        fi
    else
        printf 'Plugin installation requires curl.\n' >&2
    fi
}

check_nnn
detect_shell

options=''
select_function_name

if ask_yes_no "$(printf '\n%s' 'Open text files in the terminal editor (-e)?')"; then
    options=' -e'
    editor=${VISUAL:-${EDITOR:-vi}}
    printf 'Text files will be opened with: %s\n' "$editor"
fi

printf '\nChoose any additional beginner-friendly options.\n'
add_option '-d' 'Start in detail mode (permissions, size, and time)'
add_option '-D' 'Show directories in their context colour'
add_option '-H' 'Show hidden files by default'
add_option '-n' 'Type to navigate (filter mode always on)'
add_option '-Q' 'Quit without a confirmation prompt'
add_option '-R' 'Stop at the first and last entry instead of wrapping around'

use_nuke=0
install_plugins

download_helper
configured_helper=$(configure_helper | rename_function | configure_nuke) || exit 1

printf '\nGenerated nnn function with:%s\n' "${options:- no option selected}"
print_configuration_help

if ask_yes_no "Append this function to $rc_file? It changes directory on quit."; then
    mkdir -p "$(dirname "$rc_file")" || exit 1
    {
        printf '\n# nnn quitcd helper generated by misc/quitcd/setup.sh\n'
        printf '%s\n' "$configured_helper"
    } >> "$rc_file" || exit 1
    printf 'Appended the nnn function to %s\n' "$rc_file"
else
    printf '\n# >>> nnn quitcd helper >>>\n%s\n# <<< nnn quitcd helper <<<\n' "$configured_helper"
fi

cat <<'EOF'

Useful links:
    Usage, Config: https://github.com/jarun/nnn/wiki/Usage
    Plugins: https://github.com/jarun/nnn/tree/master/plugins
    Wiki: https://github.com/jarun/nnn/wiki
    Use cases: https://github.com/jarun/nnn/wiki/Basic-use-cases#the-nnn-magic
    Advanced: https://github.com/jarun/nnn/wiki/Advanced-use-cases
    Live Previews: https://github.com/jarun/nnn/wiki/Live-previews
    Troubleshooting: https://github.com/jarun/nnn/wiki/Troubleshooting
EOF