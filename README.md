<h1 align="center">nnn - <i>supercharge your productivity!</i></h1>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600" alt="Latest release" /></a>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/tiny-repos/nnn.svg?header=in repos" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://en.wikipedia.org/wiki/Privacy-invasive_software"><img src="https://img.shields.io/badge/privacy-✓-crimson?maxAge=2592000" alt="Privacy Awareness" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/license-BSD%202--Clause-yellow.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center"><a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/MPWpmos.png" /></a></p>
<p align="center"><i>type-to-nav & du (click to see demo video)</i></p>

## Introduction

<img align="left" src="misc/logo/logo-128x128.png">

`nnn` (or `n³`) is a full-featured terminal file manager. It's tiny and nearly 0-config with an [incredible performance](https://github.com/jarun/nnn/wiki/Performance).

`nnn` can analyze disk usage, batch rename, launch apps and pick files. The [plugin repository](https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins) has tons of plugins and documentation to extend the capabilities further e.g. preview hovered, (un)mount disks, find & list, file/dir diff, upload files. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim).

It runs smoothly on the Pi, [Termux](https://www.youtube.com/watch?v=AbaauM7gUJw) (Android), Linux, macOS, BSD, Haiku, Cygwin, WSL, across DEs and GUI utilities or a strictly CLI environment.

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/donate-PayPal-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

## Black magic! :dark_sunglasses:

- Load, sort, filter thousands of files instantly
- Type to navigate with automatic dir selection
- Select files from anywhere (not just a single dir)
- Never lose context - start where you quit `nnn`
- Edit and preview markdown, man page, html
- Open a file and auto-advance to the next
- Export (filtered) list of visible files
- find/fd/grep/ripgrep/fzf from `nnn` and list in `nnn`
- Unlimited bookmarks, plugins, cmds with custom hotkeys
- Write a plugin in any language you know
- Configure the middle mouse click to do anything
- Fuzzy search subtree and open a file (or its parent dir)
- Load four dirs with custom settings at once
- Notifications on cp, mv, rm completion
- Auto-sync selection to system clipboard
- Open text files detached in another pane/tab/window
- Create files/dirs/duplicates with parents (like `mkdir -p`)
- Toggle hidden with <kbd>.</kbd>, visit HOME with <kbd>~</kbd>, last dir with <kbd>-</kbd>
- Pin a frequently visited dir at runtime
- Mount any cloud storage service in a few keypresses
- Mount and modify archives
- Filter filtered entries
- Sort files by access time and inode change time
- Access selection from another instance of `nnn`
- Compile out features you don't need
- Watch matrix text fly or read fortune messages
- Configure in 5 minutes!

## Features

- Frugal
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
  - *Type-to-nav* mode with dir auto-select
  - Contexts (_aka_ tabs/workspaces) with custom colors
  - Sessions, bookmarks with hotkeys; pin and visit a dir
  - Remote mounts (needs sshfs, rclone)
  - Familiar shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>), quick reference
  - CD on quit (*easy* shell integration)
  - Auto-advance on opening files
- Search
  - Instant filtering with *search-as-you-type*
  - Regex (POSIX/PCRE) and string (default) filters
  - Subtree search plugin to open or edit files
- Sort
  - Ordered pure numeric names by default (visit _/proc_)
  - Case-insensitive version (_aka_ natural) sort
  - By file name, access/change/mod (default) time, size, extension
  - Reverse sort
- Mimes
  - Open with desktop opener or specify a custom opener
  - Preview hovered files in FIFO-based previewer
  - Create, list, extract, mount (FUSE based) archives
  - Option to open all text files in EDITOR
- Information
  - Detailed file information
  - Media information plugin
- Convenience
  - Run plugins and custom commands with hotkeys
  - FreeDesktop compliant trash (needs trash-cli)
  - Cross-dir file/all/range selection
  - Batch renamer for selection or dir
  - Display a list of files from stdin
  - Copy (as), move (as), delete, archive, link selection
  - Dir updates, notification on cp, mv, rm completion
  - Copy file paths to system clipboard on select
  - Create (with parents), rename, duplicate (anywhere) files and dirs
  - Launch GUI apps, run commands, spawn a shell, toggle executable
  - Hovered file set as `$nnn` at prompt and spawned shell
  - Lock terminal after configurable idle timeout

## Quickstart

1. [Install](https://github.com/jarun/nnn/wiki/Usage#installation) `nnn` and deps (if you need any).
2. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
3. Use option `-e` in your alias to open text files in `$VISUAL`/`$EDITOR`/ vi. [Open detached](https://github.com/jarun/nnn/wiki/Basic-use-cases#detached-text) if you wish.
4. [Install plugins](https://github.com/jarun/nnn/tree/master/plugins#installing-plugins).
5. Use option `-x` to copy selected file paths to system clipboard and show notis on cp, mv, rm completion.
6. For a CLI-only environment, customize and use plugin [`nuke`](https://github.com/jarun/nnn/blob/master/plugins/nuke) with option `-c` (overrides `-e`).

Don't memorize! Arrows (or <kbd>h</kbd> <kbd>j</kbd> <kbd>k</kbd> <kbd>l</kbd>), <kbd>/</kbd>, <kbd>q</kbd> suffice. <kbd>Tab</kbd> creates, cycles contexts. <kbd>?</kbd> lists shortcuts.

[![Wiki](https://img.shields.io/badge/RTFM-nnn%20Wiki-important?maxAge=2592000)](https://github.com/jarun/nnn/wiki)

## Developers

- [Arun Prakash Jana](https://github.com/jarun) (Copyright © 2016-2020)
- [0xACE](https://github.com/0xACE)
- [Anna Arad](https://github.com/annagrram)
- [KlzXS](https://github.com/KlzXS)
- [Léo Villeveygoux](https://github.com/leovilok)
- [Maxim Baz](https://github.com/maximbaz)
- [Todd Yamakawa](https://github.com/toddyamakawa)
- and other contributors

Visit the [ToDo list](https://github.com/jarun/nnn/issues/594) to contribute or see the features in progress.

## Elsewhere

- [Wikipedia](https://en.wikipedia.org/wiki/Nnn_(file_manager))
- [ArchWiki](https://wiki.archlinux.org/index.php/Nnn)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [gHacks Tech News](https://www.ghacks.net/2019/11/01/nnn-is-an-excellent-command-line-based-file-manager-for-linux-macos-and-bsds/)
- Hacker News [[1](https://news.ycombinator.com/item?id=18520898)] [[2](https://news.ycombinator.com/item?id=19850656)]
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- LinuxLinks [[1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)] [[2](https://www.linuxlinks.com/bestconsolefilemanagers/)] [[3](https://www.linuxlinks.com/excellent-system-tools-nnn-portable-terminal-file-manager/)]
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)
