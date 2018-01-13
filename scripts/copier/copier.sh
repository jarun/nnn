#!/bin/sh

# comment the next line to convert newlines to spaces
IFS=
echo -n $1 | `xsel --clipboard --input`
