## nnn

Noice is Not Noice, a noicer fork...

<br>

[![nnn-demo-20170405.mp4](https://i.imgur.com/VGA6IcB.png)](https://static.zhimingwang.org/nnn-demo-20170405.mp4 "Click for some action!")

<p align="center"><a href="https://static.zhimingwang.org/nnn-demo-20170405.mp4">Click for some action!</a></p>

### Table of Contents

- [Introduction](#introduction)
- [Why fork?](#why-fork)
- [Original features](#original-features)
- [nnn toppings](#nnn-toppings)
  - [Behaviour and navigation](#behaviour-and-navigation)
  - [File associations](#file-associations)
  - [Optimizations](#optimizations)
- [Installation](#installation)
- [Usage](#usage)
- [Keyboard shortcuts](#keyboard-shortcuts)
- [Filters](#filters)
- [File type abbreviations](#file-type-abbreviations)
- [Help](#help)
- [Copy current file path to clipboard](#copy-current-file-path-to-clipboard)
- [Change file associations](#change-file-associations)

### Introduction

nnn is a fork of [noice](http://git.2f30.org/noice/), a blazing-fast lightweight terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed considering terminal based systems. There is no config file and mime associations are hard-coded. However, the incredible user-friendliness and speed make it a perfect utility on modern distros.

nnn can use the default desktop opener at runtime. It also adds new navigation options, a disk usage analyzer mode, comprehensive file details and much more. For a complete list, see [nnn-toppings](#nnn-toppings).

You can try

    alias n='nnn -d'
and see how nnn simplifies those long desktop sessions.

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/nnn/issues/1).

### Why fork?

I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind.

### Original features

- Super-easy navigation
- Pre-defined associaitons for different file types
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
#### File associations
  - Environment variable `NNN_OPENER` to let desktop opener handle it all. E.g.:

        export NNN_OPENER=xdg-open
        export NNN_OPENER=gnome-open
        export NNN_OPENER=gvfs-open
  - Selective file associations (ignored if `NNN_OPENER` is set):
    - Associate plain text files (determined using file) with vi
    - Associate common audio and video mimes with mpv
    - Associate PDF files with [zathura](https://pwmt.org/projects/zathura/)
    - Removed `less` as default file opener (there is no universal standalone opener utility)
    - You can customize further (see [how to change file associations](#change-file-associations))
  - Environment variable `NNN_FALLBACK_OPENER` is the last line of defense:
    - If the executable in static file association is missing
    - If a file type was not handled in static file association
    - This may be the best option to set your desktop opener to
#### Optimizations
  - All redundant buffer removal
  - All frequently used local chunks now static
  - No runtime surprises (0 malloc/free)
  - Removed some redundant string allocation and manipulation
  - Simplified some roundabout procedures
  - `-O3` level optimization, warning fixes
  - Added compilation flag `-march=native`
  - Remove generated config.h on `make clean`
  - strip the final binary

The following top excerpt shows the difference in nnn and ncdu memory usage while listing `/usr/bin` with 1439 entries in disk usage analyzer mode, sorted by total content size:

```
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
 6054 vaio      20   0   56768  45268   2300 S   0.0  0.9   0:02.82 ncdu
10806 vaio      20   0   21228   8572   2432 S   0.0  0.2   0:00.07 nnn -d
```

### Installation

nnn needs a curses implementation and standard libc.

Download the [latest master](https://github.com/jarun/nnn/archive/master.zip) or clone this repository. Compile and install:

    $ make
    $ sudo make install
No plans of packaging at the time.

### Usage

Start nnn (default: current directory):

    $ nnn [-d] [path_to_dir]

    -d: open in detail view mode
`>` indicates the currently selected entry in nnn.

### Keyboard shortcuts

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

### Filters

Filters support regexes to display only the matched entries in the current directory view. This effectively allows searching through the directory tree for a particular entry.

Filters do not stack on top of each other. They are applied anew every time.

An empty filter expression resets the filter.

If nnn is invoked as root the default filter will also match hidden files.

### File type abbreviations

The following abbreviations are used in the detail view:

| Symbol | File Type |
| --- | --- |
| `/` | Directory |
| `*` | Executable |
| `|` | Fifo |
| `=` | Socket |
| `@` | Symbolic Link |
| `b` | Block Device |
| `c` | Character Device |

### Help

    $ man nnn

### Copy current file path to clipboard

nnn can pipe the absolute path of the current file to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on OS X.

Sample Linux copier script:

    #!/bin/sh

    echo -n $1 | xsel --clipboard --input

export `NNN_OPENER`:

    export NNN_COPIER="/home/vaio/copier.sh"

Start nnn and use `Ctrl-k` to copy the absolute path (from `/`) of the file under the cursor to clipboard.

### Change file associations

If you want to set custom applications for certain mime types, or change the ones set already (e.g. vi, mpv, zathura), modify the `assocs` structure in [config.def.h](https://github.com/jarun/nnn/blob/master/config.def.h) (it's easy). Then re-compile and install.
