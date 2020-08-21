#!/bin/sh
#
# Usage: ./misc/test/benchmark.sh ./nnn /tmp/testdir1 ./testdir2 ...
#
# Don't forget to build nnn in benchmark mode: make O_BENCH=1

# Use a test dir filled with genfiles.sh to get interesting output
# (or maybe /usr/lib/)

LANG=C

TIME_VAL=${TIME_VAL:-"real"}

SAMPLES=${SAMPLES:-100}

EXE=$1

bench_val () {
    (time "$1" "$2") 2>&1 |\
    awk '$1=="'"$TIME_VAL"'"{match($2, /[0-9]*\.[0-9]*/) ; print substr($2, RSTART, RLENGTH)}'
}

bench_dir () {
    i=$SAMPLES
    printf "$2"
    while [ $((i--)) -gt 0 ] ; do
        printf "\t%s" "$(bench_val "$1" "$2")"
    done
    printf "\n"
}

shift

for dir in "$@" ; do
    bench_dir "$EXE" "$dir"
done

