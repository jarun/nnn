<h3 align="center"><img src="misc/logo/logo-128x128.png" alt="nnn"><br>nnn - <i>Supercharge your productivity!</i></h3>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600&label=rel" alt="Latest release" /></a>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/tiny-repos/nnn.svg?header=repos" alt="Availability"></a>
<a href="https://travis-ci.org/jarun/nnn"><img src="https://img.shields.io/travis/jarun/nnn/master.svg?label=travis" alt="Travis Status" /></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circleci" alt="CircleCI Status" /></a>
<a href="https://en.wikipedia.org/wiki/Privacy-invasive_software"><img src="https://img.shields.io/badge/privacy-✓-crimson?maxAge=2592000" alt="Privacy Awareness" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/©-BSD%202--Clause-important.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center"><a href="https://asciinema.org/a/353811"><img src="https://i.imgur.com/InHB5DB.png" /><br></a></p>
<p align="center"><i>icons and colors (click for asciicast) </i></p>

<h3 align="center">[<a
href="https://github.com/jarun/nnn#features">Features</a>] [<a
href="https://github.com/jarun/nnn#quickstart">Quickstart</a>] [<a
href="https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins">Plugins</a>] [<a
href="https://github.com/jarun/nnn/wiki/Performance">Performance</a>]</h3>

`nnn` (_n³_) is a full-featured terminal file manager. It's tiny and nearly 0-config with an incredible performance.

`nnn` can analyze disk usage, batch rename, launch apps and pick files. The plugin repository has tons of plugins and documentation to extend the capabilities further e.g. [preview](https://github.com/jarun/nnn/wiki/Live-previews), (un)mount disks, find & list, file/dir diff, upload files. There's an independent [(neo)vim plugin](https://github.com/mcchrish/nnn.vim).

It runs smoothly on the Pi, [Termux](https://www.youtube.com/watch?v=AbaauM7gUJw) (Android), Linux, macOS, BSD, Haiku, Cygwin, WSL, across DEs and GUI utilities or a strictly CLI environment.

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/donate-PayPal-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

<br>

<p align="center"><a href="https://www.youtube.com/watch?v=U2n5aGqou9E"><img src="https://i.imgur.com/MPWpmos.png" /></a></p>
<p align="center"><i>type-to-nav & du (click for demo video)</i></p>

## Black magic! :dark_sunglasses:

- Instantly load, sort, filter thousands of files
- Type to navigate with automatic dir selection
- find/fd/grep/ripgrep/fzf from `nnn` and list in `nnn`
- Never lose context - start where you quit
- Mount any cloud storage service in a few keypresses
- Select files from anywhere (not just a single dir)
- Unlimited bookmarks, plugins, cmds with custom hotkeys
- Write a plugin in any language you know
- Edit and preview markdown, man page, html
- Open a file and auto-advance to the next
- Filter filtered entries
- Export (filtered) list of visible files
- Configure the middle mouse click to do anything
- Fuzzy search subtree and open a file (or its parent dir)
- Load four dirs with custom settings at once
- Notifications on cp, mv, rm completion
- Auto-sync selection to system clipboard
- Access selection from another instance of `nnn`
- Open text files detached in another pane/tab/window
- Mount and modify archives
- Create files/dirs/duplicates with parents (like `mkdir -p`)
- Toggle hidden with <kbd>.</kbd>, visit HOME with <kbd>~</kbd>, last dir with <kbd>-</kbd>
- Mark a frequently visited dir at runtime
- Sort by modification, access and inode change time
- Compile out/in features with make variables
- Watch matrix text fly or read fortune messages
- Configure in 5 minutes!

## Features

- Frugal
  - Typically needs less than 3.5MB resident memory
  - Works with 8 colors (and xterm 256 colors)
  - Disk-IO sensitive (few disk reads and writes)
  - No FPU usage (all integer maths, even for file size)
  - Minimizes screen refresh with fast line redraws
  - Tiny binary (typically around 100KB)
  - 1-column mode for smaller terminals and form factors
  - Hackable - compile in/out features and dependencies
- Portable
  - Language-agnostic plugins
  - Static binary available (no need to install)
  - Minimal library deps, easy to compile
  - No config file, minimal config with sensible defaults
  - Widely available on many packagers
  - Touch enabled, comfortable on handhelds too!
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
  - Sessions, bookmarks with hotkeys; mark and visit a dir
  - Remote mounts (needs sshfs, rclone)
  - Familiar shortcuts (arrows, <kbd>~</kbd>, <kbd>-</kbd>, <kbd>@</kbd>), quick look-up
  - CD on quit (*easy* shell integration)
  - Auto-advance on opening files
- Search
  - Instant filtering with *search-as-you-type*
  - Regex (POSIX/PCRE) and string (default) filters
  - Subtree search plugin to open or edit files
- Sort
  - Ordered pure numeric names by default (visit _/proc_)
  - Case-insensitive version (_aka_ natural) sort
  - By name, access/change/mod (default) time, size, extn
  - Reverse sort
- Mimes
  - Preview hovered files in FIFO-based previewer
  - Open with desktop opener or specify a custom opener
  - File-specific colors (or minimal _dirs in context color_)
  - Icons (customize and compile-in)
  - Plugins for image and video thumbnails
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
  - Display file list from stdin or plugin
  - Copy (as), move (as), delete, archive, link selection
  - Dir updates, notification on cp, mv, rm completion
  - Copy file paths to system clipboard on select
  - Create (with parents), rename, duplicate files and dirs
  - Launch apps, run commands, spawn a shell, toggle exe
  - Hovered file set as `$nnn` at prompt and spawned shell
  - Lock terminal after configurable idle timeout
  - Basic support for screen readers and braille displays

## Quickstart

1. [Install](https://github.com/jarun/nnn/wiki/Usage#installation) `nnn` and deps (if you need any). All files are opened with the desktop opener by default.
2. Add option `-e` to your alias to open text files in `$VISUAL`/`$EDITOR`/ vi. [Open detached](https://github.com/jarun/nnn/wiki/Basic-use-cases#detached-text) if you wish.
3. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
4. [Install plugins](https://github.com/jarun/nnn/tree/master/plugins#installation).
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

Visit the [ToDo list](https://github.com/jarun/nnn/issues/629) to contribute or see the features in progress.

## Elsewhere

- [ArchWiki](https://wiki.archlinux.org/index.php/Nnn)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [gHacks Tech News](https://www.ghacks.net/2019/11/01/nnn-is-an-excellent-command-line-based-file-manager-for-linux-macos-and-bsds/)
- Hacker News [[1](https://news.ycombinator.com/item?id=18520898)] [[2](https://news.ycombinator.com/item?id=19850656)]
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- LinuxLinks [[1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)] [[2](https://www.linuxlinks.com/bestconsolefilemanagers/)] [[3](https://www.linuxlinks.com/excellent-system-tools-nnn-portable-terminal-file-manager/)]
- [Open Source For You](https://www.opensourceforu.com/2019/12/nnn-this-feature-rich-terminal-file-manager-will-enhance-your-productivity/)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine - Issue 135](https://fullcirclemagazine.org/issue-135/)
