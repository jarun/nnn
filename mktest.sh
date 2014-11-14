#!/bin/sh

# Create test files and directories

test -e test && {
    echo "Remove test and try again"
    exit 1
}

mkdir test && cd test

echo 'It works!' > normal.txt
echo 'Με δουλέβει;' > 'κοινό.txt'
ln -s normal.txt ln-normal.txt
ln -s normal.txt ln-normal
mkdir normal-dir
ln -s normal-dir ln-normal-dir
ln -s nowhere ln-nowhere
mkfifo mk-fifo
touch no-access && chmod 000 no-access
ln -s ../normal.txt normal-dir/ln-normal.txt
ln -s ../normal.txt normal-dir/ln-normal
