export NNN_TMPFILE="/tmp/nnn"

n()
{
        if [ -n "$1" ]; then
                nnn -d "$1"
        else
                nnn -d
        fi

        if [ -f $NNN_TMPFILE ]; then
                . $NNN_TMPFILE
                rm $NNN_TMPFILE
        fi
}
