#!/bin/bash

# Description: Pick files and pipe the line-separated list to another utility
#
# Shell: bash
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

nnn -p /tmp/pickerout
> /tmp/picked
while read -d $'\0' line ; do
    echo $line >> /tmp/picked
done < /tmp/pickerout
echo $line >> /tmp/picked
cat /tmp/picked

rm /tmp/pickerout /tmp/picked
