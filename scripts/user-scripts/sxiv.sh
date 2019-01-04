#!/usr/bin/env sh

# Description: Open images in current directory in sxiv
#
# Shell: generic
# Author: Arun Prakash Jana

sxiv -q * >/dev/null 2>&1 &
