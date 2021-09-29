# NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'

# The behaviour is set to cd on quit (nnn checks if NNN_TMPFILE is set)
# To cd on quit only on ^G, set NNN_TMPFILE after nnn invocation
# A custom path can also be set e.g. set NNN_TMPFILE=/tmp/.lastd
set NNN_TMPFILE=~/.config/nnn/.lastd

# Unmask ^Q (, ^V etc.) (if required, see `stty -a`) to Quit nnn
# stty start undef
# stty stop undef
# stty lwrap undef
# stty lnext undef

alias n 'nnn; source "$NNN_TMPFILE"; rm -f "$NNN_TMPFILE"'
