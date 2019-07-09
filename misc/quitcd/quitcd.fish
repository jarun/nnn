# Rename this file to match the name of the function
# e.g. ~/.config/fish/functions/n.fish
# or, add the lines to the 'config.fish' file.

function n --description 'support nnn quit and change directory'
    # NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'
    set -x NNN_TMPFILE ~/.config/nnn/.lastd

    nnn $argv

    if test -e $NNN_TMPFILE
            source $NNN_TMPFILE
            rm $NNN_TMPFILE
    end
end
