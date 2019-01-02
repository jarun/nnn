# Rename this file to match the name of the function
# e.g. ~/.config/fish/functions/n.fish
# or, add the lines to the 'config.fish' file.

export NNN_TMPFILE="/tmp/nnn"

function n --description 'support nnn quit and change directory'
        nnn $argv

        if test -e $NNN_TMPFILE
                source $NNN_TMPFILE
                rm $NNN_TMPFILE
        end
end
