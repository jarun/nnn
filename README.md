<h2 align="center">nnn (<i>type less, do more, way faster</i>)</h2>

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
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

<p align="center">
<a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/onpq3vP.png" /></a>
</p>

<p align="center"><i>Modes of nnn (light with filter, detail, du analyzer) with memory usage (click for a demo video)</i></a></p>

## ToC

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
- [Concepts](#concepts)
  - [Contexts](#contexts)
    - [Context-specific color](#context-specific-color)
    - [Dual pane](#dual-pane)
  - [Selection](#selection)
  - [Filters](#filters)
  - [Navigate-as-you-type](#navigate-as-you-type)
  - [File indicators](#file-indicators)
  - [Hot-plugged drives](#hot-plugged-drives)
  - [Help](#help)
- [Troubleshooting](#troubleshooting)
  - [Tmux configuration](#tmux-configuration)
  - [BSD terminal issue](#bsd-terminal-issue)
  - [100% CPU usage](#100-cpu-usage)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)

## Introduction

`nnn` is a full-featured file manager for low-end devices and the regular desktop. It's extremely light and fast (**[performance](https://github.com/jarun/nnn/wiki/performance)**).

`nnn` is also a disk usage analyzer, a fuzzy app launcher, a batch file renamer and a file picker. Many **[plugins](https://github.com/jarun/nnn/tree/master/plugins)** are available to extend its power. Custom plugins are easy to add. There's an independent [(neo)vim picker plugin](https://github.com/mcchrish/nnn.vim) project.

It runs on Linux, macOS, Raspberry Pi, BSD, Cygwin, Linux subsystem for Windows and Termux on Android. `nnn` works seamlessly with DEs and GUI utilities. It's nearly zero-config (with sensible defaults) and can be setup in less than 5 minutes.

Visit the **[wiki](https://github.com/jarun/nnn/wiki)** for more information on configuration, operational notes and tips.

Here's a video of [`nnn` on Termux (Android)](https://www.youtube.com/watch?v=AbaauM7gUJw).

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
- Plugins
  - (Un)mount external drives
  - File and directory diff
  - Play selection in MOC
  - View PDF as text
  - View images/thumbnails in terminal
  - Upload image to Imgur
  - Paste text to Ubuntu pastebin
  - Upload file to transfer.sh
  - Split and join files
  - Create MP3 ringtones
  - Calculate and verify checksums
  - Read a text file
  - Hex viewer
  - and more...
- Convenience
  - Needs minimal configuration
  - Select files across dirs; all/range selection
  - Copy, move, delete, archive, link selection
  - Batch rename selection or dir entries
  - FreeDesktop compliant trash (needs trash-cli)
  - SSHFS mounts (needs sshfs)
  - Mouse support
  - Create, rename, duplicate files and directories
  - Show copy, move progress on Linux (needs avdcpmv)
  - Per-context directory color (default: blue)
  - Spawn a shell in the current directory
  - Launch applications, run a command
  - Run current file as executable
  - Change directory at exit (*easy* shell integration)
  - Edit file in EDITOR or open in PAGER
  - Take quick notes
  - Lock the terminal (needs a locker)
  - Shortcut reference a keypress away
- Unicode support
- Follows Linux kernel coding style
- Highly optimized, static analysis integrated code
- Minimal library dependencies
- Available on many distros

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

There's a good chance `nnn` is already available in the default repos of your distro. Find a list of known packagers below.

- [Alpine Linux](https://pkgs.alpinelinux.org/packages?name=nnn) (`apk add nnn`)
- [Arch Linux](https://www.archlinux.org/packages/community/x86_64/nnn/) (`pacman -S nnn`)
- [CRUX](https://crux.nu/portdb/?a=search&q=nnn) (`prt-get depinst nnn`)
- [Debian](https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [DPorts](https://github.com/DragonFlyBSD/DPorts/tree/master/misc/nnn) (`pkg install nnn`)
- [Fedora](https://apps.fedoraproject.org/packages/nnn) (`dnf install nnn`)
- [FreeBSD](https://svnweb.freebsd.org/ports/head/misc/nnn/) (`pkg install nnn`)
- [Gentoo](https://packages.gentoo.org/packages/app-misc/nnn) (`emerge nnn`)
- [macOS/Homebrew](http://formulae.brew.sh/formula/nnn) (`brew install nnn`)
- [MacPorts](https://www.macports.org/ports.php?by=name&substr=nnn) (`port install nnn`)
- [Milis Linux](https://notabug.org/milislinux/milis/src/master/talimatname/genel/n/nnn/talimat) (`mps kur nnn`)
- [nixpkgs](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn) (`nix-env -i nnn`)
- [NuTyX](https://www.nutyx.org/en/?type=pkg&branch=rolling&arch=x86_64&searchpkg=nnn) (`cards install nnn`)
- [OpenBSD](https://cvsweb.openbsd.org/cgi-bin/cvsweb/ports/sysutils/nnn/) (`pkg_add nnn`)
- [openSUSE](https://software.opensuse.org/package/nnn) (and packages for several other distros) (`zypper in nnn`)
- [pkgrsc](http://pkgsrc.se/sysutils/nnn) (`pkg_add nnn`)
- [Raspbian Testing](https://archive.raspbian.org/raspbian/pool/main/n/nnn/) (`apt-get install nnn`)
- [Slackware](http://slackbuilds.org/repository/14.2/system/nnn/) (`slackpkg install nnn`)
- [Solus](https://packages.getsol.us/shannon/n/nnn/) (`eopkg install nnn`)
- [Source Mage](http://codex.sourcemage.org/test/shell-term-fm/nnn/) (`cast nnn`)
- [Termux](https://github.com/termux/termux-packages/tree/master/packages/nnn) (`pkg in nnn`)
- [Ubuntu](https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Void Linux](https://github.com/void-linux/void-packages/tree/master/srcpkgs/nnn) (`xbps-install -S nnn`)

#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora and Ubuntu are available with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

To cook yourself, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Then install the dependencies and compile (e.g. on Ubuntu 16.04):

    $ sudo apt-get install pkg-config libncursesw5-dev libreadline6-dev
    $ make
    $ sudo make strip install

`PREFIX` is supported, in case you want to install to a different location.

- Compilation notes for [Raspberry Pi](https://github.com/jarun/nnn/issues/182)
- Instructions for [Cygwin](https://github.com/jarun/nnn/wiki/Cygwin-instructions)

#### Shell completion

Option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`misc/auto-completion/`](misc/auto-completion). Please refer to your shell's manual for installation instructions.

## Usage

#### Configuration

`nnn` supports the following environment variables for configuration. All of them are optional (set if you need). There is no config file. Any associated files are stored under `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/`.

| Example `export` | Description |
| --- | --- |
| `NNN_BMS='d:~/Documents;D:~/Docs archive/'` | specify bookmarks (max 10) |
| `NNN_USE_EDITOR=1` | open text files in `$VISUAL` (else `$EDITOR`, fallback vi) |
| `NNN_CONTEXT_COLORS='1234'` | specify per context color [default: '4444' (all blue)] |
| `NNN_SSHFS_OPTS='sshfs -o reconnect,idmap=user'` | specify SSHFS options |
| `NNN_NOTE='/home/user/Dropbox/notes'` | absolute path to note file [default: none] |
| `NNN_OPENER=mimeopen` | custom file opener |
| `NNN_IDLE_TIMEOUT=300` | idle seconds before locking terminal [default: disabled] |
| `NNN_COPIER=copier` | clipboard copier script [default: none] |
| `NNN_NO_AUTOSELECT=1` | do not auto-select matching dir in _nav-as-you-type_ mode |
| `NNN_RESTRICT_NAV_OPEN=1` | open files on <kbd> ↵</kbd>, not <kbd>→</kbd> or <kbd>l</kbd> |
| `NNN_TRASH=1` | trash files to the desktop Trash [default: delete] |
| `NNN_OPS_PROG=1` | show copy, move progress on Linux |

#### Cmdline options

```
usage: nnn [-b key] [-d] [-e] [-H] [-i] [-n]
           [-p file] [-s] [-S] [-v] [-w] [-h] [PATH]

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
 -p file selection file (stdout if '-')
 -s      string filters [default: regex]
 -S      du mode
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
         ! ^]  Spawn SHELL       C  Execute entry
         R ^V  Pick plugin       L  Lock terminal
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

When there's a unique match and it's a directory, `nnn` auto selects the directory and enters it in this mode. To disable this behaviour,

    export NNN_NO_AUTOSELECT=1

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
| `@/` | Symbolic Link to directory |
| `b` | Block Device |
| `c` | Character Device |
| `?` | Unknown |

#### Hot-plugged drives

External storage devices can be (un)mounted using the plugin [nmount](https://github.com/jarun/nnn/blob/master/plugins/nmount).

For auto-mounting external storage drives use udev rules or udisks wrappers.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

## Troubleshooting

##### Tmux configuration

`nnn` might not handle keypresses correctly when used with tmux (see issue #104 for more details). Set `TERM=xterm-256color` to address it.

##### BSD terminal issue

TLDR: Use the keybind <kbd>K</kbd> to toggle selection if you are having issues with <kbd>^Y</kbd>.

By default in OpenBSD & FreeBSD (and probably on macOS as well), `stty` maps <kbd>^Y</kbd> to `DSUSP`. This means that typing <kbd>^Y</kbd> will suspend `nnn` as if you typed <kbd>^Z</kbd> (you can bring `nnn` back to the foreground by issuing `fg`) instead of entering multi-selection mode. You can check this with `stty -a`. If it includes the text `dsusp = ^Y`, issuing `stty dsusp undef` will disable this `DSUSP` and let `nnn` receive the <kbd>^Y</kbd> instead.

##### 100% CPU usage

There is a known issue where if you close the terminal directly with `nnn` **_waiting for a spawned process_**, a deadlock occurs and `nnn` uses 100% CPU. Please see issue [#225](https://github.com/jarun/nnn/issues/225) for more details. Make sure you quit the spawned process before closing the terminal. It's not a problem if there is no spawned process (`nnn` isn't blocked) as `nnn` checks if the parent process has exited.

## Why fork?

`nnn` was initially forked from [noice](http://git.2f30.org/noice/) but is significantly [different](https://github.com/jarun/nnn/wiki/nnn-vs.-noice) today. I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. `noice` was rudimentary. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind. Check out the [design considerations](https://github.com/jarun/nnn/wiki/design-considerations) for more details.

Trivia: The name `nnn` is a recursive acronym for the initial words from _Noice is Not Noice, a noicer fork..._, suggested by a longtime friend.

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

Contributions are welcome. Please visit the [ToDo list](https://github.com/jarun/nnn/issues/306).
