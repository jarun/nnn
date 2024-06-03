# Run nnn with dynamic changing directory to the environment.
#
# $env.XDG_CONFIG_HOME sets the home folder for `nnn` folder and its $env.NNN_TMPFILE variable.
# See manual NNN(1) for more information.
#
# Import module using `use quitcd.nu n` to have `n` command in your context.
export def --env n [
	...args : string # Extra flags to launch nnn with.
	--selective = false # Change directory only when exiting via ^G.
] -> nothing {

	# The behaviour is set to cd on quit (nnn checks if $env.NNN_TMPFILE is set).
	# Hard-coded to its respective behaviour in `nnn` source-code.
	let nnn_tmpfile = $env
		| default '~/.config/' 'XDG_CONFIG_HOME'
		| get 'XDG_CONFIG_HOME'
		| path join 'nnn/.lastd'
		| path expand

	# Launch nnn. Add desired flags after `^nnn`, ex: `^nnn -eda ...$args`,
	# or make an alias `alias n = n -eda`.
	if $selective {
		^nnn ...$args
	} else {
		NNN_TMPFILE=$nnn_tmpfile ^nnn ...$args
	}

	if ($nnn_tmpfile | path exists) {
		# Remove <cd '> from the first part of the string and the last single quote <'>.
		# Fix post-processing of nnn's given path that escapes its single quotes with POSIX syntax.
		let path = open $nnn_tmpfile
			| str replace --all --regex `^cd '|'$` ``
			| str replace --all `'\''` `'`

		^rm -- $nnn_tmpfile

		cd $path
	}
}
