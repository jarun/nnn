#!/usr/bin/env sh

# Description: Edit file as superuser
#
# Shell: POSIX compliant
# Author: Anna Arad

EDITOR="${EDITOR:-vim}"

if type sudo >/dev/null 2>&1; then
    sudo -E "$EDITOR" "$1"
elif type sudoedit >/dev/null 2>&1; then
    sudoedit -E "$1"
elif type doas >/dev/null 2>&1; then
    doas "$EDITOR" "$1"
fi
