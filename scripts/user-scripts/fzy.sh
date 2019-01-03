#!/usr/bin/env sh

# Description: Fuzzy find files in directory subtree with fzy and open using xdg-open
#
# Shell: generic
# Author: Arun Prakash Jana

xdg-open $(find -type f | fzy) >/dev/null 2>&1
