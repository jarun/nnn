## nnn

Noice is Not Noice, a noicer fork...

### Table of Contents

- [Introduction](#introduction)
- [Why fork?](#why-fork)
- [Original features](#original-features)
- [nnn toppings](#nnn-toppings)
- [Installation](#installation)
- [Usage](#usage)
- [Keyboard shortcuts](#keyboard-shortcuts)
- [Filters](#filters)
- [File type abbreviations](#file-type-abbreviations)
- [Help](#help)
- [Copy current file path to clipboard](#copy-current-file-path-to-clipboard)
- [Change file associations](#change-file-associations)

### Introduction

nnn is a fork of [noice](http://git.2f30.org/noice/), a blazing-fast terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. It is developed with terminal based systems in mind. However, the incredible user-friendliness and speed make it a perfect utility on modern distros.

The only issue with noice is hard-coded file associations. There is no config file (better performance and simpler to maintain) and one has to modify the source to change associations (see [how to change file associations](#change-file-associations)). nnn solves the problem by adding the flexibility of using the default desktop opener at runtime. There are several other improvements too (see [fork-toppings](#fork-toppings)).

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/nnn/issues/1).

### Why fork?

I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind.

### Original features

- Super-easy navigation
- Open files with default-associated programs
- Jump to home directory
- Filter contents in current directory
- Show/hide hidden files
- Sort entries by modification time (newest to oldest)
- Spawn a `SHELL` in current directory (fallback sh)
- Run `top`
- Edit a file with `EDITOR` (fallback vi)
- Page through a file in `PAGER` (fallback less)

### nnn toppings

- Behaviour and navigation
  - Detail view (default: disabled) with:
    - file type
    - modification time
    - human-readable file size
    - current item in reverse video
    - number of items in current directory
    - full name of currently selected file
  - Directories first
  - Sort numeric names in numeric order
  - Case-insensitive alphabetic content listing instead of upper case first
  - Roll over at the first and last entries of a directory (with Up/Down keys)
  - Removed navigation restriction with relative paths (and let permissions handle it)
  - Sort entries by file size (largest to smallest)
  - Shortcut to invoke file name copier (set using environment variable `NNN_COPIER`)
- File associations
  - Environment variable `NNN_OPENER` to let desktop opener handle it all. E.g.:

        export NNN_OPENER=xdg-open
        export NNN_OPENER=gnome-open
        export NNN_OPENER=gvfs-open
  - Selective file associations (ignored if `NNN_OPENER` is set):
    - Associate plain text files with vi (using `file` command)
    - Remove video file associations (to each his own favourite video player)
    - Associate common audio mimes with mpv
    - Associate PDF files with [zathura](https://pwmt.org/projects/zathura/)
    - Use environment variable `NNN_FALLBACK_OPENER` to open other non-associated files
    - Removed `less` as default file opener (there is no universal standalone opener utility)
- Optimizations
  - Efficient memory usage, 0 malloc()
  - Complete redundant buffer removal
  - All frequently used local chunks now static
  - Removed some redundant string allocation and manipulation
  - Simplified some roundabout procedures
  - `-O3` level optimization, warning fixes
  - Added compilation flag `-march=native`
  - Remove generated config.h on `make clean`
  - strip the final binary

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
| `Up`, `k`, `^P` | previous entry |
| `Down`, `j`, `^N` | next entry |
| `PgUp`, `^U` | scroll half page up |
| `PgDn`, `^D` | scroll half page down |
| `Home`, `^`, `^A` | jump to first dir entry |
| `End`, `$`, `^E` | jump to last dir entry |
| `Right`, `Enter`, `l`, `^M` | open file or enter dir |
| `Left`, `Backspace`, `h`, `^H` | parent dir |
| `~` | jump to home dir |
| `/`, `&` | filter dir contents |
| `c` | show change dir prompt |
| `d` | toggle detail view |
| `.` | toggle hide dot files |
| `s` | toggle sort by file size |
| `t` | toggle sort by modified time |
| `!` | spawn `SHELL` in `PWD` (fallback sh) |
| `z` | run `top` |
| `e` | edit entry in `EDITOR` (fallback vi) |
| `p` | open entry with `PAGER` (fallback less) |
| `^K` | invoke file name copier |
| `^L` | redraw window |
| `q` | quit |

### Filters

Filters support regexes to display only the matched entries in the current directory view. This effectively allows searching through the directory tree for a particular entry.

Filters do not stack on top of each other. They are applied anew every time. An empty filter expression resets the filter.

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
