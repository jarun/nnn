#!/bin/sh

cat /path/to/.nnncp | xargs -0 | xsel -bi

# Termux
# cat /path/to/.nnncp | xargs -0 | termux-clipboard-set
# e.g.: cat /data/data/com.termux/files/home/.nnncp | xargs -0 | termux-clipboard-set
