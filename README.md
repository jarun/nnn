## nnn

Noice is Not Noice, a noicer fork...

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://aur.archlinux.org/packages/nnn"><img src="https://img.shields.io/aur/version/nnn.svg?maxAge=600" alt="AUR" /></a>
<a href="http://formulae.brew.sh/formula/nnn"><img src="https://img.shields.io/homebrew/v/nnn.svg?maxAge=600" alt="Homebrew" /></a>
<a href="https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/debian-10+-blue.svg?maxAge=2592000" alt="Debian Buster+" /></a>
<a href="https://apps.fedoraproject.org/packages/nnn"><img src="https://img.shields.io/badge/fedora-27+-blue.svg?maxAge=2592000" alt="Fedora 27+" /></a>
<a href="https://software.opensuse.org/package/nnn"><img src="https://img.shields.io/badge/opensuse%20leap-15.0+-blue.svg?maxAge=2592000" alt="openSUSE Leap 15.0+" /></a>
<a href="https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1"><img src="https://img.shields.io/badge/ubuntu-17.10+-blue.svg?maxAge=2592000" alt="Ubuntu Artful+" /></a>
</p>

<p align="center">
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://travis-ci.org/jarun/nnn.svg?branch=master" alt="Build Status" /></a>
</p>

[![nnn screencast](https://i.imgur.com/wNtI24c.jpg)](https://vimeo.com/233223942 "Click to see nnn in action!")

<p align="center"><i>nnn in action! (click to play video)</i></a></p>

`nnn` is probably the [fastest and most resource-sensitive](#performance) file manager you have ever used. It integrates seamlessly with your DE and favourite GUI utilities, has a unique [navigate-as-you-type](#navigate-as-you-type-mode) mode with auto-select, disk usage analyzer mode, bookmarks, familiar navigation shortcuts, subshell spawning and much more.

[Integrate utilities](https://github.com/jarun/nnn#sample-scripts) like sxiv or fzy easily; `nnn` supports as many scripts as you need!

[Quickstart](#quickstart) and see how `nnn` simplifies those long desktop sessions...

Have fun with it! Missing a feature? Want to contribute? Head to the rolling [ToDo list](https://github.com/jarun/nnn/issues/110).

*Love smart and efficient utilities? Explore [my repositories](https://github.com/jarun?tab=repositories). Buy me a cup of coffee if they help you.*

<p align="center">
<a href="https://saythanks.io/to/jarun"><img src="https://img.shields.io/badge/say-thanks!-ff69b4.svg" /></a>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-green.svg" alt="Donate via PayPal!" /></a>
</p>

### Table of Contents

- [Features](#features)
- [Performance](#performance)
- [Installation](#installation)
  - [Dependencies](#dependencies)
  - [From a package manager](#from-a-package-manager)
  - [Release packages](#release-packages)
  - [From source](#from-source)
- [Shell completion](#shell-completion)
- [Usage](#usage)
  - [Cmdline options](#cmdline-options)
  - [Keyboard shortcuts](#keyboard-shortcuts)
  - [Filters](#filters)
  - [Navigate-as-you-type mode](#navigate-as-you-type-mode)
  - [File indicators](#file-indicators)
  - [File handling](#file-handling)
  - [Help](#help)
- [Quickstart](#quickstart)
- [How to](#how-to)
  - [add bookmarks](#add-bookmarks)
  - [copy file paths](#copy-file-paths)
    - [to clipboard](#to-clipboard)
    - [when X is missing](#when-x-is-missing)
  - [copy, move, delete files](#copy-move-delete-files)
  - [cd on quit](#cd-on-quit)
  - [run custom scripts](#run-custom-scripts)
    - [sample scripts](#sample-scripts)
  - [dual-pane or multi-pane](#dual-pane-or-multi-pane)
  - [change dir color](#change-dir-color)
  - [use cd .....](#use-cd-)
  - [integrate patool](#integrate-patool)
  - [work faster at rename prompt](#work-faster-at-rename-prompt)
  - [set idle timeout](#set-idle-timeout)
  - [show hot plugged drives](#show-hot-plugged-drives)
  - [tmux config](#tmux-config)
- [Why fork?](#why-fork)
- [Mentions](#mentions)
- [Developers](#developers)

### Features

- Modes - basic, detail (default), disk usage analyzer (du)
- Navigation
  - Familiar, easy shortcuts (arrows, `~`, `-`, `&`)
  - *Navigate-as-you-type* mode with dir auto-select for the maverick
  - Handy bookmarks, start at bookmark, pin and visit directory
  - chdir prompt with tab completion (interprets cd ..... too!)
  - Roll-over at edges, page through entries
  - Show directories in custom color (default: enabled in blue)
- Sorting
  - Directories always listed on top
  - Sort by file name, modification time, size
  - Numeric order (1, 2, ... 10, 11, ...) for numeric names
- Search
  - Superfast directory content filtering with *search-as-you-type*
  - Desktop search (gnome-search-tool, catfish) integration
- Mimes
  - Open with desktop opener (default) or specify a custom app
  - Create, list and extract archives (needs atool/patool)
  - Optionally open text files in EDITOR (fallback vi)
  - Customizable script to handle actions (sparsely used)
- Information
  - Detailed stat-like file information
  - Media information (needs mediainfo or exiftool, if specified)
- Convenience
  - Create, rename files and directories
  - Batch rename/move/delete current directory entries in vidir (from moreutils)
  - Spawn SHELL (fallback sh) in the current directory
  - Run custom scripts in the current directory
  - Copy absolute file paths with/without X (*easy* shell integration)
  - Change directory at exit (*easy* shell integration)
  - Open any file in EDITOR (fallback vi) or PAGER (fallback less)
  - Open current directory in a custom GUI file manager
  - Terminal screensaver/locker integration
- Unicode support
- Highly optimized code, minimal resource usage

### Performance

`nnn` vs. ncdu memory usage in disk usage analyzer mode (400K files on disk):

<pre>
  PID USER      PR  NI    VIRT    <b>RES</b>    SHR S  %CPU %MEM     TIME+ COMMAND
 5034 vaio      20   0   71628  <b>59932</b>   2412 S   0.0  0.7   0:01.22 ncdu /
 4949 vaio      20   0   14812   <b>3616</b>   2560 S   0.0  0.0   0:00.83 nnn -S /
</pre>

`nnn` vs. midnight commander vs. ranger memory usage while viewing a directory with 13.5K files, sorted by size:

<pre>
  PID USER      PR  NI    VIRT    <b>RES</b>    SHR S  %CPU %MEM     TIME+ COMMAND
31885 vaio      20   0  139720  <b>91220</b>   8460 S   0.0  1.1   0:02.96 /usr/bin/python -O /usr/bin/ranger
30108 vaio      20   0   72152  <b>12468</b>   7336 S   0.0  0.2   0:00.06 mc
30168 vaio      20   0   16476   <b>5072</b>   2640 S   0.0  0.1   0:00.22 nnn -c 1 -i
</pre>

Intrigued? Find out [HOW](https://github.com/jarun/nnn/wiki/performance-factors).

### Installation

#### Dependencies

`nnn` needs libncursesw (on Linux or ncurses on OS X) and standard libc.

#### From a package manager

- [AUR](https://aur.archlinux.org/packages/nnn/) (`yaourt -S nnn`)
- [Debian](https://packages.debian.org/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Fedora](https://apps.fedoraproject.org/packages/nnn) (`dnf install nnn`)
- [FreeBSD](https://www.freshports.org/misc/nnn) (`pkg install nnn`)
- [Gentoo](https://packages.gentoo.org/packages/app-misc/nnn) (`emerge nnn`)
- [Homebrew](http://formulae.brew.sh/formula/nnn) (`brew install nnn`)
- [NixOS](https://github.com/NixOS/nixpkgs/tree/master/pkgs/applications/misc/nnn) (`nix-env -i nnn`)
- [OpenBSD](https://cvsweb.openbsd.org/cgi-bin/cvsweb/ports/sysutils/nnn/) (`pkg_add nnn`)
- [openSUSE](https://software.opensuse.org/package/nnn) (and packages for several other distros) (`zypper in nnn`)
- [Raspbian Testing](https://archive.raspbian.org/raspbian/pool/main/n/nnn/) (`apt-get install nnn`)
- [Slackware](http://slackbuilds.org/repository/14.2/system/nnn/) (`slackpkg install nnn`)
- [Source Mage](http://codex.sourcemage.org/test/shell-term-fm/nnn/) (`cast nnn`)
- [Ubuntu](https://packages.ubuntu.com/search?keywords=nnn&searchon=names&exact=1) (`apt-get install nnn`)
- [Ubuntu PPA](https://launchpad.net/~twodopeshaggy/+archive/ubuntu/jarun/) (`apt-get install nnn`)
- [Void Linux](https://github.com/void-linux/void-packages/tree/master/srcpkgs/nnn) (`xbps-install -S nnn`)

#### Release packages

Packages for Arch Linux, CentOS, Debian, Fedora and Ubuntu are available with the [latest stable release](https://github.com/jarun/nnn/releases/latest).

#### From source

To cook yourself, download the [latest stable release](https://github.com/jarun/nnn/releases/latest) or clone this repository (*risky*). Then install the dependencies and compile (e.g. on Ubuntu 16.04):

    $ sudo apt-get install pkg-config libncursesw5-dev
    $ make
    $ sudo make install

`PREFIX` is supported, in case you want to install to a different location.

### Shell completion

Search keyword and option completion scripts for Bash, Fish and Zsh can be found in respective subdirectories of [`scripts/auto-completion/`](scripts/auto-completion). Please refer to your shell's manual for installation instructions.

### Usage

#### Cmdline options

```
usage: nnn [-b key] [-c N] [-e] [-i] [-l]
           [-p nlay] [-S] [-v] [-h] [PATH]

The missing terminal file manager for X.

positional args:
  PATH   start dir [default: current dir]

optional args:
 -b key  bookmark key to open
 -c N    dir color, disables if N>7
 -e      use exiftool instead of mediainfo
 -i      start in navigate-as-you-type mode
 -l      start in light mode
 -p nlay path to custom nlay
 -S      start in disk usage analyser mode
 -v      show program version
 -h      show this help
```

`>` indicates the currently selected entry in `nnn`.

#### Keyboard shortcuts

```
            Key  Function
              ----
       ↑, k, ^P  Up
       ↓, j, ^N  Down
       PgUp, ^U  Scroll up
       PgDn, ^D  Scroll down
 Home, g, ^, ^A  First entry
  End, G, $, ^E  Last entry
    →, ↵, l, ^M  Open file/enter dir
 ←, Bksp, h, ^H  Parent dir
             ^O  Open with...
     Insert, ^I  Toggle nav-as-you-type
              ~  Go HOME
              &  Start dir
              -  Last visited dir
              /  Filter entries
              .  Toggle show hidden
             ^B  Bookmark prompt
              b  Pin current dir
             ^V  Go to pinned dir
              c  cd prompt
              d  Toggle detail view
              D  File details
           m, M  Brief/full media info
              n  Create new
             ^R  Rename entry
              r  Open dir in vidir
              s  Toggle sort by size
              S  Toggle apparent size
             ^J  Toggle du mode
              t  Toggle sort by mtime
          !, ^]  Spawn SHELL in dir
              R  Run custom script
              e  Edit in EDITOR
              p  Open in PAGER
              f  Archive entry
              F  List archive
             ^F  Extract archive
      Space, ^K  Copy file path
             ^Y  Toggle multi-copy
              y  Show copy buffer
             ^T  Toggle path quote
             ^L  Redraw, clear prompt
            Esc  Exit prompt
              L  Lock terminal
              o  Open DE filemanager
             ^/  Open DE search app
              ?  Help, settings
          Q, ^G  Quit and cd
          q, ^X  Quit
```

Help & settings, file details, media info and archive listing are shown in the PAGER. Please use the PAGER-specific keys in these screens.

#### Filters

Filters support regexes to instantly (search-as-you-type) list the matching entries in the current directory.

Ways to exit filter prompt:
- press <kbd>^L</kbd> to clear filter followed by <kbd>Bksp</kbd> (to clear the filter symbol, like vi)
  - at other prompts <kbd>^L</kbd> followed by <kbd>Enter</kbd> discards all changes and exits prompt
- run a search with no matches and press <kbd>Enter</kbd>

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

#### File handling

| External dependency | Operation |
| --- | --- |
| xdg-open (Linux), open(1) (OS X), cygstart (Cygwin) | desktop opener |
| mediainfo, exiftool | multimedia file details |
| gnome-search-tool, catfish | desktop search utility |
| atool, patool ([integration](#integrate-patool)) | create, list and extract archives |
| vidir from moreutils | batch rename, move, delete dir entries |
| vlock (Linux), bashlock (OS X), lock(1) (BSD) | terminal locker |
| $EDITOR (overridden by $VISUAL, if defined) | edit files (fallback vi) |
| $PAGER | page through files (fallback less) |
| $SHELL | spawn a shell, run script (fallback sh) |

- To edit all text files in EDITOR (preferably CLI, fallback vi):

      export NNN_USE_EDITOR=1
- To enable the desktop file manager key, set `NNN_DE_FILE_MANAGER`. E.g.:

      export NNN_DE_FILE_MANAGER=thunar
      export NNN_DE_FILE_MANAGER=nautilus

Customizable script [nlay](https://github.com/jarun/nnn/wiki/all-about-nlay) is used to run desktop search utility and terminal locker.

#### Help

    $ nnn -h
    $ man nnn
To lookup keyboard shortcuts at runtime, press <kbd>?</kbd>.

### Quickstart

1. Install the [utilities required](#file-handling) for your regular activities.
2. Configure file path copy [using X clipboard](#to-clipboard) or [without X](#when-x-is-missing).
3. Configure [cd on quit](#cd-on-quit).
4. Optionally open all text files in EDITOR (fallback vi):

       export NNN_USE_EDITOR=1
5. Run `n`.
6. Press <kbd>?</kbd> for help on keyboard shortcuts anytime.
7. For additional functionality [setup custom scripts](#run-custom-scripts).

### How to

#### add bookmarks

Set environment variable `NNN_BMS` as a string of `key:location` pairs (max 10) separated by semicolons (`;`):

    export NNN_BMS='doc:~/Documents;u:/home/user/Cam Uploads;D:~/Downloads/'

The bookmark prompt also understands the <kbd>~</kbd> (HOME), <kbd>-</kbd> (last visited directory) and <kbd>&</kbd> (start directory) shortcuts.

#### copy file paths

File paths can be copied to the clipboard or to a specific temporary file (if X is unavailable, for example). When in multi-copy mode, currently copied file paths can be listed by pressing `y`.

##### to clipboard

`nnn` can pipe the absolute path of the current file or multiple files to a copier script. For example, you can use `xsel` on Linux or `pbcopy` on OS X.

Sample Linux copier script:

    #!/bin/sh

    # comment the next line to convert newlines to spaces
    IFS=

    echo -n $1 | xsel --clipboard --input

export `NNN_COPIER`:

    export NNN_COPIER="/path/to/copier.sh"

Use <kbd>^K</kbd> to copy the absolute path (from `/`) of the file under the cursor to clipboard.

To copy multiple file paths, switch to the multi-copy mode using <kbd>^Y</kbd>. In this mode you can

- select multiple files one by one by pressing <kbd>^K</kbd> on each entry; or,
- navigate to another file in the same directory to select a range of files.

Pressing <kbd>^Y</kbd> again copies the paths to clipboard and exits the multi-copy mode.

To wrap each file path within single quotes, export `NNN_QUOTE_ON`:

    export NNN_QUOTE_ON=1
This is particularly useful if you are planning to copy the whole string to the shell to run a command. Quotes can be toggled at runtime using <kbd>^T</kbd>.

Note that the filename is not escaped. So copying may still fail for filenames having quote(s) in them.

##### when X is missing

A very common scenario on headless remote servers connected via SSH. As the clipboard is missing, `nnn` copies the path names to the tmp file `DIR/.nnncp`, where `DIR` (by priority) is:

    $HOME or,
    $TMPDIR or,
    /tmp

`nnn` needs to know X is unavailable:

    export NNN_NO_X=1

To see the path to the copy file, run `nnn`, press `?` and look up `NNN_NO_X`.

Note: despite the name of the environment variable, this method works even if X is available.

Use <kbd>^Y</kbd> and/or <kbd>^K</kbd> to copy file paths as usual. To use the copied paths from the cmdline, use command substitution. For example, if `DIR` above is `/home/user`:

    # bash/zsh
    ls -ltr `cat /home/user/.nnncp`
    ls -ltr $(cat /home/user/.nnncp)

    # fish
    ls -ltr (cat /home/user/.nnncp)

An alias may be handy:

    alias ncp='cat /home/user/.nnncp'

so you can easily copy, move or delete multiple files together:

    # bash/zsh
    ls -ltr `ncp`
    ls -ltr $(ncp)
    cp -rvf `ncp` .
    mv `ncp` .
    rm `ncp` -rf

    # fish
    ls -ltr (ncp)
    cp -rvf (ncp) .
    mv (ncp) .
    rm (ncp) -rf

Note that you may want to keep quotes disabled in this case.

#### copy, move, delete files

The `nnn` workflow to copy, move or delete files is:

1. Copy the absolute paths using <kbd>^Y</kbd> and/or <kbd>^K</kbd>
2. To copy or move files navigate to the destination directory. You can also fire a new instance of `nnn` in another tab of your terminal emulator and open the destination directory.
3. Spawn a subshell in the destination directory (<kbd>!</kbd>)
4. While typing the desired command, copy the file paths (usually <kbd>^-Shift-V</kbd>) from the clipboard. If X is unavailable, refer to [this section](#when-x-is-missing).

In addition, `nnn` integrates with vidir. vidir supports batch file move and delete.

#### cd on quit

To quit `nnn` and switch to the directory last opened follow the instructions below.

Pick the appropriate file for your shell from [`scripts/quitcd`](scripts/quitcd) and add the contents to your shell's rc file. You'll need to spawn a new shell for the change to take effect. You should start `nnn` as `n` (or modify the function name to something else). To change directory on quit press `Q` (it's _capital_) or `^G` while exiting.

As you might notice, `nnn` uses the environment variable `NNN_TMPFILE` to write the last visited directory path. You can change it.

#### run custom scripts

`nnn` can invoke custom scripts with the currently selected file name as argument 1.

Export the path to the custom executable script:

    export NNN_SCRIPT=/usr/local/bin/nscript

Press <kbd>R</kbd> to run the script in the current directory.

It's possible to run multiple scripts with `nnn` as long as the scripts are in the same location and share the same prefix. To enable multiple scripts,

    export NNN_MULTISCRIPT=1

With the example of `NNN_SCRIPT` above, some more scripts could be:

    /usr/local/bin/nscript1
    /usr/local/bin/nscript2
    /usr/local/bin/nscriptcustom1
    /usr/local/bin/nscriptcustom2
    and so on...

Type the correct suffix  when prompted on pressing the keybind <kbd>R</kbd>. To use the base script (`NNN_SCRIPT`), just press <kbd>Enter</kbd>.

##### sample scripts

- Open image files in current dir in **sxiv**:

      #!/usr/bin/env sh

      sxiv -q * >/dev/null 2>&1

- Fuzzy find files in **fzy** and open with xdg-open:

      #!/usr/bin/env sh

      xdg-open $(find -type f | fzy) >/dev/null 2>&1

#### dual-pane or multi-pane

`nnn` doesn't have a native dual-pane or multi-pane mode. Use it with tmux, GNU Screen, Terminator or Tilix.

#### change dir color

The default color for directories is blue. Option `-c` accepts color codes from 0 to 7 to use a different color:

    0-black, 1-red, 2-green, 3-yellow, 4-blue, 5-magenta, 6-cyan, 7-white

Any other value disables colored directories.

#### use cd .....

To jump to the n<sup>th</sup> level parent, use `n + 1` dots (the first `.` denotes PWD). For example, to jump to the 6<th> parent of the current directory, use 7 dots. If the number of dots would take you *beyond* `/` (which isn't possible), you'll be placed at `/`.

#### integrate patool

On systems where `atool` is not available but `patool` is, drop two copies of the Python3 script [natool](https://github.com/jarun/nnn/blob/master/scripts/natool) as `atool` and `apack` somewhere in `$PATH`.

#### work faster at rename prompt

The rename prompt supports some bash-like command-line shortcuts - <kbd>^A</kbd>, <kbd>^E</kbd>, <kbd>^U</kbd>. <kbd>^L</kbd> clears the name.

#### set idle timeout

The terminal screensaver is disabled by default. To set the wait time in seconds, use environment variable `NNN_IDLE_TIMEOUT`.

#### show hot plugged drives

Enable volume management in your DE file manager and set removable drives or media to be auto-mounted when inserted. Then visit the usual mount point location (`/mnt` or `/media/user`) in `nnn`.

#### tmux config

`nnn` might not handle keypresses correctly when used with tmux (see issue #104 for more details). Set `TERM=xterm-256color` to address it.

### Why fork?

`nnn` was initially forked from [noice](http://git.2f30.org/noice/) but is significantly [different](https://github.com/jarun/nnn/wiki/nnn-vs.-noice) today. I chose to fork because:
- one can argue my approach deviates from the goal of the original project -  keep the utility `suckless`. In my opinion evolution is the taste of time.
- I would like to have a bit of control on what features are added in the name of desktop integration. A feature-bloat is the last thing in my mind. Check out [nnn design considerations](https://github.com/jarun/nnn/wiki/nnn-design-considerations) for more details.

### Mentions

- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [LinuxLinks](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)

### Developers

1. Copyright © 2014-2016 Lazaros Koromilas
2. Copyright © 2014-2016 Dimitris Papastamos
3. Copyright © 2016-2018 [Arun Prakash Jana](https://github.com/jarun)
