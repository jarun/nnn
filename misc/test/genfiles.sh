#!/bin/sh

# Generates 100000 files in the current directory

i=1; while [ $i -le 100000 ]; do
    mktemp -p . -t 'XXXXXXXXXXXXXXXXXXXXX'
    i=$(( i + 1 ))
done
