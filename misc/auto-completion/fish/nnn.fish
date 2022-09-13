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

complete -c nnn -s a    -d 'auto-create NNN_FIFO'
complete -c nnn -s A    -d 'disable dir auto-enter'
complete -c nnn -s b -r -d 'bookmark key to open' -x -a '(echo $NNN_BMS | awk -F: -v RS=\; \'{print $1"\t"$2}\')'
complete -c nnn -s B    -d 'use bsdtar for archives'
complete -c nnn -s c    -d 'cli-only opener'
complete -c nnn -s C    -d 'color by context'
complete -c nnn -s d    -d 'start in detail mode'
complete -c nnn -s D    -d 'dirs in context color'
complete -c nnn -s e    -d 'open text files in $VISUAL/$EDITOR/vi'
complete -c nnn -s E    -d 'use EDITOR for undetached edits'
complete -c nnn -s f    -d 'use readline history file'
complete -c nnn -s g    -d 'regex filters'
complete -c nnn -s H    -d 'show hidden files'
complete -c nnn -s i    -d 'show current file info'
complete -c nnn -s J    -d 'no auto-advance on selection'
complete -c nnn -s K    -d 'detect key collision and exit'
complete -c nnn -s l -r -d 'lines to move per scroll'
complete -c nnn -s n    -d 'start in type-to-nav mode'
complete -c nnn -s o    -d 'open files only on Enter'
complete -c nnn -s p -r -d 'copy selection to file' -a '-\tstdout'
complete -c nnn -s P -r -d 'plugin key to run' -x -a '(echo $NNN_PLUG | awk -F: -v RS=\; \'{print $1"\t"$2}\')'
complete -c nnn -s Q    -d 'disable quit confirmation'
complete -c nnn -s r    -d 'show cp, mv progress (Linux-only)'
complete -c nnn -s R    -d 'disable rollover at edges'
complete -c nnn -s s -r -d 'load session by name' -x -a '@\t"last session" (ls $sessions_dir)'
complete -c nnn -s S    -d 'persistent session'
complete -c nnn -s t -r -d 'timeout in seconds to lock'
complete -c nnn -s T -r -d 'a d e r s t v'
complete -c nnn -s u    -d 'use selection (no prompt)'
complete -c nnn -s U    -d 'show user and group'
complete -c nnn -s V    -d 'show program version and exit'
complete -c nnn -s x    -d 'notis, sel to system clipboard, xterm title'
complete -c nnn -s h    -d 'show program help'
