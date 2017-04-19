## nnn

Noice is Not Noice, a noicer fork...

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://aur.archlinux.org/packages/nnn"><img src="https://img.shields.io/aur/version/nnn.svg?maxAge=600" alt="AUR" /></a>
<a href="https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/"><img src="https://img.shields.io/badge/ubuntu-PPA-blue.svg?maxAge=2592000" alt="Ubuntu PPA" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://travis-ci.org/jarun/nnn.svg?branch=master" alt="Build Status" /></a>
</p>

[![nnn-demo-20170405.mp4](https://i.imgur.com/VGA6IcB.png)](https://static.zhimingwang.org/nnn-demo-20170405.mp4 "Click for some action!")

<p align="center"><a href="https://static.zhimingwang.org/nnn-demo-20170405.mp4">Click for some action!</a></p>

### Table of Contents

- [Introduction](#introduction)
- [Why fork?](#why-fork)
- [Original features](#original-features)
- [nnn toppings](#nnn-toppings)
  - [Behaviour and navigation](#behaviour-and-navigation)
  - [File association](#file-association)
  - [Optimization](#optimization)
- [Performance](#performance)
- [Installation](#installation)
- [Quickstart](#quickstart)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Filters](#filters)
  - [File type abbreviations](#file-type-abbreviations)
  - [Help](#help)
- [How to](#how-to)
  - [cd on quit](#cd-on-quit)
  - [Copy current file path to clipboard](#copy-current-file-path-to-clipboard)
  - [Change file associations](#change-file-associations)
- [Developers](#developers)

### Introduction

nnn is a fork of [noice](http://git.2f30.org/noice/), a blazing-fast lightweight terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed considering terminal based systems. There is no config file and mime associations are hard-coded. However, the incredible user-friendliness and speed make it a perfect utility on modern distros.

nnn can use the default desktop opener at runtime. It adds new navigation options, enhanced DE integration, a disk usage analyzer mode, comprehensive file details and much more. For a complete list, see [nnn toppings](#nnn-toppings).

Follow the instructions in the [quickstart](#quickstart) section and see how nnn simplifies those long desktop sessions.

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/nnn/issues/1).

<p align="right">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://tuxtricks.files.wordpress.com/2016/12/donate.png" alt="Donate via PayPal!" title="Donate via PayPal!" /></a>
</p>

### Why fork?

I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind.

### Original features

- Super-easy navigation
- Pre-defined associations for different file types
- Jump to home directory
- Filter contents in current directory
- Show/hide hidden files
- Sort entries by modification time (newest to oldest)
- Spawn a `SHELL` in current directory (fallback sh)
- Run `top`
- Edit a file with `EDITOR` (fallback vi)
- Page through a file in `PAGER` (fallback less)

### nnn toppings

#### Behaviour and navigation
  - Detail view (default: disabled) with:
    - file type (directory, regular, symlink etc.)
    - modification time
    - human-readable file size
    - current item in reverse video
    - number of items in current directory
    - full name of currently selected file in 'bar'
  - Show details of the currently selected file (stat, file)
  - Disk usage analyzer mode (within the same fs, doesn't follow symlinks)
  - Directories first (even with sorting)
  - Sort numeric names in numeric order
  - Case-insensitive alphabetic content listing instead of upper case first
  - Key `-` to jump to last visited directory
  - Roll over at the first and last entries of a directory (with Up/Down keys)
  - Removed navigation restriction with relative paths (and let permissions handle it)
  - Sort entries by file size (largest to smallest)
  - Shortcut to invoke file name copier (set using environment variable `NNN_COPIER`)
  - Change to last visited directory on quit

#### File association
  - Set `NNN_OPENER` to let a desktop opener handle it all. E.g.:

        export NNN_OPENER=xdg-open
        export NNN_OPENER="gio open"
        export NNN_OPENER=gvfs-open
  - Selective file associations (ignored if `NNN_OPENER` is set):
    - Associate plain text files (determined using file) with vi
    - Associate common audio and video mimes with mpv
    - Associate PDF files with [zathura](https://pwmt.org/projects/zathura/)
    - Removed `less` as default file opener (there is no universal standalone opener utility)
    - You can customize further (see [how to change file associations](#change-file-associations))
  - `NNN_FALLBACK_OPENER` is the last line of defense:
    - If the executable in static file association is missing
    - If a file type was not handled in static file association
    - This may be the best option to set your desktop opener to
  - To enable the desktop file manager key, set `NNN_DE_FILE_MANAGER`. E.g.:

        export NNN_DE_FILE_MANAGER=thunar

#### Optimization
  - All redundant buffer removal
  - All frequently used local chunks now static
  - Removed some redundant string allocation and manipulation
  - Simplified some roundabout procedures
  - Compiler warnings fixed
  - strip the final binary

### Performance

nnn vs. ncdu memory usage while listing `/usr/bin` with 1439 entries in disk usage analyzer mode, sorted by total content size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
10406 vaio      20   0   53808  42284   2248 S   0.0  0.8   0:00.82 ncdu
10409 vaio      20   0   20452   9172   2356 S   0.0  0.2   0:00.83 nnn -d
```

nnn vs. ranger memory usage while viewing a directory with 10,178 files, sorted by size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
22465 vaio      20   0  233956 192136   7896 S   0.0  3.9   0:05.31 /usr/bin/python -O /usr/bin/ranger
21743 vaio      20   0   55984  44648   2468 S   0.0  0.9   0:01.17 nnn -d
```

### Installation

nnn needs libncursesw on Linux (or ncurses on OS X) and standard libc.

- If you are using **Homebrew**, run:

      brew install jarun/nnn/nnn
- Packages are available on
  - [AUR](https://aur.archlinux.org/packages/nnn/)
  - [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/)
- To compile and install, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Run:

      $ make
      $ sudo make install

### Quickstart

Add the following to your shell's rc file for the best experience:

- If you want to open nnn in detail mode each time:

      alias n='nnn -d'
- Set your preferred desktop opener as fallback. E.g.:

      export NNN_FALLBACK_OPENER=xdg-open
- Set a desktop file manager to open directories with (if you need). E.g.:

      export NNN_DE_FILE_MANAGER=thunar
- Start nnn.

      n

### Usage

#### Cmdline options

    usage: nnn [-d] [-S] [-v] [h] [PATH]

    The missing terminal file browser for X.

    positional arguments:
      PATH           directory to open [default: current dir]

    optional arguments:
      -d             start in detail view mode
      -S             start in disk usage analyzer mode
      -v             show program version and exit
      -h             show this help and exit

`>` indicates the currently selected entry in nnn.

#### Keyboard shortcuts

| Key | Function |
| --- | --- |
| `Up`, `k`, `^P` | Previous entry |
| `Down`, `j`, `^N` | Next entry |
| `PgUp`, `^U` | Scroll half page up |
| `PgDn`, `^D` | Scroll half page down |
| `Home`, `g`, `^`, `^A` | Jump to first entry |
| `End`, `G`, `$`, `^E` | Jump to last entry |
| `Right`, `Enter`, `l`, `^M` | Open file or enter dir |
| `Left`, `Backspace`, `h`, `^H` | Go to parent dir |
| `~` | Jump to HOME dir |
| `-` | Jump to last visited dir |
| `o` | Open dir in desktop file manager |
| `/`, `&` | Filter dir contents |
| `c` | Show change dir prompt |
| `d` | Toggle detail view |
| `D` | Toggle current file details screen |
| `.` | Toggle hide .dot files |
| `s` | Toggle sort by file size |
| `S` | Toggle disk usage analyzer mode |
| `t` | Toggle sort by modified time |
| `!` | Spawn `SHELL` in `PWD` (fallback sh) |
| `z` | Run `top` |
| `e` | Edit entry in `EDITOR` (fallback vi) |
| `p` | Open entry in `PAGER` (fallback less) |
| `^K` | Invoke file name copier |
| `^L` | Force a redraw |
| `?` | Toggle help screen |
| `q` | Quit |
| `Q` | Quit and change directory |

#### Filters

Filters support regexes to display only the matched entries in the current directory view. This effectively allows searching through the directory tree for a particular entry.

Filters do not stack on top of each other. They are applied anew every time.

An empty filter expression resets the filter.

If nnn is invoked as root the default filter will also match hidden files.

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

#### Help

    $ man nnn
To lookup keyboard shortcuts at runtime, press `?`.

### How to

#### cd on quit

Pick the appropriate file for your shell from [misc/quitcd](https://github.com/jarun/nnn/tree/master/misc/quitcd) and add the contents to your shell's rc file. You'll need to spawn a new shell for the change to take effect. You should start nnn as `n` (or modify the function name to something else).

As you might notice, nnn uses the environment variable `NNN_TMPFILE` to write the last visited directory path. You can change it.

#### Copy current file path to clipboard

nnn can pipe the absolute path of the current file to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on OS X.

Sample Linux copier script:

    #!/bin/sh

    echo -n $1 | xsel --clipboard --input

export `NNN_OPENER`:

    export NNN_COPIER="/home/vaio/copier.sh"

Start nnn and use `^K` to copy the absolute path (from `/`) of the file under the cursor to clipboard.

#### Change file associations

If you want to set custom applications for certain mime types, or change the ones set already (e.g. vi, mpv, zathura), modify the `assocs` structure in [config.def.h](https://github.com/jarun/nnn/blob/master/config.def.h) (it's easy). Then re-compile and install.

### Developers

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2017 [Arun Prakash Jana](https://github.com/jarun)
