#!/usr/bin/env sh

# Statically compile nnn with netbsd-curses and musl libc on Ubuntu
#
# netbsd-curses: https://github.com/sabotage-linux/netbsd-curses
# musl libc: https://www.musl-libc.org/
#
# Notes:
#   - Run the script within the top-level nnn directory
#   - installs musl and downloads netbsd-curses library
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
[ ! -d "./netbsd-curses" ] && git clone https://github.com/sabotage-linux/netbsd-curses --depth=1

# Compile the static libraries
cd netbsd-curses
[ ! -d "./libs" ] && mkdir libs || rm -vf libs/*
make CC=musl-gcc CFLAGS=-O3 LDFLAGS=-static all-static -j$((`nproc`+1))
cp -v libcurses/libcurses.a libterminfo/libterminfo.a libs/

# Compile nnn
cd ..
[ -e "./netbsd-curses" ] || rm "$BIN"
musl-gcc -O3 -DNORL -DNOMOUSE -std=c11 -Wall -Wextra -Wshadow -I./netbsd-curses/libcurses -o "$BIN" src/nnn.c -Wl,-Bsymbolic-functions -L./netbsd-curses/libs -lcurses -lterminfo -static
strip "$BIN"

# Run the binary with it selected
./"$BIN" -d "$BIN"
