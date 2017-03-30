## noice

A fork of the [noice](http://git.2f30.org/noice/) file browser to make it more friendly towards major distros (which `suck more` by some standards).

### Table of Contents

- [Introduction](#introduction)
- [Why fork?](#why-fork)
- [Default features](#default-features)
- [Fork toppings](#fork-toppings)
- [Installation](#installation)
- [Usage](#usage)
- [Keyboard shortcuts](#keyboard-shortcuts)
- [File type abbreviations](#file-type-abbreviations)
- [Help](#help)
- [Change file associations](#change-file-associations)

### Introduction

noice is a blazing-fast terminal file browser with easy keyboard shortcuts for navigation, opening files and running tasks. noice is developed with terminal based systems in mind. However, the incredible user-friendliness and speed make it a perfect utility on modern distros. Navigate to `/usr/bin` from your regular file browser and noice to feel the difference.

The only issue with noice is hard-coded file association. There is no config file (better performance and simpler to maintain) and you have to modify the source to change associations (see [how to change file associations](#change-file-associations)). This fork solves the problem by adding the flexibility of using the default desktop opener at runtime. There are several other improvements too (see [fork-toppings](#fork-toppings)).

Have fun with it! PRs are welcome. Check out [#1](https://github.com/jarun/noice/issues/1).

### Why fork?

I chose to fork noice because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind.

### Default features

- Super-easy navigation
- Open files with default-associated programs
- Jump to home directory
- Filter contents in current directory
- Show/hide hidden files
- Sort entries by modification time (newest to oldest)
- Spawn a shell in current directory
- Run `top`
- Open a file with `vim` or `less`

### Fork toppings

- Behaviour and navigation
  - Detail view (default: disabled) with:
    - file type
    - modification time
    - human-readable file size
    - number of entries in current directory
  - Case-insensitive alphabetic content listing instead of upper case first
  - Roll over at the first and last entries of a directory (with Up/Down keys)
  - Sort entries by file size (largest to smallest)
- File associations
  - Environment variable `NOICE_OPENER` to override all associations and open all files with your desktop environments default file opener. Examples:

        export NOICE_OPENER=xdg-open
        export NOICE_OPENER=gnome-open
        export NOICE_OPENER=gvfs-open
  - Selective file associations (ignored if `NOICE_OPENER` is set):
    - Associate plain text files with vim (using `file` command)
    - Remove video file associations (to each his own favourite video player)
    - Associate common audio mimes with lightweight [fmedia](http://fmedia.firmdev.com/)
    - Associate PDF files with [zathura](https://pwmt.org/projects/zathura/)
    - Removed `less` as default file opener
    - Use environment variable `NOICE_FALLBACK_OPENER` to open other non-associated files
- Compilation
  - Use `-O3` for compilation, fixed warnings
  - Added compilation flag `-march=native`
  - Remove generated config.h on `make clean`
  - strip the final binary

### Installation

noice needs a curses implementation and standard libc.

Download the [latest master](https://github.com/jarun/noice/archive/master.zip) or clone this repository. Compile and install:

    $ make
    $ sudo make install
No plans of packaging at the time.

### Usage

Start noice (default: current directory):

    $ noice [path_to_dir]
`>` indicates the currently selected entry.

### Keyboard shortcuts

| Key | Function |
| --- | --- |
| `Down`, `j`, `Ctrl-n` | next entry |
| `Up`, `k`, `Ctrl-p` | previous entry |
| `>`, `Enter`, `l` | open file or enter dir |
| `<`, `Backspace`, `h` | parent dir |
| `Page Down`, `Ctrl-d` | one page down |
| `Page Up`, `Ctrl-u` | one page up |
| `Home`, `Ctrl-a`, `^` | jump to first dir entry |
| `End`, `Ctrl-e`, `$` | jump to last dir entry |
| `~` | jump to home dir |
| `/`, `&` | filter dir contents |
| `c` | show change dir prompt |
| `d` | toggle detail view |
| `.` | toggle hide dot files |
| `s` | toggle sort by file size |
| `t` | toggle sort by modified time |
| `!` | spawn a shell in current dir |
| `e` | edit entry in `vim` |
| `p` | open entry with `less` pager |
| `z` | run `top` |
| `Ctrl-l` | redraw window |
| `q` | quit noice |

### File type abbreviations

The following abbreviations are used in the detail view:

| Symbol | File Type |
| --- | --- |
| B | Block Device |
| C | Character Device |
| D | Directory |
| E | Executable |
| F | Fifo |
| L | Symbolic Link |
| R | Regular File |
| S | Socket |

### Help

    $ man noice

### Change file associations

If you want to set custom applications for certain mime types, or change the ones set already (e.g. vim, fmedia, zathura), modify the `assocs` structure in [config.def.h](https://github.com/jarun/noice/blob/master/config.def.h) (it's easy). Then re-compile and install.
