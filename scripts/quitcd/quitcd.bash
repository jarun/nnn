n()
{
    nnn "$@"

    NNN_TMPFILE=~/.config/nnn/.lastd

    if [ -f $NNN_TMPFILE ]; then
            . $NNN_TMPFILE
            rm -f $NNN_TMPFILE > /dev/null
    fi
}
