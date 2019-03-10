#!/usr/bin/env sh

# Description: Fuzzy find a file in directory subtree with fzy and open using xdg-open
#
# Shell: generic
# Author: Arun Prakash Jana

# bash, zsh
xdg-open $(find -type f | fzy) >/dev/null 2>&1

# fish
# xdg-open (find -type f | fzy) >/dev/null 2>&1
