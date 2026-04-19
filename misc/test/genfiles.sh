#!/bin/bash

# Generates 100000 files in the current directory

i=1
while [ $i -le 100000 ]; do
    # Use mktemp with proper options for cross-platform compatibility
    # Linux: mktemp -t creates a temp file in TMPDIR with the given prefix
    # macOS: mktemp -t uses the template for the filename in TMPDIR
    # For better portability, use mktemp without -p and -t on Linux
    if ! mktemp .XXXXXXXXXXXXXXX > /dev/null 2>&1; then
        echo "Warning: mktemp failed at iteration $i" >&2
        i=$(( i + 1 ))
        continue
    fi
    i=$(( i + 1 ))
done
