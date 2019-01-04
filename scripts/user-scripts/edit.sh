#!/usr/bin/env sh

# Description: Fuzzy find a file in directory subtree with fzy and edit in vim
#
# Shell: generic
# Author: Arun Prakash Jana

# bash, zsh
vim $(find -type f | fzy)

# fish
# vim (find -type f | fzy)
