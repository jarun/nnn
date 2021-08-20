This is a fork of [jarun/nnn](https://github.com/jarun/nnn) to support compiling on macOS 10.11 thanks to [mach_gettime.c](https://gist.github.com/alfwatt/3588c5aa1f7a1ef7a3bb) by [Alf Watt](github.com/alfwatt).

Other than that, it's my playground for learning to use git and github.

The "fork" is in the `macos-10.11` branch, all the rest is from upstream.

The Makefile only compiles in the replacement `clock_gettime` if `sw_vers` returns `Mac OS X` `10.11.x`.
