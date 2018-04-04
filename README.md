## nnn

Noice is Not Noice, a noicer fork...

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://aur.archlinux.org/packages/nnn"><img src="https://img.shields.io/aur/version/nnn.svg?maxAge=600" alt="AUR" /></a>
<a href="http://formulae.brew.sh/formula/nnn"><img src="https://img.shields.io/homebrew/v/nnn.svg?maxAge=600" alt="Homebrew" /></a>
<a href="https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/debian-10+-blue.svg?maxAge=2592000" alt="Debian Buster+" /></a>
<a href="https://apps.fedoraproject.org/packages/nnn"><img src="https://img.shields.io/badge/fedora-27+-blue.svg?maxAge=2592000" alt="Fedora 27+" /></a>
<a href="https://software.opensuse.org/package/nnn"><img src="https://img.shields.io/badge/opensuse%20leap-15.0+-blue.svg?maxAge=2592000" alt="openSUSE Leap 15.0+" /></a>
<a href="https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/ubuntu-17.10+-blue.svg?maxAge=2592000" alt="Ubuntu Artful+" /></a>
</p>

<p align="center">
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://travis-ci.org/jarun/nnn.svg?branch=master" alt="Build Status" /></a>
</p>

[![nnn screencast](https://s26.postimg.org/9pjzvc9g9/nnn_demo.jpg)](https://vimeo.com/233223942 "Click to see nnn in action!")

<p align="center"><i>nnn in action!</i></a></p>

`nnn` is probably the [fastest and most resource-sensitive](#performance) (with all its capabilities) file browser you have ever used. It's extremely flexible too - integrates with your DE and favourite GUI utilities, works with the desktop opener, supports bookmarks, has smart navigation shortcuts, [navigate-as-you-type](#navigate-as-you-type-mode) mode, disk usage analyzer mode, comprehensive file details and much more. `nnn` was initially forked from [noice](http://git.2f30.org/noice/) but is significantly [different](https://github.com/jarun/nnn/wiki/nnn-vs.-noice) today.

If you want to edit a file in vi with some soothing music in the background while referring to a spec in your GUI PDF viewer, `nnn` got it! [Quickstart](#quickstart) and see how `nnn` simplifies those long desktop sessions...

Have fun with it! Missing a feature? Want to contribute? Head to the rolling [ToDo list](https://github.com/jarun/nnn/issues/58).

*Love smart and efficient terminal utilities? Explore my repositories. Buy me a cup of coffee if they help you.*

<p align="center">
<a href="https://saythanks.io/to/jarun"><img src="https://img.shields.io/badge/say-thanks!-ff69b4.svg" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-green.svg" alt="Donate via PayPal!" /></a>
</p>

### Table of Contents

- [Features](#features)
- [Performance](#performance)
- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [From a package manager](#from-a-package-manager)
  - [Release packages](#release-packages)
  - [From source](#from-source)
- [Shell completion](#shell-completion)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Filters](#filters)
  - [Navigate-as-you-type mode](#navigate-as-you-type-mode)
  - [File type abbreviations](#file-type-abbreviations)
  - [File handling](#file-handling)
  - [Help](#help)
- [Quickstart](#quickstart)
- [How to](#how-to)
  - [add bookmarks](#add-bookmarks)
  - [use cd .....](#use-cd-)
  - [cd on quit](#cd-on-quit)
  - [copy file paths to clipboard](#copy-file-paths-to-clipboard)
  - [copy file paths when X is missing](#copy-file-paths-when-x-is-missing)
  - [run a custom script](#run-a-custom-script)
  - [change dir color](#change-dir-color)
  - [file copy, move, delete](#file-copy-move-delete)
  - [boost chdir prompt](#boost-chdir-prompt)
  - [work faster at rename prompt](#work-faster-at-rename-prompt)
  - [set idle timeout](#set-idle-timeout)
  - [show hot plugged drives](#show-hot-plugged-drives)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)

### Features

- Modes - basic, detail, disk usage analyzer (du)
- Navigation
  - Familiar, easy shortcuts (arrows, `~`, `-`, `&`)
  - *Navigate-as-you-type* mode for the maverick
  - Handy bookmarks, start at bookmark, pin and visit directory
  - chdir prompt with tab completion (interprets cd ..... too!)
  - Roll-over at edges, page through entries
  - Show directories in custom color (default: enabled in blue)
- Sorting
  - Directories always listed on top
  - Sort by file name, modification time, size
  - Numeric order (1, 2, ... 10, 11, ...) for numeric names
- Search
  - Superfast directory content filtering with *search-as-you-type*
  - Desktop search (gnome-search-tool, catfish) integration
- Mimes
  - Open with desktop opener (default) or specify a custom app
  - List and extract archives (needs atool)
  - Optionally open text files in EDITOR (fallback vi)
  - Customizable bash script [nlay](https://github.com/jarun/nnn/wiki/all-about-nlay) to handle actions
- Information
  - Detailed stat-like file information
  - Media information (needs mediainfo or exiftool, if specified)
- Convenience
  - Create, rename files and directories
  - Batch rename/move/delete current directory entries in vidir (from moreutils)
  - Spawn SHELL (fallback sh) in the current directory
  - Run a custom script in the current directory
  - Copy absolute file paths with/without X (*easy* shell integration)
  - Change directory at exit (*easy* shell integration)
  - Open any file in EDITOR (fallback vi) or PAGER (fallback less)
  - Open current directory in a custom GUI file browser
  - Terminal screensaver/locker (default vlock, customizable) integration
- Unicode support
- Highly optimized code, minimal resource usage

### Performance

`nnn` vs. ncdu memory usage in disk usage analyzer mode (401385 files on disk):

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
 5034 vaio      20   0   70360  58764   2244 S   0.0  0.7   0:00.80 ncdu /
 4949 vaio      20   0   17520   4224   2584 S   0.0  0.1   0:00.54 nnn -S /
```

`nnn` vs. midnight commander vs. ranger memory usage while viewing a directory with 13790 files, sorted by size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
16255 vaio      20   0  101392  59304   7928 S   0.0  0.7   0:00.68 /usr/bin/python -O /usr/bin/ranger
15971 vaio      20   0   65732  11784   6848 S   0.0  0.1   0:00.56 mc
16198 vaio      20   0   18520   4900   2536 S   0.3  0.1   0:00.14 nnn
```

Intrigued? Find out [HOW](https://github.com/jarun/nnn/wiki/performance-factors).

### Installation

#### Dependencies

`nnn` needs libreadline, libncursesw (on Linux or ncurses on OS X) and standard libc.

#### From a package manager

- [AUR](https://aur.archlinux.org/packages/nnn/) (`pacman -S nnn`)
- [Debian](https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Fedora](https://apps.fedoraproject.org/packages/nnn) (`dnf install nnn`)
- [FreeBSD](https://www.freshports.org/misc/nnn) (`pkg install nnn`)
- [Gentoo](https://packages.gentoo.org/packages/app-misc/nnn) (`emerge nnn`)
- [Homebrew](http://formulae.brew.sh/formula/nnn) (`brew install nnn`)
- [NixOS](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn) (`nix-env -i nnn`)
- [openSUSE](https://software.opensuse.org/package/nnn) (and packages for several other distros) (`zypper in nnn`)
- [Slackware](http://slackbuilds.org/repository/14.2/system/nnn/) (`slackpkg install nnn`)
- [Source Mage](http://codex.sourcemage.org/test/shell-term-fm/nnn/) (`cast nnn`)
- [Ubuntu](https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/) (`apt-get install nnn`)
- [Void Linux](https://github.com/voidlinux/void-packages/tree/master/srcpkgs/nnn) (`xbps-install -S nnn`)

#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora and Ubuntu are available with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

To cook yourself, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Then install the dependencies and compile (e.g. on Ubuntu 16.04):

    $ sudo apt-get install libncursesw5-dev libreadline6-dev
    $ make
    $ sudo make install

`PREFIX` is supported, in case you want to install to a different location.

### Shell completion

Search keyword and option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`scripts/auto-completion/`](scripts/auto-completion). Please refer to your shell's manual for installation instructions.

### Usage

#### Cmdline options

```
usage: nnn [-b key] [-c N] [-e] [-i] [-l]
           [-p nlay] [-S] [-v] [-h] [PATH]

The missing terminal file browser for X.

positional arguments:
  PATH   start dir [default: current dir]

optional arguments:
 -b key  specify bookmark key to open
 -c N    specify dir color, disables if N>7
 -e      use exiftool instead of mediainfo
 -i      start in navigate-as-you-type mode
 -l      start in light mode (fewer details)
 -p nlay path to custom nlay
 -S      start in disk usage analyzer mode
 -v      show program version and exit
 -h      show this help and exit
```

`>` indicates the currently selected entry in `nnn`.

#### Keyboard shortcuts

```
            Key | Function
              - + -
       ↑, k, ^P | Prev entry
       ↓, j, ^N | Next entry
       PgUp, ^U | Scroll half page up
       PgDn, ^D | Scroll half page down
 Home, g, ^, ^A | First entry
  End, G, $, ^E | Last entry
    →, ↵, l, ^M | Open file/enter dir
 ←, Bksp, h, ^H | Parent dir
             ^O | Open with...
     Insert, ^I | Toggle nav-as-you-type
              ~ | Go HOME
              & | Start-up dir
              - | Last visited dir
              / | Filter entries
             ^/ | Open desktop search app
              . | Toggle show . files
             ^B | Bookmark prompt
              b | Pin current dir
             ^V | Go to pinned dir
              c | Change dir prompt
              d | Toggle detail view
              D | File details
              m | Brief media info
              M | Full media info
              n | Create new
             ^R | Rename entry
              r | Open dir in vidir
              s | Toggle sort by size
          S, ^J | Toggle du mode
              t | Toggle sort by mtime
          !, ^] | Spawn SHELL in dir
              R | Run custom script
              e | Edit entry in EDITOR
              o | Open DE filemanager
              p | Open entry in PAGER
              F | List archive
             ^F | Extract archive
             ^K | Copy file path
             ^Y | Toggle multi-copy
             ^T | Toggle path quote
             ^L | Redraw, clear prompt
              L | Lock terminal
              ? | Help, settings
          Q, ^G | Quit and cd
          q, ^X | Quit
```

Help & settings, file details, media info and archive listing are shown in the PAGER. Please use the PAGER-specific keys in these screens.

#### Filters

Filters support regexes to instantly (search-as-you-type) list the matching entries in the current directory.

There are 3 ways to reset a filter:
- pressing <kbd>^L</kbd> (at the new/rename prompt <kbd>^L</kbd> followed by <kbd>Enter</kbd> discards all changes and exits prompt)
- a search with no matches
- an extra backspace at the filter prompt (like vi)

Common use cases:
- to list all matches starting with the filter expression, start the expression with a `^` (caret) symbol
- type `\.mkv` to list all MKV files

If `nnn` is invoked as root or the environment variable `NNN_SHOW_HIDDEN` is set the default filter will also match hidden files.

#### Navigate-as-you-type mode

In this mode directories are opened in filter mode, allowing continuous navigation. Works best with the **arrow keys**.

#### File type abbreviations

The following abbreviations are used in the detail view:

| Symbol | File Type |
| --- | --- |
| `/` | Directory |
| `*` | Executable |
| <code>&#124;</code> | Fifo |
| `=` | Socket |
| `@` | Symbolic Link |
| `b` | Block Device |
| `c` | Character Device |

#### File handling

| External dependency | Operation |
| --- | --- |
| xdg-open (Linux), open(1) (OS X) | desktop opener |
| mediainfo, exiftool | multimedia file details |
| gnome-search-tool, catfish | desktop search utility |
| atool | list and extract archives |
| vidir from moreutils | batch rename, move, delete dir entries |
| vlock (Linux) | terminal locker |
| $EDITOR | edit files (fallback vi) |
| $PAGER | page through files (fallback less) |
| $SHELL | spawn a shell, run script (fallback sh) |

- To edit all text files in EDITOR (preferably CLI, fallback vi):

      export NNN_USE_EDITOR=1
- To enable the desktop file manager key, set `NNN_DE_FILE_MANAGER`. E.g.:

      export NNN_DE_FILE_MANAGER=thunar
      export NNN_DE_FILE_MANAGER=nautilus

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

### Quickstart

Add the following to your shell's rc file for the best experience:

1. Use a shorter and sweeter alias:

       alias n=nnn
2. Optionally open all text files in EDITOR (fallback vi):

       export NNN_USE_EDITOR=1
3. Set a desktop file manager to open directories with (if you ever need to). E.g.:

       export NNN_DE_FILE_MANAGER=thunar

4. Run `n`.

5. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.

### How to

#### add bookmarks

Set environment variable `NNN_BMS` as a string of `key:location` pairs (max 10) separated by semicolons (`;`):

    export NNN_BMS='doc:~/Documents;u:/home/user/Cam Uploads;D:~/Downloads/'

The bookmark prompt also understands the <kbd>~</kbd> (HOME), <kbd>-</kbd> (last visited directory) and <kbd>&</kbd> (start directory) shortcuts.

#### use cd .....

To jump to the n<sup>th</sup> level parent, use `n + 1` dots (the first `.` denotes PWD). For example, to jump to the 6<th> parent of the current directory, use 7 dots. If the number of dots would take you *beyond* `/` (which isn't possible), you'll be placed at `/`.

#### cd on quit

Pick the appropriate file for your shell from [`scripts/quitcd`](scripts/quitcd) and add the contents to your shell's rc file. You'll need to spawn a new shell for the change to take effect. You should start `nnn` as `n` (or modify the function name to something else).

As you might notice, `nnn` uses the environment variable `NNN_TMPFILE` to write the last visited directory path. You can change it.

#### copy file paths to clipboard

`nnn` can pipe the absolute path of the current file or multiple files to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on OS X.

Sample Linux copier script:

    #!/bin/sh

    # comment the next line to convert newlines to spaces
    IFS=

    echo -n $1 | xsel --clipboard --input

export `NNN_COPIER`:

    export NNN_COPIER="/path/to/copier.sh"

Use <kbd>^K</kbd> to copy the absolute path (from `/`) of the file under the cursor to clipboard.

To copy multiple file paths, switch to the multi-copy mode using <kbd>^Y</kbd>. In this mode you can

- select multiple files one by one by pressing <kbd>^K</kbd> on each entry; or,
- navigate to another file in the same directory to select a range of files.

Pressing <kbd>^Y</kbd> again copies the paths to clipboard and exits the multi-copy mode.

To wrap each file path within single quotes, export `NNN_QUOTE_ON`:

    export NNN_QUOTE_ON=1
This is particularly useful if you are planning to copy the whole string to the shell to run a command. Quotes can be toggled at runtime using <kbd>^T</kbd>.

#### copy file paths when X is missing

A very common scenario on headless remote servers connected via SSH. As the clipboard is missing, `nnn` copies the path names to the tmp file `/tmp/nnncp$USER`.

`nnn` needs to know X is unavailable:

    export NNN_NO_X=1

Use <kbd>^Y</kbd> and/or <kbd>^K</kbd> to copy file paths as usual. To use the copied paths from the cmdline, use command substitution:

    # bash/zsh
    ls -ltr `cat /tmp/nnncpuser`
    ls -ltr $(cat /tmp/nnncpuser)

    # fish
    ls -ltr (cat /tmp/nnncpuser)

An alias may be handy:

    alias ncp='cat /tmp/nnncpuser'

so you can -

    # bash/zsh
    ls -ltr `ncp`
    ls -ltr $(ncp)

    # fish
    ls -ltr (ncp)

Note that you may want to keep quotes disabled in this case.

#### run a custom script

Export the path to the custom script:

    export NNN_SCRIPT=/usr/local/bin/script.sh

Sample script to open image files in current dir in sxiv:

    #!/usr/bin/env sh

    sxiv -q * >/dev/null 2>&1

Press <kbd>R</kbd> to run the script in the current directory.

#### change dir color

The default color for directories is blue. Option `-c` accepts color codes from 0 to 7 to use a different color:

    0-black, 1-red, 2-green, 3-yellow, 4-blue, 5-magenta, 6-cyan, 7-white

Any other value disables colored directories.

#### file copy, move, delete

`nnn` doesn't support file copy, move, delete natively. However, it simplifies the workflow:

1. copy the absolute paths using <kbd>^Y</kbd> and/or <kbd>^K</kbd>
2. spawn a shell in the current directory (<kbd>!</kbd>)
3. while typing the desired command, copy the file paths (usually <kbd>^-Shift-V</kbd>)

In addition, nnn integrates with vidir. vidir supports batch file move and delete.

#### boost chdir prompt

`nnn` uses libreadline for the chdir prompt input. So all the fantastic features of readline (e.g. case insensitive tab completion, history, reverse-i-search) are available to you based on your readline [configuration](https://cnswww.cns.cwru.edu/php/chet/readline/readline.html#SEC9).

#### work faster at rename prompt

The rename prompt supports some bash-like command-line shortcuts - <kbd>^A</kbd>, <kbd>^E</kbd>, <kbd>^U</kbd>. <kbd>^L</kbd> clears the name.

#### set idle timeout

The terminal screensaver is disabled by default. To set the wait time in seconds, use environment variable `NNN_IDLE_TIMEOUT`.

#### show hot plugged drives

Enable volume management in your DE file manager and set removable drives or media to be auto-mounted when inserted. Then visit the usual mount point location (`/mnt` or `/media/user`) in `nnn`.

### Why fork?

I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind. Check out [nnn design considerations](https://github.com/jarun/nnn/wiki/nnn-design-considerations) for more details.

### Mentions

- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)

### Developers

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2018 [Arun Prakash Jana](https://github.com/jarun)
