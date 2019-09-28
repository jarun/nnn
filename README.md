<h1 align="center">nnn - <i>type less, do more, way faster</i></h1>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://repology.org/metapackage/nnn"><img src="https://repology.org/badge/tiny-repos/nnn.svg" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

<p align="center"><img src="https://i.imgur.com/MPWpmos.png" /></p>
<p align="center"><i>navigate-as-you-type & du analyzer mode</i></p>

## Index

- [Introduction](#introduction)
- [Wiki, resources](#wiki-resources)
- [Features](#features)
- [Quickstart](#quickstart)
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

`nnn` is a full-featured terminal file manager. It's tiny, extremely light and fast.

`nnn` is also a disk usage analyzer, a fuzzy app launcher, a batch file renamer and a file picker. 30+ plugins extend the capabilities further. Custom plugins are easy to add.

If you prefer a lightweight system or productivity-boosting utilities, `nnn` is a delight to have. It's nearly zero-config (with sensible defaults) and can be setup in less than 5 minutes.

It runs smoothly on the Raspberry Pi, Termux on Android, Linux, macOS, BSD, Cygwin and Linux subsystem for Windows. `nnn` works seamlessly with DEs and GUI utilities.

## Wiki, resources

1. [Wiki](https://github.com/jarun/nnn/wiki) (concepts, how-tos, use cases, chronology and insights)
2. [Performance numbers](https://github.com/jarun/nnn/wiki/Performance)
3. [Plugin repository and docs](https://github.com/jarun/nnn/tree/master/plugins)
4. [(neo)vim plugin](https://github.com/mcchrish/nnn.vim)
5. [Demo video](https://www.youtube.com/watch?v=U2n5aGqou9E) (v2.1)
6. [nnn on Android](https://www.youtube.com/watch?v=AbaauM7gUJw)

## Features

- Modes
  - Light (default), detail
  - Disk usage analyzer (block/apparent)
  - File picker, (neo)vim plugin
- Navigation
  - *Navigate-as-you-type* with dir auto-select
  - Contexts (_aka_ tabs/workspaces) with configurable colors
  - Bookmarks; pin and visit a directory
  - Familiar shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>) with quick reference
  - CD on quit (*easy* shell integration)
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
  - Lots of plugins with configurable keybinds
  - FreeDesktop compliant trash (needs trash-cli)
  - SSHFS mounts (needs sshfs)
  - Cross-directory file selection, all/range selection
  - Batch rename selection or dir entries
  - Copy, move, delete, archive, link selection
  - Create, rename, duplicate files and directories
  - Spawn a shell, run apps, run commands, execute file
  - Lock terminal (needs a locker)
- Minimal deps, minimal config
- Available on many package managers
- Unicode support
- Follows Linux kernel coding style
- Highly optimized, static analysis integrated code

## Quickstart

While we strongly advise reading this page and the wiki, if you can't wait to start:

1. Install the [utilities you may need](https://github.com/jarun/nnn#utility-dependencies) based on your regular workflows.
2. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
3. Optionally open all text files in `$EDITOR` (fallback vi): `export NNN_USE_EDITOR=1`
4. For additional functionality [install plugins](https://github.com/jarun/nnn/tree/master/plugins#installing-plugins) and the GUI app launcher [`nlaunch`](https://github.com/jarun/nnn/tree/master/misc/nlaunch).

Notes:

1. Don't memorize keys. Arrows, <kbd>/</kbd> and <kbd>q</kbd> suffice. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.
2. To set `nnn` as the default file manager, follow these [instructions](https://github.com/jarun/nnn/wiki/Advanced-use-cases#configure-as-default-fm).

## Installation

#### Library dependencies

A curses library with wide char support (e.g. ncursesw), libreadline and standard libc. Makefile target `norl` drops libreadline.

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

Install `nnn` from your package manager. If the version available is dated try an alternative installation method.

<details><summary>Packaging status from Repology (expand)</summary>
<p>
<br>
<a href="https://repology.org/metapackage/nnn/versions"><img src="https://repology.org/badge/vertical-allrepos/nnn.svg" alt="Packaging status"></a>
</p>
Unlisted distros:
<p>
<br>
● CentOS (<code>yum --enablerepo=epel install nnn</code>)<br>
● <a href="https://notabug.org/milislinux/milis/src/master/talimatname/genel/n/nnn/talimat">Milis Linux</a> (<code>mps kur nnn</code>)<br>
● <a href="https://www.nutyx.org/en/?type=pkg&branch=rolling&arch=x86_64&searchpkg=nnn">NuTyX</a> (<code>cards install nnn</code>)<br>
● <a href="http://codex.sourcemage.org/test/shell-term-fm/nnn/">Source Mage</a> (<code>cast nnn</code>)<br>
</p>
</details>

#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora and Ubuntu are auto-generated with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

Download the latest stable release or clone this repository (*risky*), install deps and compile. On Ubuntu 16.04:

    $ sudo apt-get install pkg-config libncursesw5-dev libreadline6-dev
    $ make
    $ sudo make strip install

`PREFIX` is supported, in case you want to install to a different location.

Visit the [developer guides](https://github.com/jarun/nnn/wiki/Developer-guides) for compilation notes on the Pi, Cygwin and other compilation modes.

#### Shell completion

Completion scripts for Bash, Fish and Zsh are [available](misc/auto-completion). Refer to your shell's manual for installation instructions.

## Usage

#### Configuration

There is no config file. Associated files are stored under `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/`.

`nnn` supports the following optional (set if you need) environment variables.

| Example `export` | Description |
| --- | --- |
| `NNN_BMS='d:~/Documents;D:~/Docs archive/'` | key-bookmark pairs [max 10] |
| `NNN_PLUG='o:fzy-open;p:mocplay;m:nmount;t:thumb'` | key-plugin pairs (<kbd>x-key</kbd> to run) [max 10] |
| `NNN_USE_EDITOR=1` | open text files in `$VISUAL` (else `$EDITOR`, fallback vi) |
| `NNN_CONTEXT_COLORS='1234'` | specify per context color [default: '4444' (all blue)] |
| `NNN_SSHFS_OPTS='sshfs -o reconnect,idmap=user'` | specify SSHFS options |
| `NNN_OPENER=mimeopen` | custom file opener |
| `NNN_IDLE_TIMEOUT=300` | idle seconds to lock terminal [default: disabled] |
| `NNN_COPIER=copier` | clipboard copier script [default: none] |
| `NNN_TRASH=1` | trash files to the desktop Trash [default: delete] |

#### Cmdline options

```
usage: nnn [OPTIONS] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: .]

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
            ?  Help, config  Lead'  First file
         Q ^Q  Quit  ^G  QuitCD  q  Quit context
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
            A  Apparent du       S  du
            s  Size    E  Extn   t  Time
 MISC
         ! ^]  Shell             =  Launcher
         R ^V  Pick plugin   :K xK  Run plugin key K
            c  SSHFS mount       u  Unmount
           ^P  Prompt/run cmd    L  Lock
```

Note: Help & settings, file details and archive listing are shown in the `$PAGER`.

| Mouse click | Function |
|---| --- |
| Left single on context number | Visit context |
| Left single on top row after context numbers | Visit parent |
| Left single/double on last 2 rows | Toggle nav-as-you-type |
| Left single | Select context or entry |
| Left double | Select context or open entry |

#### Leader key

The Leader/Lead key provides a powerful multi-functional navigation mechanism. It is case-sensitive and understands contexts, bookmarks and location shortcuts.

| Key | Function |
|:---:| --- |
| <kbd>1-4</kbd> | Go to/create selected context |
| <kbd>]</kbd> | Go to next active context |
| <kbd>[</kbd> | Go to previous active context |
| key | Go to bookmarked location |
| <kbd>'</kbd> | Go to first file in directory |
| <kbd>~</kbd> <kbd>`</kbd> <kbd>@</kbd> <kbd>-</kbd> | Visit HOME, `/`, start, last visited dir |
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

- Copyright © 2016-2019 [Arun Prakash Jana](https://github.com/jarun)
- Copyright © 2014-2016 Lazaros Koromilas
- Copyright © 2014-2016 Dimitris Papastamos

Contributions are welcome. Head to the [ToDo list](https://github.com/jarun/nnn/issues/332).
