#
# Fish completion definition for nnn.
#
# Author:
#   Arun Prakash Jana <engineerarun@gmail.com>
#

if test -n "$XDG_CONFIG_HOME"
    set sessions_dir $XDG_CONFIG_HOME/.config/nnn/sessions
else
    set sessions_dir $HOME/.config/nnn/sessions
end

complete -c nnn -s a    -d 'use access time'
complete -c nnn -s b -r -d 'bookmark key to open' -x -a '(echo $NNN_BMS | awk -F: -v RS=\; \'{print $1"\t"$2}\')'
complete -c nnn -s c    -d 'cli-only opener'
complete -c nnn -s d    -d 'start in detail mode'
complete -c nnn -s e -r -d 'load session by name' -x -a '@\t"last session" (ls $sessions_dir)'
complete -c nnn -s f    -d 'run filter as cmd on prompt key'
complete -c nnn -s H    -d 'show hidden files'
complete -c nnn -s i    -d 'start in navigate-as-you-type mode'
complete -c nnn -s K    -d 'detect key collision'
complete -c nnn -s n    -d 'use version compare to sort files'
complete -c nnn -s o    -d 'open files only on Enter'
complete -c nnn -s p -r -d 'copy selection to file' -a '-\tstdout'
complete -c nnn -s r    -d 'show cp, mv progress (Linux-only)'
complete -c nnn -s s    -d 'use substring match for filters'
complete -c nnn -s S    -d 'start in disk usage analyzer mode'
complete -c nnn -s t    -d 'disable dir auto-select'
complete -c nnn -s v    -d 'show program version and exit'
complete -c nnn -s h    -d 'show program help'
