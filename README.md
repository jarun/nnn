## nnn

Noice is Not Noice, a noicer fork...

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://aur.archlinux.org/packages/nnn"><img src="https://img.shields.io/aur/version/nnn.svg?maxAge=600" alt="AUR" /></a>
<a href="http://formulae.brew.sh/formula/nnn"><img src="https://img.shields.io/homebrew/v/nnn.svg?maxAge=600" alt="Homebrew" /></a>
<a href="https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/debian-10+-blue.svg?maxAge=2592000" alt="Debian Buster+" /></a>
<a href="https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/ubuntu-17.10+-blue.svg?maxAge=2592000" alt="Ubuntu Artful+" /></a>
<a href="https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/"><img src="https://img.shields.io/badge/ubuntu-PPA-blue.svg?maxAge=2592000" alt="Ubuntu PPA" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://travis-ci.org/jarun/nnn.svg?branch=master" alt="Build Status" /></a>
</p>

[![nnn screencast](https://s26.postimg.org/9pjzvc9g9/nnn_demo.jpg)](https://vimeo.com/233223942 "Click to see nnn in action!")

<p align="center"><i>nnn in action!</i></a></p>

`nnn` is a fork of [noice](http://git.2f30.org/noice/), a blazing-fast lightweight terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed considering terminal based systems. There is no config file and mime associations are hard-coded. However, the incredible user-friendliness and speed make it a perfect candidate for modern distros.

`nnn` works with the desktop opener, adds new navigation options, [navigate-as-you-type](#navigate-as-you-type-mode) mode, enhanced DE integration, bookmarks, a disk usage analyzer mode, comprehensive file details and much more. Add to that a huge [performance](#performance) boost. For a detailed comparison, visit [nnn vs. noice](https://github.com/jarun/nnn/wiki/nnn-vs.-noice).

Cool things you can do with `nnn`:

- open any file in the default desktop application for the mime
- *navigate-as-you-type* (*search-as-you-type* enabled even on directory switch)
- check disk usage with number of files in current directory tree
- run desktop search utility (gnome-search-tool or catfish) in any directory
- copy absolute file path to clipboard, spawn a terminal and use the file path
- navigate instantly using shortcuts like `~`, `-`, `&` or handy bookmarks
- use `cd .....` at chdir prompt to go to a parent directory
- detailed file stats, media info, list and extract archives
- pin a directory you may need to revisit and jump to it anytime
- lock the current terminal after a specified idle time
- change directory on exit

If you want to edit a file in vim with some soothing music in the background while referring to a spec in your GUI PDF viewer, `nnn` got it! [Quickstart](#quickstart) and see how `nnn` simplifies those long desktop sessions...

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/nnn/issues/1).

*Love smart and efficient terminal utilities? Explore my repositories. Buy me a cup of coffee if they help you.*

<p align="center">
<a href="https://saythanks.io/to/jarun"><img src="https://img.shields.io/badge/say-thanks!-ff69b4.svg" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/Donate-$5-green.svg" alt="Donate via PayPal!" /></a>
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
  - [copy file path to clipboard](#copy-file-path-to-clipboard)
  - [change dir color](#change-dir-color)
  - [file copy, move, delete](#file-copy-move-delete)
  - [boost chdir prompt](#boost-chdir-prompt)
  - [set idle timeout](#set-idle-timeout)
  - [show hot plugged drives](#show-hot-plugged-drives)
- [Troubleshooting](#troubleshooting)
  - [nnn blocks on opening files](#nnn-blocks-on-opening-files)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)

### Features

- Navigation
  - Familiar shortcuts
  - *Navigate-as-you-type* mode
  - Bookmarks support; pin and visit a directory
  - Jump HOME or to the last visited directory (as usual!)
  - Jump to initial dir, chdir prompt, cd ..... (with . as PWD)
  - Roll-over at edges, page through entries
  - Show directories in custom color (default: enabled in blue)
- Disk usage analyzer mode
- Search
  - Filter directory contents with *search-as-you-type*
  - Desktop search (default gnome-search-tool, customizable) integration
- Mimes
  - Desktop opener integration
  - Optionally open text files in EDITOR (fallback vi)
  - Customizable bash script [nlay](https://github.com/jarun/nnn/wiki/all-about-nlay) to handle actions
- Information
  - Basic and detail view
  - Detailed file information
  - Media information (needs mediainfo or exiftool, if specified)
- Ordering
  - Numeric order (1, 2, ... 10, 11, ...) for numeric names
  - Sort by modification time, size
- Convenience
  - Create, rename files and directories
  - Spawn SHELL (fallback sh) in the current directory
  - Invoke file path copier (*easy* shell integration)
  - Change directory at exit (*easy* shell integration)
  - Open any file in EDITOR (fallback vi) or PAGER (fallback less)
  - List and extract archives (needs atool)
  - Open current directory in a custom GUI file browser
  - Monitor directory changes
  - Terminal screensaver (default vlock, customizable) integration
- Unicode support
- Highly optimized code, minimal resource usage

### Performance

`nnn` vs. ncdu memory usage in disk usage analyzer mode (438767 files on disk):

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
22515 vaio      20   0   60348  48712   2240 S   0.0  0.6   0:01.11 ncdu /
28306 vaio      20   0   17644   4500   2708 S   0.0  0.1   0:00.52 nnn -S /
```

`nnn` vs. mc vs. ranger memory usage while viewing a directory with 11244 files, sorted by size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
28450 vaio      20   0   93848  51548   7724 S   0.0  0.6   0:00.64 /usr/bin/python -O /usr/bin/ranger
27265 vaio      20   0   67188  13620   6908 S   0.0  0.2   0:00.16 mc
27925 vaio      20   0   20608   7168   2648 S   0.0  0.1   0:00.30 nnn
```

### Installation

#### Dependencies

`nnn` needs libreadline, libncursesw (on Linux or ncurses on OS X) and standard libc.

#### From a package manager

- [AUR](https://aur.archlinux.org/packages/nnn/)
- [Debian](https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1)
- [Homebrew](http://formulae.brew.sh/formula/nnn)
- [NixOS](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn) (`sudo nix-env -i nnn`)
- [Slackware] (http://slackbuilds.org/repository/14.2/system/nnn/)
- [Source Mage](http://codex.sourcemage.org/test/shell-term-fm/nnn/) (`cast nnn`)
- [Ubuntu](https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1)
- [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/)
- [Void Linux](https://github.com/voidlinux/void-packages/tree/master/srcpkgs/nnn) (`sudo xbps-install -S nnn`)

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
usage: nnn [-c N] [-e] [-i] [-l] [-p nlay] [-S]
           [-v] [-h] [PATH]

The missing terminal file browser for X.

positional arguments:
  PATH   directory to open [default: current dir]

optional arguments:
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
       ↑, k, ^P | Previous entry
       ↓, j, ^N | Next entry
       PgUp, ^U | Scroll half page up
       PgDn, ^D | Scroll half page down
 Home, g, ^, ^A | Jump to first entry
  End, G, $, ^E | Jump to last entry
    →, ↵, l, ^M | Open file or enter dir
 ←, Bksp, h, ^H | Go to parent dir
         Insert | Toggle navigate-as-you-type
              ~ | Go HOME
              & | Go to initial dir
              - | Go to last visited dir
              / | Filter dir contents
             ^/ | Open desktop search tool
              . | Toggle hide . files
              b | Bookmark prompt
             ^B | Pin current dir
             ^V | Go to pinned dir
              c | Change dir prompt
              d | Toggle detail view
              D | File details
              m | Brief media info
              M | Full media info
              n | Create new
             ^R | Rename selected entry
              s | Toggle sort by size
              S | Toggle disk usage mode
              t | Toggle sort by mtime
              ! | Spawn SHELL in dir
              e | Edit entry in EDITOR
              o | Open dir in file manager
              p | Open entry in PAGER
              F | List archive
             ^X | Extract archive
             ^K | Invoke file path copier
             ^L | Redraw, clear prompt
              ? | Help, settings
              Q | Quit and change dir
          q, ^Q | Quit
```

Help & settings, file details, media info and archive listing are shown in the PAGER. Please use the PAGER-specific keys in these screens.

#### Filters

Filters support regexes to instantly (search-as-you-type) list the matching entries in the current directory.

There are 3 ways to reset a filter: <kbd>^L</kbd>, a search with no matches or an extra backspace at the filter prompt (like vi).

Common examples: If you want to list all matches starting with the filter expression, start the expression with a `^` (caret) symbol. Type `\.mkv` to list all MKV files.

If `nnn` is invoked as root the default filter will also match hidden files.

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

- `nnn` uses `xdg-open` on Linux and `open(1)` on OS X as the desktop opener.
- To edit all text files in EDITOR (preferably CLI, fallback vi):

      export NNN_USE_EDITOR=1
- To enable the desktop file manager key, set `NNN_DE_FILE_MANAGER`. E.g.:

      export NNN_DE_FILE_MANAGER=thunar
      export NNN_DE_FILE_MANAGER=nautilus
- [mediainfo](https://mediaarea.net/en/MediaInfo) (or [exiftool](https://sno.phy.queensu.ca/~phil/exiftool/), if specified) is required to view media information
- [atool](http://www.nongnu.org/atool/) is required to list and extract archives

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

5. Set `NNN_NOWAIT`, if nnn [blocks on your desktop environment](#nnn-blocks-on-opening-files) when a file is open.

### How to

#### add bookmarks

Set environment variable `NNN_BMS` as a string of `key:location` pairs (max 10) separated by semicolons (`;`):

    export NNN_BMS='doc:~/Documents;u:/home/user/Cam Uploads;D:~/Downloads/'

#### use cd .....

To jump to the n<sup>th</sup> level parent, with PWD at level 0, use `n + 1` dots. For example, to jump to the 6<th> parent of the current directory, use 7 dots. If the number of dots would take you *beyond* `/` (which isn't possible), you'll be placed at `/`.

#### cd on quit

Pick the appropriate file for your shell from [`scripts/quitcd`](scripts/quitcd) and add the contents to your shell's rc file. You'll need to spawn a new shell for the change to take effect. You should start `nnn` as `n` (or modify the function name to something else).

As you might notice, `nnn` uses the environment variable `NNN_TMPFILE` to write the last visited directory path. You can change it.

#### copy file path to clipboard

`nnn` can pipe the absolute path of the current file to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on OS X.

Sample Linux copier script:

    #!/bin/sh

    echo -n $1 | xsel --clipboard --input

export `NNN_COPIER`:

    export NNN_COPIER="/path/to/copier.sh"

Start `nnn` and use <kbd>^K</kbd> to copy the absolute path (from `/`) of the file under the cursor to clipboard.

#### change dir color

The default color for directories is blue. Option `-c` accepts color codes from 0 to 7 to use a different color:

    0-black, 1-red, 2-green, 3-yellow, 4-blue, 5-magenta, 6-cyan, 7-white

Any other value disables colored directories.

#### file copy, move, delete

`nnn` doesn't support file copy, move, delete inherently. However, it simplifies the workflow:

1. copy the absolute path to a file by invoking the file path copier (<kbd>^K</kbd>)
2. spawn a shell in the current directory (<kbd>!</kbd>)
3. while typing the desired command, copy the file path (usually <kbd>^-Shift-V</kbd>)

#### boost chdir prompt

`nnn` uses libreadline for the chdir prompt input. So all the fantastic features of readline (e.g. case insensitive tab completion, history, reverse-i-search) is available to you based on your readline [configuration](https://cnswww.cns.cwru.edu/php/chet/readline/readline.html#SEC9).

#### set idle timeout

The terminal screensaver is disabled by default. To set the wait time in seconds, use environment variable `NNN_IDLE_TIMEOUT`.

#### show hot plugged drives

Enable volume management in your DE file manager and set removable drives or media to be auto-mounted when inserted. Then visit the usual mount point location (`/mnt` or `/media/user`) in `nnn`.

### Troubleshooting

#### nnn blocks on opening files

Ideally nnn should not block. Unfortunately, sometimes even the same desktop opener behaves differently on different Linux desktop environments. If `nnn` does block when a file is open, set the environment variable `NNN_NOWAIT` to any non-zero value. For example,

    export NNN_NOWAIT=1

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
3. Copyright © 2016-2017 [Arun Prakash Jana](https://github.com/jarun)
