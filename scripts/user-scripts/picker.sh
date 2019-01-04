#!/usr/bin/env sh

# Description: Pick files and pipe the line-separated list to another utility
#
# Shell: sh
# Author: Arun Prakash Jana
#
# Usage:
# Copy this file in your $PATH, make it executable and preferably name it to picker.
# Run commands like:
#   ls -l `picker`
#   cd `picker`
# or, in fish shell:
#   ls -l (picker)
#   cd (picker)
#
# NOTE: This use case is limited to picking files, other functionality may not work as expected.

nnn -p /tmp/picked
cat /tmp/picked | tr '\0' '\n'
rm /tmp/picked
