export NNN_TMPFILE="/tmp/nnn"

n()
{
        nnn -d
        if [ -f $NNN_TMPFILE ]; then
                . $NNN_TMPFILE
                rm $NNN_TMPFILE
        fi
}
