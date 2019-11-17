# Rename this file to match the name of the function
# e.g. ~/.config/fish/functions/n.fish
# or, add the lines to the 'config.fish' file.

function n --description 'support nnn quit and change directory'
    # Block nesting of nnn in subshells
    if [ (expr $NNNLVL + 0) -ge 1 ]
        echo "nnn is already running"
        return
    end

    # The default behaviour is to cd on quit (nnn checks if NNN_TMPFILE is set)
    # To cd on quit only on ^G, export NNN_TMPFILE after the call to nnn
    if test -n "$XDG_CONFIG_HOME"
        set -x NNN_TMPFILE "$XDG_CONFIG_HOME/nnn/.lastd"
    else
        set -x NNN_TMPFILE "$HOME/.config/nnn/.lastd"
    end

    nnn $argv

    if test -e $NNN_TMPFILE
        source $NNN_TMPFILE
        rm $NNN_TMPFILE
    end
end
