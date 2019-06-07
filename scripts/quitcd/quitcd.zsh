n()
{
    nnn "$@"

    NNN_TMPFILE=~/.config/nnn/.lastd

    if [ -f $NNN_TMPFILE ]; then
            . $NNN_TMPFILE
            rm $NNN_TMPFILE
    fi
}
