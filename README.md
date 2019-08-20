<h1 align="center">nnn - <i>type less, do more, way faster</i></h1>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="http://formulae.brew.sh/formula/nnn"><img src="https://img.shields.io/homebrew/v/nnn.svg?maxAge=600" alt="Homebrew" /></a>
<a href="https://www.archlinux.org/packages/community/x86_64/nnn/"><img src="https://img.shields.io/badge/archlinux-rolling-blue.svg?maxAge=600" alt="Arch Linux" /></a>
<a href="https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/debian-10+-blue.svg?maxAge=2592000" alt="Debian Buster+" /></a>
<a href="https://apps.fedoraproject.org/packages/nnn"><img src="https://img.shields.io/badge/fedora-27+-blue.svg?maxAge=2592000" alt="Fedora 27+" /></a>
<a href="https://software.opensuse.org/package/nnn"><img src="https://img.shields.io/badge/opensuse%20leap-15.0+-blue.svg?maxAge=2592000" alt="openSUSE Leap 15.0+" /></a>
<a href="https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/ubuntu-17.10+-blue.svg?maxAge=2592000" alt="Ubuntu Artful+" /></a>
</p>

<p align="center">
<a href="https://repology.org/metapackage/nnn"><img src="https://repology.org/badge/tiny-repos/nnn.svg" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center">
<a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/onpq3vP.png" /></a>
</p>

<p align="center"><i>Modes of nnn (light with filter, detail, du analyzer) with memory usage (click for a demo video)</i></a></p>

## Index

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
  - [Library dependencies](#library-dependencies)
  - [Utility dependencies](#utility-dependencies)
  - [From a package manager](#from-a-package-manager)
  - [Release packages](#release-packages)
  - [From source](#from-source)
  - [Shell completion](#shell-completion)
- [Usage](#usage)
  - [Configuration](#configuration)
  - [Cmdline options](#cmdline-options)
  - [Keyboard and mouse](#keyboard-and-mouse)
    - [Leader key](#leader-key)
  - [Help](#help)
- [Concepts](#concepts)
  - [Contexts](#contexts)
    - [Context-specific color](#context-specific-color)
    - [Dual pane](#dual-pane)
  - [Selection](#selection)
  - [Filters](#filters)
  - [Navigate-as-you-type](#navigate-as-you-type)
  - [File indicators](#file-indicators)
- [Mentions](#mentions)
- [Developers](#developers)

## Introduction

`nnn` is a full-featured terminal file manager. It's extremely light and fast (**[performance](https://github.com/jarun/nnn/wiki/Performance)**).

`nnn` is also a disk usage analyzer, a fuzzy app launcher, a batch file renamer and a file picker.

It runs smoothly on the Raspberry Pi, Termux on Android ([demo video](https://www.youtube.com/watch?v=AbaauM7gUJw)), Linux, macOS, BSD, Cygwin and Linux subsystem for Windows. `nnn` works seamlessly with DEs and GUI utilities. It's nearly zero-config (with sensible defaults) and can be setup in less than 5 minutes.

**[Plugins](https://github.com/jarun/nnn/tree/master/plugins)** can be run with custom keybinds. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim). Custom plugins are easy to add.

Visit the **[Wiki](https://github.com/jarun/nnn/wiki)** for how tos, use cases, chronology and insights.

## Features

- Modes
  - Detail (default), light
  - Disk usage analyzer (block/apparent)
  - File picker, (neo)vim plugin
- Navigation
  - *Navigate-as-you-type* with dir auto-select, *wild load*
  - 4 contexts (_aka_ tabs/workspaces)
  - Bookmarks; pin and visit a directory
  - Familiar, easy shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>)
  - Change directory at exit (*easy* shell integration)
- Sorting
  - Ordered pure numeric names by default (visit _/proc_)
  - Case-insensitive version (_aka_ natural) sort
  - Sort by file name, modification time, size, file extension
- Search
  - Instant filtering with *search-as-you-type*
  - Regex and substring match
  - Subtree search to open or edit files (using plugin)
- Mimes
  - Open with desktop opener or specify a custom app
  - Create, list, extract archives
  - Option to open all text files in EDITOR
- Information
  - Detailed file information
  - Media information (needs mediainfo/exiftool)
- Convenience
  - Minimal configuration
  - Plugin keybinds
  - Select files across dirs; all/range selection
  - Copy, move, delete, archive, link selection
  - Batch rename selection or dir entries
  - FreeDesktop compliant trash (needs trash-cli)
  - SSHFS mounts (needs sshfs)
  - Create, rename, duplicate files and directories
  - Per-context directory color (default: blue)
  - Spawn a shell, run apps, run commands, execute file
  - Take quick notes, lock terminal (needs a locker)
  - Shortcut reference a keypress away
- Unicode support
- Follows Linux kernel coding style
- Highly optimized, static analysis integrated code
- Minimal library dependencies
- Widely available

## Installation

#### Library dependencies

`nnn` needs a curses library with wide character support (like ncursesw), libreadline and standard libc. It's possible to drop libreadline using the Makefile target `norl`.

#### Utility dependencies

| Dependency | Installation | Operation |
| --- | --- | --- |
| xdg-open (Linux), open(1) (macOS), cygstart (Cygwin) | base | desktop opener |
| file, coreutils (cp, mv, rm), findutils (xargs) | base | file type, copy, move and remove |
| tar, (un)zip [atool/bsdtar for more formats] | base | create, list, extract tar, gzip, bzip2, zip |
| mediainfo / exiftool | if needed | multimedia file details |
| sshfs, fusermount(3) | if needed | mount, unmount over SSHFS |
| trash-cli | optional | trash files (default action: delete) |
| vlock (Linux), bashlock (macOS), lock(1) (BSD) | optional | terminal locker (fallback: [cmatrix](https://github.com/abishekvashok/cmatrix)) |
| advcpmv (Linux) ([integration](https://github.com/jarun/nnn/wiki#show-cp-mv-progress)) | optional | copy, move progress |
| `$VISUAL` (else `$EDITOR`), `$PAGER`, `$SHELL` | optional | fallback vi, less, sh |

#### From a package manager

`nnn` may be available in the default repos of your distro already. Find a list of known packagers below.

<details><summary>Expand</summary>
<p>
<br>
● <a href="https://pkgs.alpinelinux.org/packages?name=nnn">Alpine Linux</a> (<code>apk add nnn</code>)<br>
● <a href="https://www.archlinux.org/packages/community/x86_64/nnn/">Arch Linux</a> (<code>pacman -S nnn</code>)<br>
● CentOS (<code>yum --enablerepo=epel install nnn</code>)<br>
● <a href="https://crux.nu/portdb/?a=search&q=nnn">CRUX</a> (<code>prt-get depinst nnn</code>)<br>
● <a href="https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1">Debian</a> (<code>apt-get install nnn</code>)<br>
● <a href="https://github.com/DragonFlyBSD/DPorts/tree/master/misc/nnn">DPorts</a> (<code>pkg install nnn</code>)<br>
● <a href="https://apps.fedoraproject.org/packages/nnn">Fedora</a> (<code>dnf install nnn</code>)<br>
● <a href="https://svnweb.freebsd.org/ports/head/misc/nnn/">FreeBDS</a> (<code>pkg install nnn</code>)<br>
● <a href="https://packages.gentoo.org/packages/app-misc/nnn">Gentoo</a> (<code>emerge nnn</code>)<br>
● <a href="http://formulae.brew.sh/formula/nnn">macOS/Homebrew</a> (<code>brew install nnn</code>)<br>
● <a href="https://www.macports.org/ports.php?by=name&substr=nnn">MacPorts</a> (<code>port install nnn</code>)<br>
● <a href="https://notabug.org/milislinux/milis/src/master/talimatname/genel/n/nnn/talimat">Milis Linux</a> (<code>mps kur nnn</code>)<br>
● <a href="https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn">nixpkgs</a> (<code>nix-env -i nnn</code>)<br>
● <a href="https://www.nutyx.org/en/?type=pkg&branch=rolling&arch=x86_64&searchpkg=nnn">NuTyX</a> (<code>cards install nnn</code>)<br>
● <a href="https://cvsweb.openbsd.org/cgi-bin/cvsweb/ports/sysutils/nnn/">OpenBSD</a> (<code>pkg_add nnn</code>)<br>
● <a href="https://software.opensuse.org/package/nnn">openSUSE (and packages for several other distros)</a> (<code>zypper in nnn</code>)<br>
● <a href="http://pkgsrc.se/sysutils/nnn">pkgsrc</a> (<code>pkg_add nnn</code>)<br>
● <a href="https://archive.raspbian.org/raspbian/pool/main/n/nnn/">Raspbian Testing</a> (<code>apt-get install nnn</code>)<br>
● <a href="http://slackbuilds.org/repository/14.2/system/nnn/">Slackware</a> (<code>slackpkg install nnn</code>)<br>
● <a href="http://cook.slitaz.org/index.cgi?pkg=nnn">SliTaz Cooking</a> (<code>cooker pkg nnn</code>)<br>
● <a href="https://packages.getsol.us/shannon/n/nnn/">Solus</a> (<code>eopkg install nnn</code>)<br>
● <a href="http://codex.sourcemage.org/test/shell-term-fm/nnn/">Source Mage</a> (<code>cast nnn</code>)<br>
● <a href="https://github.com/termux/termux-packages/tree/master/packages/nnn">Termux</a> (<code>pkg in nnn</code>)<br>
● <a href="https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1">Ubuntu</a> (<code>apt-get install nnn</code>)<br>
● <a href="https://github.com/void-linux/void-packages/tree/master/srcpkgs/nnn">Void Linux</a> (<code>xbps-install -S nnn</code>)<br>
</p>
</details>

#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora and Ubuntu are available with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

To cook yourself, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Then install the dependencies and compile (e.g. on Ubuntu 16.04):

    $ sudo apt-get install pkg-config libncursesw5-dev libreadline6-dev
    $ make
    $ sudo make strip install

`PREFIX` is supported, in case you want to install to a different location.

- Compilation notes on [Raspberry Pi](https://github.com/jarun/nnn/wiki/Compile-for-Pi)
- Instructions for [Cygwin](https://github.com/jarun/nnn/wiki/Compile-on-Cygwin)

#### Shell completion

Option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`misc/auto-completion/`](misc/auto-completion). Please refer to your shell's manual for installation instructions.

## Usage

#### Configuration

`nnn` supports the following environment variables for configuration. All of them are optional (set if you need). There is no config file. Any associated files are stored under `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/`.

| Example `export` | Description |
| --- | --- |
| `NNN_BMS='d:~/Documents;D:~/Docs archive/'` | key-bookmark pairs [max 10] |
| `NNN_PLUG='o:fzy-open;p:mocplay;m:nmount;t:thumb'` | key-plugin pairs (<kbd>x-key</kbd> to run) [max 10] |
| `NNN_USE_EDITOR=1` | open text files in `$VISUAL` (else `$EDITOR`, fallback vi) |
| `NNN_CONTEXT_COLORS='1234'` | specify per context color [default: '4444' (all blue)] |
| `NNN_SSHFS_OPTS='sshfs -o reconnect,idmap=user'` | specify SSHFS options |
| `NNN_NOTE='/home/user/Dropbox/notes'` | absolute path to note file [default: none] |
| `NNN_OPENER=mimeopen` | custom file opener |
| `NNN_IDLE_TIMEOUT=300` | idle seconds before locking terminal [default: disabled] |
| `NNN_COPIER=copier` | clipboard copier script [default: none] |
| `NNN_TRASH=1` | trash files to the desktop Trash [default: delete] |
| `NNN_OPS_PROG=1` | show cp, mv progress on Linux (needs advcpmv) |

#### Cmdline options

```
usage: nnn [-b key] [-d] [-e] [-H] [-i] [-n] [-o]
           [-p file] [-s] [-S] [-t] [-v] [-w] [-h] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: current dir]

optional args:
 -b key  open bookmark key
 -d      detail mode
 -e      use exiftool for media info
 -H      show hidden files
 -i      nav-as-you-type mode
 -n      version sort
 -o      press Enter to open files
 -p file selection file (stdout if '-')
 -s      string filters [default: regex]
 -S      du mode
 -t      disable dir auto-select
 -v      show version
 -w      wild load
 -h      show help
```

#### Keyboard and mouse

Press <kbd>?</kbd> in `nnn` to see the list anytime.

```
 NAVIGATION
          ↑ k  Up          PgUp ^U  Scroll up
          ↓ j  Down        PgDn ^D  Scroll down
          ← h  Parent dir  ~ ` @ -  HOME, /, start, last
        ↵ → l  Open file/dir     .  Toggle show hidden
         g ^A  First entry    G ^E  Last entry
            b  Pin current dir  ^B  Go to pinned dir
       Tab ^I  Next context      d  Toggle detail view
         , ^/  Lead key    N LeadN  Context N
            /  Filter/Lead  Ins ^T  Toggle nav-as-you-type
          Esc  Exit prompt      ^L  Redraw/clear prompt
            q  Quit context  Lead'  First file
         Q ^Q  Quit  ^G  QuitCD  ?  Help, config
 FILES
           ^O  Open with...      n  Create new/link
            D  File details     ^R  Rename/duplicate
     ⎵ ^K / Y  Select entry/all  r  Batch rename
         K ^Y  Toggle selection  y  List selection
            P  Copy selection    X  Delete selection
            V  Move selection   ^X  Delete entry
            f  Create archive  m M  Brief/full mediainfo
           ^F  Extract archive   F  List archive
            e  Edit in EDITOR    p  Open in PAGER
 ORDER TOGGLES
           ^J  du      E  Extn   S  Apparent du
           ^W  Random  s  Size   t  Time modified
 MISC
         ! ^]  Shell   L  Lock   C  Execute entry
         R ^V  Pick plugin      xK  Run plugin key K
            c  SSHFS mount       u  Unmount
           ^P  Prompt  ^N  Note  =  Launcher
```

Note: Help & settings, file details, media info and archive listing are shown in the PAGER. Use the PAGER-specific keys in these screens.

| Mouse click | Function |
|---| --- |
| Left single on context number | Visit context |
| Left single on top row after context numbers | Visit parent |
| Left single/double on last 2 rows | Toggle nav-as-you-type |
| Left single | Select context or entry |
| Left double | Select context or open entry |

##### Leader key

The Leader/Lead key provides a powerful multi-functional navigation mechanism. It is case-sensitive and understands contexts, bookmarks and location shortcuts.

| Key | Function |
|:---:| --- |
| <kbd>1-4</kbd> | Go to/create selected context |
| <kbd>></kbd>, <kbd>.</kbd> | Go to next active context |
| <kbd><</kbd>, <kbd>,</kbd> | Go to previous active context |
| key | Go to bookmarked location |
| <kbd>'</kbd> | Go to first file in directory |
| <kbd>~</kbd> <kbd>`</kbd> <kbd>@</kbd> <kbd>-</kbd> | Go to HOME, `/`, start, last visited dir |
| <kbd>q</kbd> | Quit context |

When the filter is on, <kbd>/</kbd> works as an additional Leader key.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

## Concepts

#### Contexts

Contexts serve the purpose of exploring multiple directories simultaneously. 4 contexts are available. The status of the contexts are shown in the top left corner:

- the current context is in reverse
- other active contexts are underlined
- rest are inactive

To switch to a context press the Leader key followed by the context number (1-4).

The first time a context is entered, it copies the state of the last visited context. Each context remembers its last visited directory.

When a context is quit, the next active context is selected. If the last active context is quit, the program quits.

##### Context-specific color

Each context can have its own directory color specified:

    export NNN_CONTEXT_COLORS='1234'
colors: 0-black, 1-red, 2-green, 3-yellow, 4-blue (default), 5-magenta, 6-cyan, 7-white

##### Dual pane

Any number of `nnn` instances can be opened simultaneously using the lightweight terminal multiplexter [`dvtm`](http://www.brain-dump.org/projects/dvtm/). For example, to open 2 instances or a dual pane mode, have the following alias:

    alias n2="dvtm -m '^h' nnn nnn"

Note that the `dvtm` MOD key is redefined to <kbd>^H</kbd> as the default one (<kbd>^G</kbd>) is also an `nnn` shortcut.

#### Selection

Use <kbd>^K</kbd> to select the file under the cursor.

To select multiple files:

- press <kbd>^Y</kbd> to enter selection mode. In this mode it's possible to
  - cherry-pick individual files one by one by pressing <kbd>^K</kbd> on each entry (works across directories and contexts); or,
  - navigate to another file in the same directory to select a range of files
- press <kbd>^Y</kbd> again to save the selection and exit selection mode.

_NOTE:_ If you are on BSD/macOS, please check the [BSD terminal issue](https://github.com/jarun/nnn#bsd-terminal-issue) with <kbd>^Y</kbd> for workaround.

Selected files are visually indicated by a `+` before the entries.

The selection can now be listed, copied, moved, removed, archived or linked.

Navigate to a target directory then use <kbd>V</kbd> (move) or <kbd>P</kbd> (copy) to have the selected files moved or copied.

Absolute paths of the selected files are copied to the temporary file `.selection` in the config directory. The path is shown in the help and configuration screen. If `$NNN_COPIER` is set the file paths are also copied to the system clipboard.

#### Filters

Filters support regexes (default) to instantly (search-as-you-type) list the matching entries in the current directory.

Common use cases:
- to list all matches starting with the filter expression, start the expression with a `^` (caret) symbol
- type `\.mkv` to list all MKV files
- use `.*` to match any character (_sort of_ fuzzy search)

There is a program option to filter entries by substring match instead of regex.

#### Navigate-as-you-type

In this mode directories are opened in filter mode, allowing continuous navigation. Works best with the **arrow keys**.

When there's a unique match and it's a directory, `nnn` auto selects the directory and enters it in this mode. Use the relevant program option to disable this behaviour.

This mode takes navigation to the next level when short, unique keypress sequences are possible. For example, to reach `nnn` development directory (located at `~/GitHub/nnn`) from my `$HOME` (which is the default directory the terminal starts in), I use the sequence <kbd>g</kbd><kbd>n</kbd>.

The **_wild load_** option can be extremely handy for users who use this mode constantly. The entries are unsorted when the directory loads. Applying filters sorts the entries (with directories on top). Directory color is disabled in this mode.

#### File indicators

The following indicators are used in the detail view:

| Indicator | File Type |
|:---:| --- |
| `/` | Directory |
| `*` | Executable |
| <code>&#124;</code> | Fifo |
| `=` | Socket |
| `@` | Symbolic Link |
| `b` | Block Device |
| `c` | Character Device |
| `?` | Unknown |

## Mentions

- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [Hacker News 1](https://news.ycombinator.com/item?id=18520898)
- [Hacker News 2](https://news.ycombinator.com/item?id=19850656)
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [LinuxLinks1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)
- [LinuxLinks2](https://www.linuxlinks.com/bestconsolefilemanagers/)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)

## Developers

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2019 [Arun Prakash Jana](https://github.com/jarun)

Contributions are welcome. Please visit the [ToDo list](https://github.com/jarun/nnn/issues/324).
