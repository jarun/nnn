export NNN_TMPFILE="/tmp/nnn"

n()
{
        if [ -n "$1" ]; then
                nnn "$1"
        else
                nnn
        fi

        if [ -f $NNN_TMPFILE ]; then
                . $NNN_TMPFILE
                rm $NNN_TMPFILE
        fi
}
