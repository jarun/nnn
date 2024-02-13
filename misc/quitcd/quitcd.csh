# NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'

# The behaviour is set to cd on quit (nnn checks if NNN_TMPFILE is set)
# If NNN_TMPFILE is set to a custom path, it must be exported for nnn to see.
# To cd on quit only on ^G, set NNN_TMPFILE after the nnn invocation, and make
# sure not to use a custom path.
set NNN_TMPFILE=~/.config/nnn/.lastd

# Unmask ^Q (, ^V etc.) (if required, see `stty -a`) to Quit nnn
# stty start undef
# stty stop undef
# stty lwrap undef
# stty lnext undef

# The backslash allows one to alias n to nnn if desired without making an
# infinitely recursive alias
alias n '\nnn; source "$NNN_TMPFILE"; rm -f -- "$NNN_TMPFILE"'
