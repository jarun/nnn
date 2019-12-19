#
# Rudimentary Bash completion definition for nnn.
#
# Author:
#   Arun Prakash Jana <engineerarun@gmail.com>
#

_nnn ()
{
    COMPREPLY=()
    local IFS=$'\n'
    local cur=$2 prev=$3
    local -a opts
    opts=(
        -a
        -b
        -c
        -d
        -E
        -g
        -H
        -i
        -K
        -n
        -o
        -p
        -Q
        -r
        -R
        -s
        -S
        -t
        -v
        -x
        -h
    )
    if [[ $prev == -b ]]; then
        local bookmarks=$(echo $NNN_BMS | awk -F: -v RS=\; '{print $1}')
        COMPREPLY=( $(compgen -W "$bookmarks" -- "$cur") )
    elif [[ $prev == -p ]]; then
        COMPREPLY=( $(compgen -f -d -- "$cur") )
    elif [[ $prev == -s ]]; then
        local sessions_dir=${XDG_CONFIG_HOME:-$HOME/.config}/nnn/sessions
        COMPREPLY=( $(cd "$sessions_dir" && compgen -f -d -- "$cur") )
    elif [[ $cur == -* ]]; then
        COMPREPLY=( $(compgen -W "${opts[*]}" -- "$cur") )
    else
        COMPREPLY=( $(compgen -f -d -- "$cur") )
    fi
}

complete -o filenames -F _nnn nnn
