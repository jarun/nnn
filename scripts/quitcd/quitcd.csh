# NOTE: set NNN_TMPFILE correctly if you use 'XDG_CONFIG_HOME'
set NNN_TMPFILE=~/.config/nnn/.lastd
alias n 'nnn; source "$NNN_TMPFILE"; rm "$NNN_TMPFILE"'
