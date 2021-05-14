#!/usr/bin/env sh

# Description: Independent POSIX-compliant GUI application launcher.
#              Fuzzy find executables in $PATH and launch an application.
#              stdin, stdout, stderr are suppressed so CLI tools exit silently.
#
#              To configure launch as an independent app launcher add a keybind
#              to open launch in a terminal e.g.,
#
#              xfce4-terminal -e "${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins/launch
#
# Dependencies: fzf
#
# Usage: launch [delay]
#        delay is in seconds, if omitted launch waits for 1 sec
#
# Integration with nnn: launch is installed with other plugins, nnn picks it up.
#
# Shell: POSIX compliant
# Author: Arun Prakash Jana

# shellcheck disable=SC2086

IFS=':'

get_selection() {
    if type fzf >/dev/null 2>&1; then
        { IFS=':'; ls -H $PATH; } | sort | fzf
    else
        exit 1
    fi
}

if selection=$( get_selection ); then
    setsid "$selection" 2>/dev/null 1>/dev/null &

    if [ -n "$1" ]; then
        sleep "$1"
    else
        sleep 1
    fi
fi
