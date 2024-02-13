#!/usr/bin/env sh

# Statically compile nnn with netbsd-curses, musl-fts and musl libc on Ubuntu
#
# netbsd-curses: https://github.com/sabotage-linux/netbsd-curses
# musl-fts: https://github.com/void-linux/musl-fts
# musl libc: https://www.musl-libc.org/
#
# Dependencies: git
#
# Usage: musl-static-ubuntu.sh [no_run]
#        # optional argument - do not to execute the binary after compilation
#
# Notes:
#   - run the script within the top-level nnn directory
#   - installs musl & gits netbsd-curses, musl-fts libs
#
# Tested on Ubuntu 20.04 x86_64
# Author: Arun Prakash Jana

# Exit on first failure
set -e

# Output binary name
BIN=nnn-musl-static

# Install musl
sudo apt install -y --no-install-recommends musl musl-dev musl-tools

# Get netbsd-curses
[ ! -d "./netbsd-curses" ] && git clone https://github.com/sabotage-linux/netbsd-curses

# Enter the library dir
cd netbsd-curses

# Get the last known working version
git checkout v0.3.2

# Compile the static netbsd-curses libraries
if [ ! -d "./libs" ]; then
    mkdir libs
else
    rm -vf -- libs/*
fi
make CC=musl-gcc CFLAGS=-O3 LDFLAGS=-static all-static -j$(($(nproc)+1))
cp -v libcurses/libcurses.a libterminfo/libterminfo.a libs/

# Get musl-fts library
cd ..
[ ! -d "./musl-fts" ] && git clone https://github.com/void-linux/musl-fts --depth=1

# Compile the static musl-fts library
cd musl-fts
./bootstrap.sh
./configure
make CC=musl-gcc CFLAGS=-O3 LDFLAGS=-static -j$(($(nproc)+1))

# Compile nnn
cd ..
[ -e "./netbsd-curses" ] || rm -- "$BIN"
musl-gcc -O3 -DNORL -DNOMOUSE -std=c11 -Wall -Wextra -Wshadow -I./netbsd-curses/libcurses -I./musl-fts -o "$BIN" src/nnn.c -Wl,-Bsymbolic-functions -lpthread -L./netbsd-curses/libs -lcurses -lterminfo -static -L./musl-fts/.libs -lfts
strip "$BIN"

if [ -z "$1" ]; then
    # Run the binary with it selected
    ./"$BIN" -d "$BIN"
fi
