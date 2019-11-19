<h1 align="center">nnn - <i>type less, do more, way faster</i></h1>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://repology.org/metapackage/nnn"><img src="https://repology.org/badge/tiny-repos/nnn.svg" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

<p align="center"><a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/MPWpmos.png" /></a></p>
<p align="center"><i>navigate-as-you-type & du (click to see demo video)</i></p>

## Introduction

`nnn` is a full-featured terminal file manager. It's tiny, [extremely light and fast](https://github.com/jarun/nnn/wiki/Performance).

`nnn` is also a disk usage analyzer, a fuzzy app launcher, a batch file renamer and a file picker. The [plugin repository](https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins) has tons of plugins and plugin-specific documentation to extend the capabilities further. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim).

It runs smoothly on the Raspberry Pi, Termux [on Android](https://www.youtube.com/watch?v=AbaauM7gUJw), Linux, macOS, BSD, Cygwin and Linux subsystem for Windows. `nnn` works seamlessly with DEs and GUI utilities. It's nearly zero-config (with sensible defaults) and can be setup in less than 5 minutes.

Add to that an awesome [Wiki](https://github.com/jarun/nnn/wiki)!

## Features

- Modes
  - Light (default), detail
  - Disk usage analyzer (block/apparent)
  - File picker, (neo)vim plugin
- Navigation
  - *Navigate-as-you-type* with dir auto-select
  - Contexts (_aka_ tabs/workspaces) with custom colors
  - Sessions, bookmarks; pin and visit a dir
  - Familiar shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>), quick reference
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
  - Create, list, extract, mount (FUSE based) archives
  - Option to open all text files in EDITOR
- Information
  - Detailed file information
  - Media information (using plugin)
- Convenience
  - Run plugins and commands with custom keybinds
  - FreeDesktop compliant trash (needs trash-cli)
  - SSHFS mounts (needs sshfs)
  - Cross-dir file/all/range selection
  - Batch rename selection or dir entries
  - Copy (as), move (as), delete, archive, link selection
  - Create (with parents), rename, duplicate (anywhere) files and dirs
  - Spawn a shell, run apps, run commands, execute file
  - Lock terminal (needs a locker)
- Minimal deps, minimal config
- Widely available
- Unicode support
- Follows Linux kernel coding style
- Highly optimized, static analysis integrated code

## Quickstart

1. Install the [utilities you may need](https://github.com/jarun/nnn#utility-dependencies) based on your regular workflows.
2. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
3. Optionally open all text files in `$EDITOR` (fallback vi): `export NNN_USE_EDITOR=1`.
4. For additional functionality [install plugins](https://github.com/jarun/nnn/tree/master/plugins#installing-plugins) and the GUI app launcher [`nlaunch`](https://github.com/jarun/nnn/tree/master/misc/nlaunch).

Notes:

1. Don't memorize keys. Arrows, <kbd>/</kbd> and <kbd>q</kbd> suffice. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.
2. To set `nnn` as the default file manager, follow these [instructions](https://github.com/jarun/nnn/wiki/Advanced-use-cases#configure-as-default-fm).

## Installation

#### Library dependencies

A curses library with wide char support (e.g. ncursesw), libreadline (`make O_NORL=1` to drop) and standard libc.

#### Utility dependencies

| Dependency | Installation | Operation |
| --- | --- | --- |
| xdg-open (Linux), open(1) (macOS), cygstart (Cygwin) | base | desktop opener |
| file, coreutils (cp, mv, rm), findutils (xargs) | base | file type, copy, move and remove |
| tar, (un)zip [atool/bsdtar for more formats] | base | create, list, extract tar, gzip, bzip2, zip |
| archivemount, fusermount(3) | optional | mount, unmount archives |
| sshfs, fusermount(3) | optional | mount, unmount remotes |
| trash-cli | optional | trash files (default action: rm) |
| vlock (Linux), bashlock (macOS), lock(1) (BSD) | optional | terminal locker (fallback: [cmatrix](https://github.com/abishekvashok/cmatrix)) |
| advcpmv (Linux) ([integration](https://github.com/jarun/nnn/wiki/Advanced-use-cases#show-cp-mv-progress)) | optional | copy, move progress |
| `$VISUAL` (else `$EDITOR`), `$PAGER`, `$SHELL` | optional | fallback vi, less, sh |

#### From a package manager

Install `nnn` from your package manager. If the version available is dated try an alternative installation method.

<details><summary>Packaging status (expand)</summary>
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

Download the latest stable release or clone this repository (*risky*), install deps and compile. On Ubuntu 18.04:

    $ sudo apt-get install pkg-config libncursesw5-dev libreadline-dev
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
| `NNN_PLUG='m:nmount;t:thumb;x:_chmod +x $NNN'` | key-plugin (or cmd) pairs (<kbd>:key</kbd> to run) [max 15] |
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
 -c      cli-only opener
 -d      detail mode
 -e name load session by name
 -f      run filter as cmd on prompt key
 -H      show hidden files
 -i      nav-as-you-type mode
 -K      detect key collision
 -n      version sort
 -o      open files on Enter
 -p file selection file [stdout if '-']
 -r      use advcpmv patched cp, mv
 -s      string filters [default: regex]
 -S      du mode
 -t      disable dir auto-select
 -v      show version
 -h      show help
```

#### Keyboard and mouse

The list below is from the **dev branch**. Press <kbd>?</kbd> in `nnn` to see the keybinds in your installed version.

```
 NAVIGATION
         Up k  Up          PgUp ^U  Scroll up
       Down j  Down        PgDn ^D  Scroll down
       Left h  Parent      ~ ` @ -  HOME, /, start, last
  Ret Right l  Open              .  Toggle show hidden
         g ^A  First entry    G ^E  Last entry
            b  Pin current dir  ^B  Go to pinned dir
      (Sh)Tab  Cycle context     d  Toggle detail view
         , ^/  Lead key    N LeadN  Context N
            /  Filter/Lead  Ins ^N  Toggle nav-as-you-type
          Esc  Exit prompt   ^L F5  Redraw/clear prompt
            ?  Help, conf  ' Lead'  First file
         Q ^Q  Quit  ^G  QuitCD  q  Quit context
 FILES
           ^O  Open with...      n  Create new/link
            D  File details  ^R F2  Rename/duplicate
   Space ^J/a  Select entry/all  r  Batch rename
         m ^K  Sel range, clear  M  List selection
            P  Copy selection    K  Edit, flush sel
            V  Move selection    w  Copy/move sel as
            X  Del selection    ^X  Del entry
            f  Create archive    T  Mount archive
           ^F  Extract archive   F  List archive
            e  Edit in EDITOR    p  Open in PAGER
 ORDER TOGGLES
            A  Apparent du       S  du
            z  Size   E  Extn    t  Time
 MISC
         ! ^]  Shell      ;K :K xK  Execute plugin K
            C  Execute entry  R ^V  Pick plugin
            U  Manage session    =  Launch
            c  SSHFS mount       u  Unmount
         ] ^P  Prompt/run cmd    L  Lock
```

Notes:

1. Help & settings, file details and archive listing are shown in the `$PAGER`.
2. To change shortcuts modify key bindings in `nnn.h` and compile.

| Mouse click | Function |
|---| --- |
| Left single on context number | Visit context |
| Left single on top row after context numbers | Visit parent |
| Left single/double on last 2 rows | Toggle nav-as-you-type |
| Left single | Select context or entry |
| Left double | Select context or open entry |

#### Lead key

The Lead/Leader key provides a powerful multi-functional navigation mechanism. It is case-sensitive and understands contexts, bookmarks and location shortcuts.

| Follower key | Function |
|:---:| --- |
| <kbd>1-4</kbd> | Go to/create selected context |
| key | Go to bookmarked location |
| <kbd>'</kbd> | Go to first file in directory |
| <kbd>~</kbd> <kbd>`</kbd> <kbd>@</kbd> <kbd>-</kbd> | Visit HOME, `/`, start, last visited dir |
| <kbd>.</kbd> | Toggle show hidden files |

When the filter is on, <kbd>/</kbd> works as an additional Lead key.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

## Elsewhere

- [Wikipedia](https://en.wikipedia.org/wiki/Nnn_(file_manager))
- [ArchWiki](https://wiki.archlinux.org/index.php/Nnn)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [gHacks Tech News](https://www.ghacks.net/2019/11/01/nnn-is-an-excellent-command-line-based-file-manager-for-linux-macos-and-bsds/)
- Hacker News [[1](https://news.ycombinator.com/item?id=18520898)] [[2](https://news.ycombinator.com/item?id=19850656)]
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- LinuxLinks [[1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)] [[2](https://www.linuxlinks.com/bestconsolefilemanagers/)]
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)

## Developers

- [Arun Prakash Jana](https://github.com/jarun) (Copyright © 2016-2019)
- [0xACE](https://github.com/0xACE)
- [Anna Arad](https://github.com/annagrram)
- [KlzXS](https://github.com/KlzXS)
- [Sijmen J. Mulder](https://github.com/sjmulder)
- and other contributors

Visit the to the [ToDo list](https://github.com/jarun/nnn/issues/386) to contribute or see the current development status.
