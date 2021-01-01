<h3 align="center"><img src="misc/logo/logo-128x128.png" alt="nnn"><br>nnn - <i>Supercharge your productivity!</i></h3>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600&label=rel" alt="Latest release" /></a>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/tiny-repos/nnn.svg?header=repos" alt="Availability"></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=circle%20ci" alt="CircleCI Status" /></a>
<a href="https://github.com/jarun/nnn/actions"><img src="https://github.com/jarun/nnn/workflows/ci/badge.svg?branch=master" alt="GitHub CI Status" /></a>
<a href="https://en.wikipedia.org/wiki/Privacy-invasive_software"><img src="https://img.shields.io/badge/privacy-✓-crimson?maxAge=2592000" alt="Privacy Awareness" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/©-BSD%202--Clause-important.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center"><a href="https://asciinema.org/a/353811"><img src="https://i.imgur.com/InHB5DB.png" /></a></p>
<p align="center"><i>icons and colors asciicast</i></p>

<h3 align="center">[<a
href="https://github.com/jarun/nnn#features">Features</a>] [<a
href="https://github.com/jarun/nnn#quickstart">Quickstart</a>] [<a
href="https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins">Plugins</a>] [<a
href="https://github.com/jarun/nnn/wiki">Documentation</a>]</h3>

<br>

[![](https://user-images.githubusercontent.com/324519/94587860-062d7a80-0238-11eb-99b1-a9c9f0c32ac2.png)](https://www.youtube.com/embed/-knZwdd1ScU)

<br>

`nnn` (_n³_) is a full-featured terminal file manager. It's tiny and nearly 0-config with an [incredible speed](https://github.com/jarun/nnn/wiki/Performance).

It is designed to be unobtrusive with smart workflows to match the trains of thought.

`nnn` can analyze disk usage, batch rename, launch apps and pick files. The plugin repository has tons of plugins and documentation to extend the capabilities further e.g. [preview](https://github.com/jarun/nnn/wiki/Live-previews), (un)mount disks, find & list, file/dir diff, upload files.

There are 2 independent (neo)vim plugins - [nnn.vim](https://github.com/mcchrish/nnn.vim) and [vim-floaterm nnn wrapper](https://github.com/voldikss/vim-floaterm#nnn).

It runs smoothly on the Pi, [Termux](https://www.youtube.com/embed/AbaauM7gUJw) (Android), Linux, macOS, BSD, Haiku, Cygwin, WSL, across DEs and GUI utilities or a strictly CLI environment.

<details><summary><i><b>Expand</b></i> for some nnn magic! :dark_sunglasses:</summary><br><ul>
  <li>Instantly load, sort, filter thousands of files</li>
  <li>Type to navigate with automatic dir selection</li>
  <li>List input stream and pick entries to stdout or file</li>
  <li>find/fd/grep/ripgrep/fzf from nnn and list in nnn</li>
  <li> Never lose context - start where you quit</li>
  <li>Mount any cloud storage service in a few keypresses</li>
  <li>Select files from anywhere (not just a single dir)</li>
  <li>Unlimited bookmarks, plugins, cmds with custom hotkeys</li>
  <li>Write a plugin in any language you know</li>
  <li>Edit and preview markdown, man page, html</li>
  <li>Open a file and auto-advance to the next</li>
  <li>Filter filtered entries, export list of visible files</li>
  <li>Configure the middle mouse click to do anything</li>
  <li>Fuzzy search subtree and open a file (or its parent dir)</li>
  <li>Load four dirs with custom settings at once</li>
  <li>Notifications on cp, mv, rm completion</li>
  <li>Auto-sync selection to system clipboard</li>
  <li>Access selection from another instance of nnn</li>
  <li>Open text files detached in another pane/tab/window</li>
  <li>Mount and modify archives</li>
  <li>Create files/dirs/duplicates with parents (like <i>mkdir -p</i>)</li>
  <li>Toggle hidden with <kbd>.</kbd>, visit HOME with <kbd>~</kbd>, last dir with <kbd>-</kbd></li>
  <li>Mark a frequently visited dir at runtime</li>
  <li>Sort by modification, access and inode change time</li>
  <li>Compile out/in features with make variables</li>
  <li>Watch matrix text fly or read fortune messages</li>
  <li>Configure in 5 minutes!</li>
</ul></details>

<p align="center">
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=RMLTQ76JSXJ4Q"><img src="https://img.shields.io/badge/donate-@PayPal-1eb0fc.svg" alt="Donate via PayPal!" /></a>
</p>

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
  - Plugin to backup configuration
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
  - Create (with parents), rename, duplicate files and dirs
  - Batch renamer for selection or dir
  - List input stream of file paths from stdin or plugin
  - Copy (as), move (as), delete, archive, link selection
  - Dir updates, notification on cp, mv, rm completion
  - Copy file paths to system clipboard on select
  - Launch apps, run commands, spawn a shell, toggle exe
  - Access hovered file as `$nnn` at prompt or spawned shell
  - Lock terminal after configurable idle timeout
  - Basic support for screen readers and braille displays

## Quickstart

1. [Install](https://github.com/jarun/nnn/wiki/Usage) `nnn` and any dependencies you need. All files are opened with the desktop opener by default.
2. Add option `-e` to your alias to open text files in `$VISUAL`/`$EDITOR`/ vi. [Open detached](https://github.com/jarun/nnn/wiki/Basic-use-cases#detached-text) if you wish.
3. Configure [cd on quit](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit).
4. [Install plugins](https://github.com/jarun/nnn/tree/master/plugins#installation).
5. Use option `-x` to copy selected file paths to system clipboard and show notis on cp, mv, rm completion.
6. For a CLI-only environment, customize and use plugin [`nuke`](https://github.com/jarun/nnn/blob/master/plugins/nuke) with option `-c` (overrides `-e`).
7. Bid _ls_ goodbye! `alias ls='nnn -de'` :sunglasses:

Don't memorize! Arrows (or <kbd>h</kbd> <kbd>j</kbd> <kbd>k</kbd> <kbd>l</kbd>), <kbd>/</kbd>, <kbd>q</kbd> suffice. <kbd>Tab</kbd> creates, cycles contexts. <kbd>?</kbd> lists shortcuts.

[![Wiki](https://img.shields.io/badge/RTFM-nnn%20Wiki-important?maxAge=2592000)](https://github.com/jarun/nnn/wiki)

## Videos

- [nnn file manager on Termux (Android)](https://www.youtube.com/embed/AbaauM7gUJw)
- [NNN File Manager](https://www.youtube.com/embed/1QXU4XSqXNo)
- [This Week in Linux 114 - TuxDigital](https://www.youtube.com/watch?v=5W9ja0DQjSY&t=2059s)
- [nnn file manager basics - Linux](https://www.youtube.com/embed/il2Fm-KJJfM)
- [I'M GOING TO USE THE NNN FILE BROWSER! 😮](https://www.youtube.com/embed/U2n5aGqou9E)
- [NNN: Is This Terminal File Manager As Good As People Say?](https://www.youtube.com/embed/KuJHo-aO_FA)
- [nnn - A File Manager (By Uoou, again.)](https://www.youtube.com/embed/cnzuzcCPYsk)

## Elsewhere

- [AddictiveTips](https://www.addictivetips.com/ubuntu-linux-tips/navigate-linux-filesystem/)
- [ArchWiki](https://wiki.archlinux.org/index.php/Nnn)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [gHacks Tech News](https://www.ghacks.net/2019/11/01/nnn-is-an-excellent-command-line-based-file-manager-for-linux-macos-and-bsds/)
- Hacker News [[1](https://news.ycombinator.com/item?id=18520898)] [[2](https://news.ycombinator.com/item?id=19850656)]
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [Linux Format Issue 265; Manage files with nnn](https://linuxformat.com/archives?issue=265)
- LinuxLinks [[1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)] [[2](https://www.linuxlinks.com/bestconsolefilemanagers/)] [[3](https://www.linuxlinks.com/excellent-system-tools-nnn-portable-terminal-file-manager/)]
- [Linux Magazine; FOSSPicks](https://www.linux-magazine.com/Issues/2017/205/FOSSPicks/(offset)/15)
- [Make Tech Easier](https://www.maketecheasier.com/nnn-file-manager-terminal/)
- [Open Source For You](https://www.opensourceforu.com/2019/12/nnn-this-feature-rich-terminal-file-manager-will-enhance-your-productivity/)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine Issue 135; Review: nnn](https://fullcirclemagazine.org/issue-135/)
- [Using and Administering Linux: Volume 2: Zero to SysAdmin: Advanced Topics](https://books.google.com/books?id=MqjDDwAAQBAJ&pg=PA32)
- [Wikipedia](https://en.wikipedia.org/wiki/Nnn_(file_manager))

## Developers

- [Arun Prakash Jana](https://github.com/jarun) (Copyright © 2016-2021)
- [0xACE](https://github.com/0xACE)
- [Anna Arad](https://github.com/annagrram)
- [KlzXS](https://github.com/KlzXS)
- [Léo Villeveygoux](https://github.com/leovilok)
- [Maxim Baz](https://github.com/maximbaz)
- [Todd Yamakawa](https://github.com/toddyamakawa)
- and other contributors

Visit the [ToDo list](https://github.com/jarun/nnn/issues/781) to contribute or see the features in progress.
