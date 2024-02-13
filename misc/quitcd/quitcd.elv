# Append this file to ~/.elvish/rc.elv (Elvish > 0.17.0)

use path

fn n {|@a|
	# Block nesting of nnn in subshells
	if (has-env NNNLVL) {
		try {
			if (>= $E:NNNLVL 1) {
				echo "nnn is already running"
				return
			}
		} catch e {
			nop
		}
	}

	# The behaviour is set to cd on quit (nnn checks if NNN_TMPFILE is set)
  # If NNN_TMPFILE is set to a custom path, it must be exported for nnn to
  # see.
  if (has-env XDG_CONFIG_HOME) {
		set-env NNN_TMPFILE $E:XDG_CONFIG_HOME/nnn/.lastd
	} else {
		set-env NNN_TMPFILE $E:HOME/.config/nnn/.lastd
	}

	# Unmask ^Q (, ^V etc.) (if required, see `stty -a`) to Quit nnn
  # stty start undef
  # stty stop undef
  # stty lwrap undef
  # stty lnext undef

	# The e: prefix allows one to alias n to nnn if desired without making an
	# infinitely recursive alias
	e:nnn $@a

	if (path:is-regular $E:NNN_TMPFILE) {
		eval (slurp < $E:NNN_TMPFILE)
		rm -- $E:NNN_TMPFILE
	}
}
