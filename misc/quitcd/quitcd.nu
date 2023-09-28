# The behaviour is set to cd on quit (nnn checks if NNN_TMPFILE is set)
let cfgHome = ($env | default $"($env.HOME)/.config" XDG_CONFIG_HOME | get XDG_CONFIG_HOME)
let-env NNN_TMPFILE = $"($cfgHome)/nnn/.lastd"

def-env n [...x] {
  # Launch nnn. Add desired flags after `^nnn`, ex: `^nnn -eda ($x | str join)`
  ^nnn ($x | str join)
  let newpath = (
    if ($env.NNN_TMPFILE | path exists) {
      let rawpath = (open $env.NNN_TMPFILE | parse --regex 'cd (?P<dir>.+)').0.dir
      let newpath = ($rawpath | $"echo ($in)" | /bin/bash -s)
      ^rm -f $env.NNN_TMPFILE
      echo $newpath
    } else {
      pwd
    }
  )
  cd $newpath
}
