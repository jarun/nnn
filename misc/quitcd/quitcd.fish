export NNN_TMPFILE="/tmp/nnn"

function n --description 'support nnn quit and change directory'
        nnn $argv[1]

        if test -e $NNN_TMPFILE
                . $NNN_TMPFILE
                rm $NNN_TMPFILE
        end
end
