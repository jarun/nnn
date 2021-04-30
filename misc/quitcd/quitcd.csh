# NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'

# The default behaviour is to cd on quit (nnn checks if NNN_TMPFILE is set)
# To cd on quit only on ^G, export NNN_TMPFILE after the call to nnn
# NOTE: NNN_TMPFILE is fixed, should not be modified
set NNN_TMPFILE=~/.config/nnn/.lastd

# Unmask ^Q (, ^V etc.) (if required, see `stty -a`) to Quit nnn
# stty start undef
# stty stop undef
# stty lwrap undef
# stty lnext undef

alias n 'nnn; source "$NNN_TMPFILE"; rm -f "$NNN_TMPFILE"'
