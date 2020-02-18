<h1 align="center">nnn - <i>supercharge your productivity!</i></h1>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/tiny-repos/nnn.svg" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://en.wikipedia.org/wiki/Privacy-invasive_software"><img src="https://img.shields.io/badge/privacy-✓-crimson" alt="Privacy Awareness" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center"><a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/MPWpmos.png" /></a></p>
<p align="center"><i>navigate-as-you-type & du (click to see demo video)</i></p>

## Introduction

`nnn` is a full-featured terminal file manager. It's tiny and nearly 0-config with an [incredible performance](https://github.com/jarun/nnn/wiki/Performance).

`nnn` is also a du analyzer, an app launcher, a batch renamer and a file picker. The [plugin repository](https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins) has tons of plugins and documentation to extend the capabilities further. You can _plug_ new functionality _and play_ with a hotkey. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim).

It runs smoothly on the Pi, [Termux](https://www.youtube.com/watch?v=AbaauM7gUJw), Linux, macOS, BSD, Haiku, Cygwin, WSL, across DEs and GUI utilities or a strictly CLI environment.

[**Wiki**](https://github.com/jarun/nnn/wiki).

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/PayPal-donate-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

## Features

- Resource sensitive
  - Typically needs less than 3.5MB resident memory
  - Works with 8-bit colors
  - Disk-IO sensitive (few disk reads and writes)
  - No FPU usage (all integer maths, even for file size)
  - Minimizes screen refresh with fast line redraws
  - Tiny binary (typically less than 100KB)
- Portable
  - Statically-linked binary available
  - Language-agnostic plugins
  - Minimal library deps, easy to compile
  - Compile in/out features with make variables
  - No config file, minimal config with sensible defaults
  - Widely available on many packagers
  - Unicode support
- Quality
  - Privacy-aware (no unconfirmed user data collection)
  - POSIX-compliant, follows Linux kernel coding style
  - Highly optimized, static analysis integrated code
- Modes
  - Light (default), detail
  - Disk usage analyzer (block/apparent)
  - File picker, (neo)vim plugin
- Navigation
  - *Navigate-as-you-type* with dir auto-select
  - Contexts (_aka_ tabs/workspaces) with custom colors
  - Sessions, bookmarks with hotkeys; pin and visit a dir
  - Remote mounts (needs sshfs, rclone)
  - Familiar shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>), quick reference
  - CD on quit (*easy* shell integration)
  - Auto-proceed on opening files
- Search
  - Instant filtering with *search-as-you-type*
  - Regex (POSIX/PCRE) and string (default) filters
  - Subtree search plugin to open or edit files
- Sort
  - Ordered pure numeric names by default (visit _/proc_)
  - Case-insensitive version (_aka_ natural) sort
  - By file name, modification/access time, size, extension
  - Reverse sort
- Mimes
  - Open with desktop opener or specify a custom app
  - Create, list, extract, mount (FUSE based) archives
  - Option to open all text files in EDITOR
- Information
  - Detailed file information
  - Media information plugin
- Convenience
  - Run plugins and custom commands with hotkeys
  - FreeDesktop compliant trash (needs trash-cli)
  - Cross-dir file/all/range selection
  - Batch renamer (feature-limited) for selection or dir
  - Display a list of files from stdin
  - Copy (as), move (as), delete, archive, link selection
  - Dir updates, notification on cp, mv, rm completion
  - Copy file paths to system clipboard on select
  - Create (with parents), rename, duplicate (anywhere) files and dirs
  - Launch GUI apps, run commands, spawn a shell, toggle executable
  - Hovered file set as `$nnn` at prompt and spawned shell
  - Lock terminal after configurable idle timeout

## Quickstart

1. Install the [utilities you may need](https://github.com/jarun/nnn#utility-dependencies) based on your regular workflows.
2. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
3. To open text files in `$VISUAL` (else `$EDITOR`, fallback vi) add program option `-e` in your alias.
4. For additional functionality [install plugins](https://github.com/jarun/nnn/tree/master/plugins#installing-plugins).
5. To copy selected file paths to system clipboard and show notis on cp, mv, rm completion use option `-x`.
6. For a strictly CLI environment, customize and use plugin [`nuke`](https://github.com/jarun/nnn/blob/master/plugins/nuke).

Don't memorize! Arrows (or <kbd>h</kbd> <kbd>j</kbd> <kbd>k</kbd> <kbd>l</kbd>), <kbd>/</kbd>, <kbd>q</kbd> suffice. <kbd>Tab</kbd> creates, cycles contexts. <kbd>?</kbd> lists shortcuts.

## Installation

No permission to install packages? Get the statically linked binary from the latest release.

#### Library dependencies

A curses library with wide char support (e.g. ncursesw), libreadline (optional) and standard libc.

#### Utility dependencies

| Dependency | Installation | Operation |
| --- | --- | --- |
| xdg-open (Linux), open(1) (macOS), cygstart<br>(Cygwin), open (Haiku) | base | desktop opener |
| file, coreutils (cp, mv, rm), xargs | base | file type, copy, move and remove |
| tar, (un)zip [atool/bsdtar for more formats] | base | create, list, extract bzip2, (g)zip, tar |
| archivemount, fusermount(3) | optional | mount, unmount archives |
| sshfs, [rclone](https://rclone.org/), fusermount(3) | optional | mount, unmount remotes |
| trash-cli | optional | trash files (default action: rm) |
| vlock (Linux), bashlock (macOS), lock(1) (BSD),<br>peaclock (Haiku) | optional | terminal locker (fallback: [cmatrix](https://github.com/abishekvashok/cmatrix)) |
| advcpmv (Linux) ([integration](https://github.com/jarun/nnn/wiki/Advanced-use-cases#show-cp-mv-progress)) | optional | copy, move progress |
| `$VISUAL` (else `$EDITOR`), `$PAGER`, `$SHELL` | optional | fallback vi, less, sh |

#### From a package manager

Install `nnn` from your package manager. If the version available is dated try an alternative installation method.

<details><summary>Packaging status (expand)</summary>
<p>
<br>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/vertical-allrepos/nnn.svg" alt="Packaging status"></a>
</p>
Unlisted packagers:
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
    $ sudo make strip install

`PREFIX` is supported, in case you want to install to a different location.

See the [developer guides](https://github.com/jarun/nnn/wiki/Developer-guides) for source verification, compilation notes on the Pi, Cygwin and other tips.

#### Shell completion

Completion scripts for Bash, Fish and Zsh are [available](misc/auto-completion). Refer to your shell's manual for installation instructions.

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

- [Arun Prakash Jana](https://github.com/jarun) (Copyright © 2016-2020)
- [0xACE](https://github.com/0xACE)
- [Anna Arad](https://github.com/annagrram)
- [KlzXS](https://github.com/KlzXS)
- [Maxim Baz](https://github.com/maximbaz)
- and other contributors

`nnn` is actively developed. Visit the to the [ToDo list](https://github.com/jarun/nnn/issues/472) to contribute or see the features in progress.
