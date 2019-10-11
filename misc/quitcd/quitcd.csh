# NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'

# The default behaviour is to cd on quit (nnn checks if NNN_TMPFILE is set)
# To cd on quit only on ^G, export NNN_TMPFILE after the call to nnn
set NNN_TMPFILE=~/.config/nnn/.lastd

alias n 'nnn; source "$NNN_TMPFILE"; rm "$NNN_TMPFILE"'
