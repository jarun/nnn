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
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg" alt="Build Status" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
</p>

[![nnn video](https://i.imgur.com/pDyq5oa.jpg)](https://www.youtube.com/watch?v=U2n5aGqou9E "Click to see nnn in action!")

<p align="center"><i>nnn in action! (Thanks Luke Smith for the video!)</i></a></p>

`nnn` is probably the [fastest and most resource-sensitive](#comparison) file manager you have ever used. It integrates seamlessly with your DE and favourite GUI utilities, has a unique [navigate-as-you-type](#navigate-as-you-type-mode) mode with auto-select, disk usage analyzer mode, bookmarks, contexts, application launcher, familiar navigation shortcuts, subshell spawning and much more.

[Integrate utilities](https://github.com/jarun/nnn#sample-scripts) like sxiv or fzy easily, transfer selected files using lftp or use it as a [(neo)vim plugin](https://github.com/jarun/nnn#neovim-plugin); `nnn` supports as many scripts as you need!

It runs on Linux, macOS, Raspberry Pi, BSD, Cygwin, Linux subsystem for Windows and Termux.

[Quickstart](#quickstart) and see how `nnn` simplifies those long desktop sessions.

We need contributors. Please visit the [ToDo list](https://github.com/jarun/nnn/issues/137).

*Love smart and efficient utilities? Explore [my repositories](https://github.com/jarun?tab=repositories). Buy me a cup of coffee if they help you.*

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-green.svg" alt="Donate via PayPal!" /></a>
</p>

#### TABLE OF CONTENTS

- [Features](#features)
- [Comparison](#comparison)
- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [From a package manager](#from-a-package-manager)
  - [Release packages](#release-packages)
  - [From source](#from-source)
  - [Shell completion](#shell-completion)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Leader key](#leader-key)
  - [Contexts](#contexts)
  - [Directory color](#directory-color)
  - [Selection](#selection)
  - [Filters](#filters)
  - [Navigate-as-you-type mode](#navigate-as-you-type-mode)
  - [File indicators](#file-indicators)
  - [Utility dependencies](#utility-dependencies)
  - [Help](#help)
- [Quickstart](#quickstart)
- [How to](#how-to)
  - [add bookmarks](#add-bookmarks)
  - [copy file paths](#copy-file-paths)
    - [to clipboard](#to-clipboard)
    - [get selection manually](#get-selection-manually)
  - [cd on quit](#cd-on-quit)
  - [(neo)vim plugin](#neovim-plugin)
  - [file picker](#file-picker)
  - [run custom scripts](#run-custom-scripts)
    - [sample scripts](#sample-scripts)
  - [launch applications](#launch-applications)
  - [change dir color](#change-dir-color)
  - [integrate patool](#integrate-patool)
  - [remote transfers](#remote-transfers)
  - [prompt shortcuts](#prompt-shortcuts)
  - [set idle timeout](#set-idle-timeout)
  - [hot-plugged drives](#hot-plugged-drives)
  - [tmux configuration](#tmux-configuration)
  - [BSD terminal issue](#bsd-terminal-issue)
  - [restrict file open](#restrict-file-open)
  - [restrict 0-byte files](#restrict-0-byte-files)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)

#### FEATURES

- Modes
  - Basic, detail (default), disk usage analyzer (du)
  - Vim (or neovim) file picker (as a plugin)
- Navigation
  - Familiar, easy shortcuts (arrows, `~`, `-`, `&`)
  - *Navigate-as-you-type* with auto-select directory
  - Contexts (_aka_ tabs _aka_ workspaces)
  - Bookmarks
  - Pin and visit a directory
- Sorting
  - Directories always listed on top
  - Sort by file name, modification time, size
  - Numeric order for numeric names (visit _/proc_)
- Search
  - Instant filtering with *search-as-you-type*
- Mimes
  - Open with desktop opener or specify a custom app
  - Create, list, extract archives (needs (p)atool)
  - Open all text files in EDITOR (optional)
- Information
  - Detailed stat-like file information
  - Media information (needs mediainfo/exiftool)
- Convenience
  - Create, rename files and directories
  - Select files across directories
  - Copy, move, delete selection
  - Transfer files using lftp
  - Batch rename/move/delete (needs vidir)
  - Show directories in custom color (default: blue)
  - Spawn a subshell in the current directory
  - Run a command, launch applications
  - Run custom scripts in the current directory
  - Run current file as executable
  - Change directory at exit (*easy* shell integration)
  - Edit file in EDITOR or open in PAGER
  - Terminal locker integration
- Unicode support
- Highly optimized, static analysis integrated code
- Available on many distros

#### COMPARISON

Stripped binary (or script) size and memory usage of `nnn` and some other similar utilities while viewing a directory with 13.5K files (0 directories), sorted by size/du:

<pre>
<b>BINSZ</b>    VIRT  <b>  RES</b>    SHR S  %MEM   COMMAND
<b> 650K</b>  139720  <b>91220</b>   8460 S   1.1   ranger
<b>   1M</b>   50496  <b>15328</b>   4076 S   0.2   vifm
<b>   1M</b>   72152  <b>12468</b>   7336 S   0.2   mc
<b>  70K</b>   16068  <b> 4620</b>   2408 S   0.1   ncdu
<b>  55K</b>   15712  <b> 4368</b>   2512 S   0.1   nnn -S
</pre>

Intrigued? Find out [HOW](https://github.com/jarun/nnn/wiki/performance-factors).

#### INSTALLATION

#### Dependencies

`nnn` needs a curses library with wide character support (like ncursesw) and standard libc.

#### From a package manager

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

    $ sudo apt-get install pkg-config libncursesw5-dev
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
usage: nnn [-b key] [-C] [-e] [-i] [-l]
           [-p file] [-S] [-v] [-h] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: current dir]

optional args:
 -b key  bookmark key to open
 -C      disable directory color
 -e      use exiftool instead of mediainfo
 -i      start in navigate-as-you-type mode
 -l      start in light mode
 -p file copy selection to file (stdout if '-')
 -S      start in disk usage analyser mode
 -v      show program version
 -h      show this help
```

`>` indicates the currently selected entry in `nnn`.

#### Keyboard shortcuts

Press <kbd>?</kbd> in `nnn` to see the list anytime.

```
 NAVIGATION
       ↑, k, ^P  Up           PgUp, ^U  Scroll up
       ↓, j, ^N  Down         PgDn, ^D  Scroll down
 Home, g, ^, ^A  First entry         ~  Go HOME
  End, G, $, ^E  Last entry          &  Start dir
 ←, Bksp, h, ^H  Parent dir          -  Last visited dir
    →, ↵, l, ^M  Open file/dir       .  Toggle show hidden
              /  Filter        Ins, ^T  Toggle nav-as-you-type
              b  Pin current dir    ^W  Go to pinned dir
        Tab, ^I  Next context        d  Toggle detail view
          `, ^/  Leader key    LeaderN  Go to/create context N
            Esc  Exit prompt        ^L  Redraw/clear prompt
             ^G  Quit and cd         q  Quit context
          Q, ^Q  Quit                ?  Help, config
 FILES
             ^O  Open with...        n  Create new
              D  File details       ^R  Rename entry
          ⎵, ^K  Copy entry path     r  Open dir in vidir
          Y, ^Y  Toggle selection    y  List selection
              P  Copy selection      X  Delete selection
              V  Move selection     ^X  Delete entry
              f  Archive entry       F  List archive
             ^F  Extract archive  m, M  Brief/full media info
              e  Edit in EDITOR      p  Open in PAGER
 ORDER TOGGLES
             ^J  Disk usage          S  Apparent du
              t  Modification time   s  Size
 MISC
          !, ^]  Spawn SHELL in dir  C  Execute entry
          R, ^V  Run custom script   L  Lock terminal
             ^S  Run a command
```

Help & settings, file details, media info and archive listing are shown in the PAGER. Please use the PAGER-specific keys in these screens.

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

#### Directory color

Each context can have its own color for directories specified:

    export NNN_CONTEXT_COLORS="1234"
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

Filters support regexes to instantly (search-as-you-type) list the matching entries in the current directory.

Common use cases:
- to list all matches starting with the filter expression, start the expression with a `^` (caret) symbol
- type `\.mkv` to list all MKV files
- use `.*` to match any character (_sort of_ fuzzy search)

If `nnn` is invoked as root or the environment variable `NNN_SHOW_HIDDEN` is set the default filter will also match hidden files.

#### Navigate-as-you-type mode

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

#### Utility dependencies

| External dependency | Operation |
| --- | --- |
| xdg-open (Linux), open(1) (macOS), cygstart (Cygwin) | desktop opener |
| file | determine file type |
| cp, mv, rm, xargs (from findutils on Linux)  | copy, move and remove files |
| mediainfo, exiftool | multimedia file details |
| atool, patool ([integration](#integrate-patool)) | create, list and extract archives |
| vidir (from moreutils) | batch rename, move, delete dir entries |
| vlock (Linux), bashlock (macOS), lock(1) (BSD) | terminal locker |
| $EDITOR (overridden by $VISUAL, if defined) | edit files (fallback vi) |
| $PAGER | page through files (fallback less) |
| $SHELL | spawn a shell, run script (fallback sh) |

To edit all text files in EDITOR (preferably CLI, fallback vi):

    export NNN_USE_EDITOR=1

Arguments to the `$PAGER` and `$SHELL` should be combined together.

The option `open with` takes 1 combined argument.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

#### QUICKSTART

1. Install the [utilities required](#file-handling) for your regular activities.
2. Configure [cd on quit](#cd-on-quit).
3. Optionally open all text files in EDITOR (fallback vi):

       export NNN_USE_EDITOR=1
4. Run `n`.
5. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.
6. For additional functionality [setup custom scripts](#run-custom-scripts).

#### HOW TO

#### add bookmarks

Set environment variable `NNN_BMS` as a string of `key_char:location` pairs (max 10) separated by semicolons (`;`):

    export NNN_BMS='d:~/Documents;u:/home/user/Cam Uploads;D:~/Downloads/'

NOTE: Bookmark keys should be single-character to use them in combination with the Leader key.

#### copy file paths

##### to clipboard

Along with default copy, `nnn` can pipe the absolute path of the current file or multiple files to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on macOS. Here's a sample [copier script](https://github.com/jarun/nnn/blob/master/scripts/user-scripts/copier.sh).

To inform `nnn` of the executable copier script location:

    export NNN_COPIER="/path/to/copier.sh"

##### get selection manually

NOTE: In the following examples we assume the copy file is at `/home/user/.nnncp`.

The file paths are `NUL`-terminated, so additional processing is required to make them usable.

To get a space-separated list of the file paths in selection:

    cat /home/user/.nnncp | xargs -0 echo

To get a newline-separated list of the file paths in selection:

    # bash/zsh
    ls -ltr `cat /home/user/.nnncp | tr '\0' '\n'`
    ls -ltr $(cat /home/user/.nnncp | tr '\0' '\n')

    # fish
    ls -ltr (cat /home/user/.nnncp | tr '\0' '\n')

An alias may be handy, e.g. when you want to copy selection at the _run a command_ prompt:

    alias ncp="cat /home/user/.nnncp | tr '\0' '\n'"

so you can easily handle files together:

    # bash/zsh
    ls -ltr `ncp`
    ls -ltr $(ncp)

    # fish
    ls -ltr (ncp)

To get the list in a file:

    ncp > out.txt

#### cd on quit

To quit `nnn` and switch to the directory last opened follow the instructions below.

Pick the appropriate file for your shell from [`scripts/quitcd`](scripts/quitcd) and add the contents to your shell's rc file. You'll need to spawn a new shell for the change to take effect. You should start `nnn` as `n` (or modify the function name to something else). To change directory on quit press `^G` while exiting.

As you might notice, `nnn` uses the environment variable `NNN_TMPFILE` to write the last visited directory path. You can change it.

#### (neo)vim plugin

`nnn` can be used as a file picker/chooser within vim or neovim. Find the plugin [here](https://github.com/mcchrish/nnn.vim).

#### file picker

To use `nnn` as a file picker and redirect the output to other programs, use [picker.sh](https://github.com/jarun/nnn/blob/master/scripts/user-scripts/picker.sh).

Ways to pick files:

- use selection and press <kbd>q</kbd> to quit
- press <kbd>Enter</kbd> to pick any earlier selection and the current entry (to avoid selecting the last file in the selection twice, press <kbd>q</kbd> instead).

Notes:

- pressing <kbd>Enter</kbd> on a directory enters it; when trying to pick a directory, e.g. to pipe to `cd`, use single file selection and press <kbd>q</kbd>
- to discard selection and quit, press <kbd>^G</kbd>

#### run custom scripts

`nnn` can invoke custom scripts with the currently selected file name as argument 1.

Export the absolute path to the directory with your scripts or a single script:

    export NNN_SCRIPT=/home/user/scripts
    OR
    export NNN_SCRIPT=/usr/local/bin/nscript.sh

Press <kbd>R</kbd> to run the script in the current directory. You can also use this key to cancel choosing a script from the script directory.

##### sample scripts

Sample scripts for use cases like sxiv or fzy integration are under [user-scripts](https://github.com/jarun/nnn/tree/master/scripts/user-scripts). Feel free to contribute yours!

#### launch applications

Applications can be launched from the _run a command_ prompt. Use `&` to launch GUI applications in the background.

#### change dir color

The default color for directories is blue. Option `-c` accepts color codes from 0 to 7 to use a different color:

    0-black, 1-red, 2-green, 3-yellow, 4-blue, 5-magenta, 6-cyan, 7-white

Any other value disables colored directories.

#### integrate patool

On systems where `atool` is not available but `patool` is, drop two copies of the Python3 script [natool](https://github.com/jarun/nnn/tree/master/scripts/natool) as `atool` and `apack` somewhere in `$PATH`.

#### remote transfers

lftp can be used with `nnn` for automated remote transfers or copying a selection to a server. Visit the [wiki page](https://github.com/jarun/nnn/wiki/simplify-remote-transfers-with-lftp) for more details on the integration.

#### prompt shortcuts

The prompts (expect filter prompt) support some bash-like command-line shortcuts - <kbd>^A</kbd>, <kbd>^E</kbd>, <kbd>^U</kbd>.

Key <kbd>^L</kbd> clears and <kbd>Esc</kbd> exits _any_ prompt.

#### set idle timeout

The terminal locker is disabled by default. To set the wait time in seconds, use environment variable `NNN_IDLE_TIMEOUT`.

#### hot-plugged drives

Enable volume management in your DE file manager and set removable drives or media to be auto-mounted when inserted. Then visit the usual mount point location (`/mnt` or `/media/user`) in `nnn`.

#### tmux configuration

`nnn` might not handle keypresses correctly when used with tmux (see issue #104 for more details). Set `TERM=xterm-256color` to address it.

#### BSD terminal issue

By default in OpenBSD & FreeBSD, `stty` maps <kbd>^Y</kbd> to `DSUSP`. This means that typing <kbd>^Y</kbd> will suspend `nnn` as if you typed <kbd>^Z</kbd> (you can bring `nnn` back to the foreground by issuing `fg`) instead of entering multi-copy mode. You can check this with `stty -a`. If it includes the text `dsusp = ^Y`, issuing `stty dsusp undef` will disable this `DSUSP` and let `nnn` receive the <kbd>^Y</kbd> instead.

#### restrict file open

In order to disable opening files on accidental navigation key (<kbd>→</kbd> or <kbd>l</kbd>) press:

    export NNN_NO_FILE_OPEN_ON_NAV=1

Use <kbd>Enter</kbd> to open files.

#### restrict 0-byte files

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
- [LinuxLinks](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)

#### DEVELOPERS

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2019 [Arun Prakash Jana](https://github.com/jarun)
