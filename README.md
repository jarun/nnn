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

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

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
- [Mentions](#mentions)
- [Developers](#developers)

## Introduction

`nnn` is a full-featured terminal file manager. It's extremely light and fast (**[performance](https://github.com/jarun/nnn/wiki/Performance)**).

`nnn` is also a disk usage analyzer, a fuzzy app launcher, a batch file renamer and a file picker.

It runs smoothly on the Raspberry Pi, Termux on Android ([demo video](https://www.youtube.com/watch?v=AbaauM7gUJw)), Linux, macOS, BSD, Cygwin and Linux subsystem for Windows. `nnn` works seamlessly with DEs and GUI utilities. It's nearly zero-config (with sensible defaults) and can be setup in less than 5 minutes.

**[Plugins](https://github.com/jarun/nnn/tree/master/plugins)** can be run with custom keybinds. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim). Custom plugins are easy to add.

Visit the **[Wiki](https://github.com/jarun/nnn/wiki)** for operational concepts, how tos, use cases, chronology and insights.

## Features

- Modes
  - Detail (default), light
  - Disk usage analyzer (block/apparent)
  - File picker, (neo)vim plugin
- Navigation
  - *Navigate-as-you-type* with dir auto-select
  - 4 contexts (_aka_ tabs/workspaces)
  - Bookmarks; pin and visit a directory
  - Familiar, easy shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>)
  - Change directory at exit (*easy* shell integration)
- Sorting
  - Ordered pure numeric names by default (visit _/proc_)
  - Case-insensitive version (_aka_ natural) sort
  - By file name, modification/access time, size, extension
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
  - Media information (using plugin)
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
| sshfs, fusermount(3) | if needed | mount, unmount over SSHFS |
| trash-cli | optional | trash files (default action: delete) |
| vlock (Linux), bashlock (macOS), lock(1) (BSD) | optional | terminal locker (fallback: [cmatrix](https://github.com/abishekvashok/cmatrix)) |
| advcpmv (Linux) ([integration](https://github.com/jarun/nnn/wiki/Advanced-use-cases#show-cp-mv-progress)) | optional | copy, move progress |
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

- Compilation notes on [Raspberry Pi](https://github.com/jarun/nnn/wiki/Developer-guides#compile-for-pi)
- Instructions for [Cygwin](https://github.com/jarun/nnn/wiki/Developer-guides#compile-on-cygwin)

#### Shell completion

Option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`misc/auto-completion/`](misc/auto-completion). Please refer to your shell's manual for installation instructions.

## Usage

#### Configuration

`nnn` supports the following environment variables for configuration. All of them are optional (set if you need). There is no config file. Associated files are stored under `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/`.

| Example `export` | Description |
| --- | --- |
| `NNN_BMS='d:~/Documents;D:~/Docs archive/'` | key-bookmark pairs [max 10] |
| `NNN_PLUG='o:fzy-open;p:mocplay;m:nmount;t:thumb'` | key-plugin pairs (<kbd>x-key</kbd> to run) [max 10] |
| `NNN_USE_EDITOR=1` | open text files in `$VISUAL` (else `$EDITOR`, fallback vi) |
| `NNN_CONTEXT_COLORS='1234'` | specify per context color [default: '4444' (all blue)] |
| `NNN_SSHFS_OPTS='sshfs -o reconnect,idmap=user'` | specify SSHFS options |
| `NNN_NOTE='/home/user/Dropbox/notes'` | absolute path to note file [default: none] |
| `NNN_OPENER=mimeopen` | custom file opener |
| `NNN_IDLE_TIMEOUT=300` | idle seconds to lock terminal [default: disabled] |
| `NNN_COPIER=copier` | clipboard copier script [default: none] |
| `NNN_TRASH=1` | trash files to the desktop Trash [default: delete] |

#### Cmdline options

```
usage: nnn [-a] [-b key] [-d] [-f] [-H] [-i] [-n] [-o]
           [-p file] [-r] [-s] [-S] [-t] [-v] [-h] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: current dir]

optional args:
 -a      use access time
 -b key  open bookmark key
 -d      detail mode
 -f      run filter as cmd on prompt key
 -H      show hidden files
 -i      nav-as-you-type mode
 -n      version sort
 -o      press Enter to open files
 -p file selection file (stdout if '-')
 -r      show cp, mv progress on Linux
 -s      string filters [default: regex]
 -S      du mode
 -t      disable dir auto-select
 -v      show version
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
          Esc  Exit prompt   ^L F5  Redraw/clear prompt
            q  Quit context  Lead'  First file
         Q ^Q  Quit  ^G  QuitCD  ?  Help, config
 FILES
           ^O  Open with...      n  Create new/link
            D  File detail   ^R F2  Rename/duplicate
     ⎵ ^K / Y  Select entry/all  r  Batch rename
         K ^Y  Toggle selection  y  List selection
            P  Copy selection    X  Delete selection
            V  Move selection   ^X  Delete entry
            f  Create archive    C  Execute entry
           ^F  Extract archive   F  List archive
            e  Edit in EDITOR    p  Open in PAGER
 ORDER TOGGLES
           ^J  du                S  Apparent du
            s  Size    E  Extn   t  Time modified
 MISC
         ! ^]  Shell   ^N  Note  L  Lock
         R ^V  Pick plugin  F12 xK  Run plugin key K
            c  SSHFS mount       u  Unmount
           ^P  Prompt            =  Launcher
```

Note: Help & settings, file details and archive listing are shown in the PAGER. Use the PAGER-specific keys in these screens.

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
| <kbd>]</kbd> | Go to next active context |
| <kbd>[</kbd> | Go to previous active context |
| key | Go to bookmarked location |
| <kbd>'</kbd> | Go to first file in directory |
| <kbd>~</kbd> <kbd>`</kbd> <kbd>@</kbd> <kbd>-</kbd> | Go to HOME, `/`, start, last visited dir |
| <kbd>.</kbd> | Toggle show hidden files |
| <kbd>q</kbd> | Quit context |

When the filter is on, <kbd>/</kbd> works as an additional Leader key.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

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

Contributions are welcome. Please visit the [ToDo list](https://github.com/jarun/nnn/issues/329).
