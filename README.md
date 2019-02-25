## nnn

Noice is Not Noice, a noicer fork...

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
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg" alt="Build Status" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
</p>

[![nnn video](https://i.imgur.com/ZB5UdQ8.jpg)](https://www.youtube.com/watch?v=U2n5aGqou9E "Click to see nnn in action!")

<p align="center"><i>nnn in action! (Thanks Luke Smith for the video!)</i></a></p>

`nnn` is smooth... like butter. It's also probably the [fastest and most lightweight](#comparison) file manager you have ever used.

`nnn` integrates seamlessly with your DE and favourite GUI utilities, has a unique _navigate-as-you-type_ mode with auto-select, disk usage analyzer mode, bookmarks, contexts, application launcher, familiar navigation shortcuts, subshell spawning, quick notes and much more.

Integrate utilities like sxiv (view images in directory) or fzy (fuzzy search subtree) easily, transfer selected files using lftp or use it as a (neo)vim plugin; `nnn` supports as many scripts as you need! Refer to the [How to](https://github.com/jarun/nnn/wiki/How-to) section on wiki for more details.

It runs on Linux, macOS, Raspberry Pi, BSD, Cygwin, Linux subsystem for Windows and Termux on Android.

[Quickstart](#quickstart) and see how `nnn` simplifies those long desktop sessions.

*Love smart and efficient utilities? Explore [my repositories](https://github.com/jarun?tab=repositories). Buy me a cup of coffee if they help you.*

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

#### TABLE OF CONTENTS

- [Features](#features)
- [Comparison](#comparison)
- [Installation](#installation)
  - [Library dependencies](#library-dependencies)
  - [Utility dependencies](#utility-dependencies)
  - [From a package manager](#from-a-package-manager)
  - [Release packages](#release-packages)
  - [From source](#from-source)
  - [Shell completion](#shell-completion)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Leader key](#leader-key)
  - [Contexts](#contexts)
    - [Context-specific color](#context-specific-color)
  - [Selection](#selection)
  - [Filters](#filters)
  - [Navigate-as-you-type](#navigate-as-you-type)
  - [File indicators](#file-indicators)
  - [Configuration](#configuration)
  - [Help](#help)
- [Quickstart](#quickstart)
- [How to](#how-to)
- [Troubleshooting](#troubleshooting)
  - [Tmux configuration](#tmux-configuration)
  - [BSD terminal issue](#bsd-terminal-issue)
  - [Restrict file open](#restrict-file-open)
  - [Restrict 0-byte files](#restrict-0-byte-files)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)
- [Contributions](#contributions)

#### FEATURES

- Modes
  - Basic, detail (default), disk usage analyzer (du)
  - File picker, vim (or neovim) plugin
- Navigation
  - Contexts (_aka_ tabs _aka_ workspaces)
  - *Navigate-as-you-type* with auto-select directory
  - Bookmarks
  - Familiar, easy shortcuts (arrows, `~`, `-`, `&`)
  - Pin and visit a directory
- Sorting
  - Directories always listed on top
  - Sort by file name, modification time, size
  - Ordered pure numeric names by default (visit _/proc_)
  - Version (_aka_ natural) sort
- Search
  - Instant filtering with *search-as-you-type*
  - Regex and substring match
- Mimes
  - Open with desktop opener or specify a custom app
  - Create, list, extract archives (needs (p)atool)
  - Open all text files in EDITOR (optional)
- Information
  - Detailed stat-like file information
  - Media information (needs mediainfo/exiftool)
- Convenience
  - Create, rename files and directories
  - Select files across dirs; all/range selection
  - Copy, move, delete, archive selection
  - Show copy, move progress on Linux (needs avdcpmv)
  - Create sym/hard link(s) to selection
  - Transfer files using lftp
  - Batch rename/move/delete (needs vidir)
  - Show directories in custom color (default: blue)
  - Per-context directory color
  - Spawn a subshell in the current directory
  - Run a command, launch applications
  - Run custom scripts in the current directory
  - Repository of custom scripts
  - Run current file as executable
  - Change directory at exit (*easy* shell integration)
  - Edit file in EDITOR or open in PAGER
  - Take quick notes
  - Terminal locker integration
  - Shortcut reference a keypress away
- Unicode support
- Follows Linux kernel coding style
- Highly optimized, static analysis integrated code
- Available on many distros

#### COMPARISON

Stripped binary (or script) size and memory usage of `nnn` and some other similar utilities while viewing a directory with 13.5K files (0 directories), sorted by size/du:

<pre>
<b>BINSZ</b>    VIRT  <b>  RES</b>    SHR S  %MEM   COMMAND
<b> 650K</b>  139720  <b>91220</b>   8460 S   1.1   ranger
<b>   1M</b>   50496  <b>15328</b>   4076 S   0.2   vifm
<b>   1M</b>   72152  <b>12468</b>   7336 S   0.2   mc
<b>  55K</b>   15740  <b> 4348</b>   2460 S   0.1   nnn -S
</pre>

Intrigued? Find out [HOW](https://github.com/jarun/nnn/wiki/performance-factors).

#### INSTALLATION

#### Library dependencies

`nnn` needs a curses library with wide character support (like ncursesw), libreadline and standard libc. It's possible to drop libreadline using the Makefile target `norl`.

#### Utility dependencies

| External dependency | Operation |
| --- | --- |
| xdg-open (Linux), open(1) (macOS), cygstart (Cygwin) | desktop opener |
| file | determine file type |
| cp, mv, rm, xargs (from findutils on Linux)  | copy, move and remove files |
| mediainfo, exiftool | multimedia file details |
| atool, patool ([integration](https://github.com/jarun/nnn/wiki/How-to#integrate-patool)) | create, list and extract archives |
| vidir (from moreutils) | batch rename, move, delete dir entries |
| vlock (Linux), bashlock (macOS), lock(1) (BSD) | terminal locker |
| advcpmv (Linux) ([integration](https://github.com/jarun/nnn/wiki/How-to#show-cp-mv-progress)) | copy, move progress |
| $EDITOR (overridden by $VISUAL, if defined) | edit files (fallback vi) |
| $PAGER (less, most) | page through files (fallback less) |
| $SHELL (single coombined argument) | spawn a shell, run script (fallback sh) |

#### From a package manager

- [Alpine Linux](https://pkgs.alpinelinux.org/packages?name=nnn) (`apk add nnn`)
- [Arch Linux](https://www.archlinux.org/packages/community/x86_64/nnn/) (`pacman -S nnn`)
- [Debian](https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Fedora](https://apps.fedoraproject.org/packages/nnn) (`dnf install nnn`)
- [FreeBSD](https://www.freshports.org/misc/nnn) (`pkg install nnn`)
- [Gentoo](https://packages.gentoo.org/packages/app-misc/nnn) (`emerge nnn`)
- [macOS/Homebrew](http://formulae.brew.sh/formula/nnn) (`brew install nnn`)
- [Milis Linux](https://notabug.org/milislinux/milis/src/master/talimatname/genel/n/nnn/talimat) (`mps kur nnn`)
- [NixOS](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn) (`nix-env -i nnn`)
- [OpenBSD](https://cvsweb.openbsd.org/cgi-bin/cvsweb/ports/sysutils/nnn/) (`pkg_add nnn`)
- [openSUSE](https://software.opensuse.org/package/nnn) (and packages for several other distros) (`zypper in nnn`)
- [pkgrsc](http://pkgsrc.se/sysutils/nnn) (`pkg_add nnn`)
- [Raspbian Testing](https://archive.raspbian.org/raspbian/pool/main/n/nnn/) (`apt-get install nnn`)
- [Slackware](http://slackbuilds.org/repository/14.2/system/nnn/) (`slackpkg install nnn`)
- [Solus](https://packages.getsol.us/shannon/n/nnn/) (`eopkg install nnn`)
- [Source Mage](http://codex.sourcemage.org/test/shell-term-fm/nnn/) (`cast nnn`)
- [Termux](https://github.com/termux/termux-packages/tree/master/packages/nnn) (`pkg in nnn`)
- [Ubuntu](https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/) (`apt-get install nnn`)
- [Void Linux](https://github.com/void-linux/void-packages/tree/master/srcpkgs/nnn) (`xbps-install -S nnn`)


#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora, Solus, and Ubuntu are available with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

To cook yourself, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Then install the dependencies and compile (e.g. on Ubuntu 16.04):

    $ sudo apt-get install pkg-config libncursesw5-dev libreadline6-dev
    $ make
    $ sudo make install

`PREFIX` is supported, in case you want to install to a different location.

- Compilation information for [Raspberry Pi](https://github.com/jarun/nnn/issues/182)
- Instructions for [Cygwin](https://github.com/jarun/nnn/wiki/Cygwin-instructions)

#### Shell completion

Search keyword and option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`scripts/auto-completion/`](scripts/auto-completion). Please refer to your shell's manual for installation instructions.

#### USAGE

#### Cmdline options

```
usage: nnn [-b key] [-C] [-e] [-i] [-l] [-n]
           [-p file] [-s] [-S] [-v] [-h] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: current dir]

optional args:
 -b key  open bookmark key
 -C      disable directory color
 -e      use exiftool for media info
 -i      nav-as-you-type mode
 -l      light mode
 -n      use version compare to sort
 -p file selection file (stdout if '-')
 -s      string filters [default: regex]
 -S      du mode
 -v      show version
 -h      show help
```

`>` indicates the currently selected entry in `nnn`.

#### Keyboard shortcuts

Press <kbd>?</kbd> in `nnn` to see the list anytime.

```
 NAVIGATION
          ↑ k  Up          PgUp ^U  Scroll up
          ↓ j  Down        PgDn ^D  Scroll down
          ← h  Parent dir        ~  Go HOME
        ↵ → l  Open file/dir     &  Start dir
    Home g ^A  First entry       -  Last visited dir
     End G ^E  Last entry        .  Toggle show hidden
            /  Filter       Ins ^T  Toggle nav-as-you-type
            b  Pin current dir  ^B  Go to pinned dir
       Tab ^I  Next context      d  Toggle detail view
         , ^/  Leader key  N LeadN  Enter context N
          Esc  Exit prompt      ^L  Redraw/clear prompt
           ^G  Quit and cd       q  Quit context
         Q ^Q  Quit              ?  Help, config
 FILES
           ^O  Open with...      n  Create new/link
            D  File details     ^R  Rename entry
     ⎵ ^K / Y  Select entry/all  r  Open dir in vidir
         K ^Y  Toggle selection  y  List selection
            P  Copy selection    X  Delete selection
            V  Move selection   ^X  Delete entry
            f  Create archive  m M  Brief/full mediainfo
           ^F  Extract archive   F  List archive
            e  Edit in EDITOR    p  Open in PAGER
 ORDER TOGGLES
           ^J  Disk usage        S  Apparent du
            t  Time modified     s  Size
 MISC
         ! ^]  Spawn SHELL       C  Execute entry
         R ^V  Run/pick script   L  Lock terminal
           ^P  Command prompt   ^N  Take note
```

Help & settings, file details, media info and archive listing are shown in the PAGER. Please use the PAGER-specific keys in these screens.

The option `open with` supports a combined argument.

#### Leader key

The Leader key (<kbd>`</kbd> or <kbd>^/</kbd>) provides a powerful multi-functional navigation mechanism. It is case-sensitive and understands contexts, bookmarks and handy location shortcuts.

| Key | Function |
|:---:| --- |
| <kbd>1-4</kbd> | Go to/create selected context |
| <kbd>></kbd>, <kbd>.</kbd> | Go to next active context |
| <kbd><</kbd>, <kbd>,</kbd> | Go to previous active context |
| key | Go to bookmarked location |
| <kbd>~</kbd> | Go to HOME directory |
| <kbd>-</kbd> | Go to last visited directory |
| <kbd>&</kbd> | Go to start directory |
| <kbd>q</kbd> | Quit context |

#### Contexts

Contexts serve the purpose of exploring multiple directories simultaneously. 4 contexts are available. The status of the contexts are shown in the top left corner:

- the current context is in reverse
- other used contexts are underlined
- rest are unused

To switch to a context press the Leader key followed by the context number (1-4).

The first time a context is entered, it copies the state of the last visited context. Each context remembers its start directory and last visited directory.

When a context is quit, the next active context is selected. If the last active context is quit, the program quits.

##### Context-specific color

Each context can have its own color for directories specified:

    export NNN_CONTEXT_COLORS='1234'
colors: 0-black, 1-red, 2-green, 3-yellow, 4-blue (default), 5-magenta, 6-cyan, 7-white

#### Selection

Use <kbd>^K</kbd> to copy the absolute path of the file under the cursor.

To copy multiple absolute file paths:

- press <kbd>^Y</kbd> (or <kbd>Y</kbd>) to enter selection mode. In this mode it's possible to
  - cherry-pick individual files one by one by pressing <kbd>^K</kbd> on each entry (works across directories and contexts); or,
  - navigate to another file in the same directory to select a range of files
- press <kbd>^Y</kbd> (or <kbd>Y</kbd>) _again_ to copy the paths and exit the selection mode

The files in the list can now be copied (<kbd>P</kbd>), moved (<kbd>V</kbd>) or removed (<kbd>X</kbd>).

To list the file paths copied to memory press <kbd>y</kbd>.

File paths are copied to the temporary file `DIR/.nnncp`, where `DIR` (by priority) is:

    $HOME or,
    $TMPDIR or,
    /tmp

The path is shown in the help and configuration screen.

#### Filters

Filters support regexes by default to instantly (search-as-you-type) list the matching entries in the current directory.

Common use cases:
- to list all matches starting with the filter expression, start the expression with a `^` (caret) symbol
- type `\.mkv` to list all MKV files
- use `.*` to match any character (_sort of_ fuzzy search)

There is a program opton to filter entries by substring match.

If `nnn` is invoked as root or the environment variable `NNN_SHOW_HIDDEN` is set the default filter will also match hidden files.

#### Navigate-as-you-type

In this mode directories are opened in filter mode, allowing continuous navigation. Works best with the **arrow keys**.

In case of only one match and it's a directory, `nnn` auto selects the directory and enters it in this mode. To disable this behaviour,

    export NNN_NO_AUTOSELECT=1

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

#### Configuration

`nnn` supports the following environment variables for configuration.

| Example `export` | Description |
| --- | --- |
| `NNN_BMS='d:~/Documents;D:~/Docs archive/'` | specify bookmarks (max 10) |
| `NNN_OPENER=mimeopen` | custom file opener |
| `NNN_CONTEXT_COLORS='1234'` | specify per context color [default: '4444' (all blue)] |
| `NNN_IDLE_TIMEOUT=300` | idle time to lock terminal [default: disabled] |
| `NNN_COPIER='copier.sh'` | system clipboard copier script [default: none] |
| `NNN_SCRIPT=/home/user/scripts[/script.sh]` | path to script dir or a single script |
| `NNN_NOTE=/home/user/Dropbox/Public/notes` | path to note file [default: none] |
| `NNN_TMPFILE=/tmp/nnn` | file to write current open dir path to for cd on quit |
| `NNN_USE_EDITOR=1` | Open text files in `$EDITOR` (`$VISUAL`, if defined; fallback vi) |
| `NNN_SHOW_HIDDEN=1` | show hidden (.) files [default: do not show hidden if not root ] |
| `NNN_NO_AUTOSELECT=1` | do not auto-select matching dir in _nav-as-you-type` mode |
| `NNN_RESTRICT_NAV_OPEN=1` | open files on <kbd> ↵</kbd>, not <kbd>→</kbd> or <kbd>l</kbd> |
| `NNN_RESTRICT_0B=1` | do not open 0-byte files |
| `NNN_CP_MV_PROG=1` | show copy, move progress on Linux |

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

#### QUICKSTART

1. Install the [utilities required](#utility-dependencies) for your regular activities.
2. Configure [cd on quit](https://github.com/jarun/nnn/wiki/How-to#cd-on-quit).
3. Optionally open all text files in EDITOR (fallback vi):

       export NNN_USE_EDITOR=1
4. Run `n`.
5. Don't memorize keys. Arrows, <kbd>/</kbd> and <kbd>q</kbd> suffice. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.
6. For additional functionality [setup custom scripts](https://github.com/jarun/nnn/wiki/How-to#run-custom-scripts).

#### HOW TO

Please visit the [How to](https://github.com/jarun/nnn/wiki/How-to) wiki page.

#### TROUBLESHOOTING

##### Tmux configuration

`nnn` might not handle keypresses correctly when used with tmux (see issue #104 for more details). Set `TERM=xterm-256color` to address it.

##### BSD terminal issue

By default in OpenBSD & FreeBSD, `stty` maps <kbd>^Y</kbd> to `DSUSP`. This means that typing <kbd>^Y</kbd> will suspend `nnn` as if you typed <kbd>^Z</kbd> (you can bring `nnn` back to the foreground by issuing `fg`) instead of entering multi-copy mode. You can check this with `stty -a`. If it includes the text `dsusp = ^Y`, issuing `stty dsusp undef` will disable this `DSUSP` and let `nnn` receive the <kbd>^Y</kbd> instead.

##### Restrict file open

In order to disable opening files on accidental navigation key (<kbd>→</kbd> or <kbd>l</kbd>) press:

    export NNN_RESTRICT_NAV_OPEN=1

Use <kbd>Enter</kbd> to open files.

##### Restrict 0-byte files

Restrict opening 0-byte files due to [unexpected behaviour](https://github.com/jarun/nnn/issues/187); use _edit_ or _open with_ to open the file.

    export NNN_RESTRICT_0B=1

#### WHY FORK?

`nnn` was initially forked from [noice](http://git.2f30.org/noice/) but is significantly [different](https://github.com/jarun/nnn/wiki/nnn-vs.-noice) today. I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. `noice` was rudimentary. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind. Check out [nnn design considerations](https://github.com/jarun/nnn/wiki/nnn-design-considerations) for more details.

#### MENTIONS

- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [Hacker News](https://news.ycombinator.com/item?id=18520898)
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [LinuxLinks1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)
- [LinuxLinks2](https://www.linuxlinks.com/bestconsolefilemanagers/)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)

#### DEVELOPERS

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2019 [Arun Prakash Jana](https://github.com/jarun)

#### CONTRIBUTIONS

We need contributors. Please visit the [ToDo list](https://github.com/jarun/nnn/issues/213).
