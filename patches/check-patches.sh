#!/bin/bash
#
# Usage: ./misc/test/check-patches.sh
#
# Bash script that checks for any of the patches failing to apply.
# Read patches/README.md for more information.

export PATCH_OPTS="--merge"
patches=("O_COLEMAK" "O_GITSTATUS" "O_NAMEFIRST" "O_RESTOREPREVIEW")
z=$(( 1 << ${#patches[@]} ))
pid=$$
ret=0
trap 'ret=1' SIGUSR1

for ((n=1; n < z; ++n)); do
    for ((i=0; i < ${#patches[@]}; ++i)); do
        printf "%s=%d " "${patches[$i]}" "$(( (n & (1 << i)) != 0 ))"
    done | tee "/dev/stderr" | (
        make clean -s
        if ! xargs make 2>&1; then
            echo "[FAILED]" >&2
            kill -SIGUSR1 "$pid"
        else
            echo "[SUCCESS]" >&2
        fi
        git restore src
    ) >/dev/null
done
exit "$ret"
