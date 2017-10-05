n()
{
        export NNN_TMPFILE="$(mktemp -u nnn.XXXXXXXX)"

        nnn "$@"

        if [ -f $NNN_TMPFILE ]; then
                . $NNN_TMPFILE
                rm $NNN_TMPFILE
        fi
}
