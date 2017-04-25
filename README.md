## nnn

Noice is Not Noice, a noicer fork...

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://aur.archlinux.org/packages/nnn"><img src="https://img.shields.io/aur/version/nnn.svg?maxAge=600" alt="AUR" /></a>
<a href="http://braumeister.org/formula/nnn"><img src="https://img.shields.io/homebrew/v/nnn.svg?maxAge=600" alt="Homebrew" /></a>
<a href="https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/"><img src="https://img.shields.io/badge/ubuntu-PPA-blue.svg?maxAge=2592000" alt="Ubuntu PPA" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://travis-ci.org/jarun/nnn.svg?branch=master" alt="Build Status" /></a>
</p>

[![nnn-demo-20170405.mp4](https://i.imgur.com/VGA6IcB.png)](https://static.zhimingwang.org/nnn-demo-20170405.mp4 "Click for some action!")

<p align="center"><a href="https://static.zhimingwang.org/nnn-demo-20170405.mp4">Click for some action!</a></p>

### Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Performance](#performance)
- [nlay](#nlay)
- [Installation](#installation)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Filters](#filters)
  - [File type abbreviations](#file-type-abbreviations)
  - [File handling](#file-handling)
  - [Help](#help)
- [Quickstart](#quickstart)
- [How to](#how-to)
  - [cd on quit](#cd-on-quit)
  - [Copy current file path to clipboard](#copy-current-file-path-to-clipboard)
  - [Boost chdir prompt](#boost-chdir-prompt)
  - [Change file associations](#change-file-associations)
- [Why fork?](#why-fork)
- [Developers](#developers)

### Introduction

nnn is a fork of [noice](http://git.2f30.org/noice/), a blazing-fast lightweight terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed considering terminal based systems. There is no config file and mime associations are hard-coded. However, the incredible user-friendliness and speed make it a perfect utility on modern distros.

nnn can use the default desktop opener at runtime. It also comes with `nlay` - a customizable bash script to handle media types. It adds new navigation options, enhanced DE integration, a disk usage analyzer mode, comprehensive file details and much more. Add to that a huge [performance](#performance) boost. For a detailed comparison, visit [nnn vs. noice](https://github.com/jarun/nnn/wiki/nnn-vs.-noice).

Follow the instructions in the [quickstart](#quickstart) section and see how nnn simplifies those long desktop sessions.

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/nnn/issues/1).

<p align="right">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://tuxtricks.files.wordpress.com/2016/12/donate.png" alt="Donate via PayPal!" title="Donate via PayPal!" /></a>
</p>

### Features

- Super-easy navigation with roll-over at edges
- Jump HOME or back to the last visited directory (as usual!)
- Jump to initial dir, chdir prompt, cd ..... (with . as PWD)
- Desktop opener integration to handle mime types
- Customizable bash script nlay to handle known file types
- Disk usage analyzer mode
- Basic and detail view (with stat and file information)
- Show media information (needs mediainfo)
- Sort by modification time, size
- Sort numeric names in numeric order (1, 2, ... 10, 11, ...)
- Search directory contents using regex expressions
- Spawn a shell in the current directory
- Invoke file path copier (*easy* shell integration)
- Quit and change directory (*easy* shell integration)
- Open any file in EDITOR (fallback vi) or PAGER (fallback less)
- VIM-ish keybinds
- UTF-8 support

### Performance

nnn vs. ncdu memory usage while listing an external disk with 13,790 files in disk usage analyzer mode:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
31163 vaio      20   0   65508  53980   2320 S   0.0  1.1   0:01.96 ncdu /
28863 vaio      20   0   21348   7812   2476 S   0.0  0.2   0:01.75 nnn -d
```

nnn vs. mc vs. ranger memory usage while viewing a directory with 10,178 files, sorted by size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
22465 vaio      20   0  233956 192136   7896 S   0.0  3.9   0:05.31 /usr/bin/python -O /usr/bin/ranger
20369 vaio      20   0   64664  10980   6888 S   0.0  0.2   0:00.70 mc
28863 vaio      20   0   19876   6436   2620 S   0.0  0.1   0:00.19 nnn -d
```

### nlay

nnn comes with an easily customizable bash shell script to media types - nlay. To know more about it, visit [nlay on wiki](https://github.com/jarun/nnn/wiki/all-about-nlay).

### Installation

nnn needs libreadline, libncursesw (on Linux or ncurses on OS X) and standard libc.

- Packages are available on
  - [AUR](https://aur.archlinux.org/packages/nnn/)
  - [Homebrew](http://braumeister.org/formula/nnn)
  - [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/)
- To compile and install, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Run:

      $ make
      $ sudo make install

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

```
                Key | Function
                   -+-
          Up, k, ^P | Previous entry
        Down, j, ^N | Next entry
           PgUp, ^U | Scroll half page up
           PgDn, ^D | Scroll half page down
     Home, g, ^, ^A | Jump to first entry
      End, G, $, ^E | Jump to last entry
Right, Enter, l, ^M | Open file or enter dir
  Left, Bksp, h, ^H | Go to parent dir
                  ~ | Jump to HOME dir
                  & | Jump to initial dir
                  - | Jump to last visited dir
                  o | Open dir in NNN_DE_FILE_MANAGER
                  / | Filter dir contents
                  c | Show change dir prompt
                  d | Toggle detail view
                  D | Toggle current file details screen
                  m | Show concise mediainfo
                  M | Show full mediainfo
                  . | Toggle hide .dot files
                  s | Toggle sort by file size
                  S | Toggle disk usage analyzer mode
                  t | Toggle sort by modified time
                  ! | Spawn SHELL in PWD (fallback sh)
                  z | Run top
                  e | Edit entry in EDITOR (fallback vi)
                  p | Open entry in PAGER (fallback less)
                 ^K | Invoke file name copier
                 ^L | Force a redraw
                  ? | Toggle help screen
                  q | Quit
                  Q | Quit and change directory
```

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

#### File handling

nnn is designed to play files using multiple strategies (in order of decreasing priority):
  - Set `NNN_OPENER` to let a desktop opener handle it all. E.g.:

        export NNN_OPENER=xdg-open
        export NNN_OPENER="gio open"
        export NNN_OPENER=gvfs-open
  - If nnn recognizes the file extension, it invokes nlay (which invokes the players). Default players:
    - mpv - audio and video
    - viewnior - image
    - [zathura](https://pwmt.org/projects/zathura/) - pdf
    - vim - plain text
    - to add, remove recognized extensions in nnn, see [how to change file associations](#change-file-associations)
  - If a file without any extension is a plain text file, it is opened in vi
  - Set `NNN_FALLBACK_OPENER` as the fallback opener. E.g.:

        export NNN_FALLBACK_OPENER=xdg-open
        export NNN_FALLBACK_OPENER="gio open"
        export NNN_FALLBACK_OPENER=gvfs-open
  - To enable the desktop file manager key, set `NNN_DE_FILE_MANAGER`. E.g.:

        export NNN_DE_FILE_MANAGER=thunar
        export NNN_DE_FILE_MANAGER=nautilus
  - [mediainfo](https://mediaarea.net/en/MediaInfo) is required to view media information

#### Help

    $ man nnn
To lookup keyboard shortcuts at runtime, press `?`.

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

#### Boost chdir prompt

nnn uses libreadline for the chdir prompt input. So all the flexibility of readline (e.g. case insensitive tab completion, history) is available to you based on your readline [configuration](https://cnswww.cns.cwru.edu/php/chet/readline/readline.html#SEC9).

#### Change file associations

If `NNN_OPENER` is not set, nnn tries to recognize a file by the file extension and invokes nlay. To change the extensions recognized by nnn, modify the `assocs` structure in [config.def.h](https://github.com/jarun/nnn/blob/master/config.def.h) (it's easy). Then re-compile and install.

If you want to add a file extension mainline, please raise a bug. Without it nnn will not invoke nlay.

nlay has provisions (disabled by default) to handle a specific file extension too. However, the extension should be recognized by nnn first.

### Why fork?

I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind. Check out [nnn design considerations](https://github.com/jarun/nnn/wiki/nnn-design-considerations) for more details.

### Developers

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2017 [Arun Prakash Jana](https://github.com/jarun)
