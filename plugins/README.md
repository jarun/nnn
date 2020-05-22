<h1 align="center">nnn plugins</h1>

<p align="center"><img src="https://i.imgur.com/SpT0L2W.png" /></p>
<p align="center"><i>read ebooks with plugin gutenread (Android)</i></p>

<p align="center"><img src="https://i.imgur.com/14iPDIq.png" /></p>
<p align="center"><i>image preview with plugin imgthumb</i></p>

## Introduction

Plugins extend the capabilities of `nnn`. They are _executable_ scripts (or binaries) `nnn` can communicate with and trigger. This mechanism fits perfectly with the fundamental design to keep the core file manager lean and fast, by delegating repetitive (but not necessarily file manager-specific) tasks to the plugins which can be run with custom hotkeys.

`nnn` is _**language-agnostic**_ when it comes to plugins. You can write a plugin in any (scripting) language you are comfortable in!

## Installing plugins

The following command installs or updates (after backup) all plugins:

```sh
curl -Ls https://raw.githubusercontent.com/jarun/nnn/master/plugins/getplugs | sh
```

Plugins are installed to `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`.

## List of plugins

| Plugin (a-z) | Description | Lang | Dependencies |
| --- | --- | --- | --- |
| autojump | Navigate to dir/path | sh | autojump |
| bookmarks | Use named bookmarks managed with symlinks | sh | fzf |
| boom | Play random music from dir | sh | [moc](http://moc.daper.net/) |
| dups | List non-empty duplicate files in current dir | sh | find, md5sum,<br>sort uniq xargs |
| chksum | Create and verify checksums | sh | md5sum,<br>sha256sum |
| diffs | Diff for selection (limited to 2 for directories) | sh | vimdiff |
| dragdrop | Drag/drop files from/into nnn | sh | [dragon](https://github.com/mwh/dragon) |
| finder | Run custom find command and list | sh | - |
| fzcd | Change to the directory of a fuzzy-selected file/dir | sh | fzf |
| fzhist | Fuzzy-select a cmd from history, edit in `$EDITOR` and run | sh | fzf |
| fzopen | Fuzzy find a file in dir subtree and edit or open | sh | fzf, xdg-open |
| fzz | Change to any directory in the z database with fzf | sh | fzf, z |
| getplugs | Update plugins to installed `nnn` version | sh | curl |
| gutenread | Browse, download, read from Project Gutenberg | sh | curl, unzip, w3m<br>[epr](https://github.com/wustho/epr) (optional) |
| hexview | View a file in hex in `$PAGER` | sh | xxd |
| imgresize | Resize images in dir to screen resolution | sh | [imgp](https://github.com/jarun/imgp) |
| imgthumb | View thumbnail of an image or dir of images | sh | [lsix](https://github.com/hackerb9/lsix) |
| imgur | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) | bash | - |
| imgview | Browse images, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts)), [rename](https://github.com/jarun/nnn/wiki/Basic-use-cases#browse-rename-images)| sh | sxiv/[viu](https://github.com/atanunq/viu), less|
| ipinfo | Fetch external IP address and whois information | sh | curl, whois |
| kdeconnect | Send selected files to an Android device | sh | kdeconnect-cli |
| launch | GUI application launcher | sh | fzf |
| mediainf | Show media information | sh | mediainfo |
| mimelist | List files by mime in subtree | sh | fd/find |
| moclyrics | Show lyrics of the track playing in moc | sh | [ddgr](https://github.com/jarun/ddgr), [moc](http://moc.daper.net/) |
| mocplay | Append (and/or play) selection/dir/file in moc | sh | [moc](http://moc.daper.net/) |
| mp3conv | Extract audio from multimedia as mp3 | sh | ffmpeg |
| nbak | Backs up `nnn` config | sh | tar, awk |
| nmount | Toggle mount status of a device as normal user | sh | pmount, udisks2 |
| nuke | Sample file opener (CLI-only by default) | sh | _see in-file docs_ |
| oldbigfile | List large files by access time | sh | find, sort |
| organize | Auto-organize files in directories by file type | sh | file |
| pdfread | Read a PDF or text file aloud | sh | pdftotext, mpv,<br>pico2wave |
| pdfview | View PDF file in `$PAGER` | sh | pdftotext/<br>mupdf-tools |
| picker | Pick files and list one per line (to pipe) | sh | nnn |
| preview-kitty | File previewer in a kitty terminal pane | sh | [kitty](https://sw.kovidgoyal.net/kitty/), optional:<br>[exa](https://github.com/ogham/exa) [bat](https://github.com/sharkdp/bat) mediainfo |
| preview-tabbed | `tabbed`/xembed based file previewer | bash | _see in-file docs_ |
| preview-tui | Simple TUI file previewer (needs NNN_FIFO) | sh | tmux (>= 3.0)/xterm/<br>\$TERMINAL, file, tree |
| pskill | Fuzzy list by name and kill process or zombie | sh | fzf, ps, sudo/doas |
| renamer | Batch rename selection or files in dir | sh | [qmv](https://www.nongnu.org/renameutils/)/[vidir](https://joeyh.name/code/moreutils/) |
| ringtone | Create a variable bitrate mp3 ringtone from file | sh | date, ffmpeg |
| splitjoin | Split file or join selection | sh | split, cat |
| suedit | Edit file using superuser permissions | sh | sudoedit/sudo/doas |
| treeview | Informative tree output in `$EDITOR` | sh | tree |
| uidgid | List user and group of all files in dir | sh | ls, less |
| upgrade | Upgrade nnn manually on Debian 9 Stretch | sh | curl |
| upload | Paste text to ix.io, upload binary to file.io | sh | curl, jq, tr |
| vidthumb | Show video thumbnails in terminal | sh | [ffmpegthumbnailer](https://github.com/dirkvdb/ffmpegthumbnailer),<br>[lsix](https://github.com/hackerb9/lsix) |
| wall | Set wallpaper or change colorscheme | sh | nitrogen/pywal |
| x2sel | Copy `\n`-separated file list from system clipboard to sel | sh | _see in-file docs_ |

## Invoking a plugin

Press the plugin shortcut (<kbd>;</kbd> or <kbd>^S</kbd>) followed by the assigned key character. E.g., with the below config:

```sh
export NNN_PLUG='f:finder;o:fzopen;p:mocplay;d:diffs;t:nmount;v:imgview'
```

Plugin `finder` can be run with the keybind <kbd>;f</kbd>, `fzopen` can be run with <kbd>;o</kbd> and so on... The key vs. plugin pairs are shown in the help and config screen.

Alternatively, combine with <kbd>Alt</kbd> (i.e. <kbd>Alt+key</kbd>).

To select and invoke a plugin from the plugin directory, press <kbd>Enter</kbd> (to _enter_ the plugin dir) after the plugin shortcut.

#### Skip directory refresh after running a plugin

`nnn` refreshes the directory after running a plugin to reflect any changes by the plugin. To disable this (say while running the `mediainfo` plugin on some filtered files), add a `-` before the plugin name:

```sh
export NNN_PLUG='m:-mediainf'
```

Now `nnn` will not refresh the directory after running the `mediainfo` plugin.

## Running commands as plugin

To assign keys to arbitrary non-background, non-shell-interpreted cli commands and invoke like plugins, add `_` (underscore) before the command.

For example:

```sh
export NNN_PLUG='x:_chmod +x $nnn;g:_git log;s:_smplayer $nnn'
```

Now <kbd>;x</kbd> can be used to make a file executable, <kbd>;g</kbd> can be used to the git log of a git project directory, <kbd>;s</kbd> can be used to preview a partially downloaded media file.

#### Skip user confirmation after command execution

`nnn` waits for user confirmation (the prompt `Press Enter to continue`) after it executes a command as plugin (unlike plugins which can add a `read` to wait). To skip this, add a `*` after the command. For example:

```sh
export NNN_PLUG='s:_smplayer $nnn*;n:-_vim /home/vaio/Dropbox/Public/synced_note*'
```

Now there will be no prompt after <kbd>;s</kbd> and <kbd>;n</kbd>.

Note: Do not use `*` with programs those run and exit e.g. cat.

#### Run GUI app as plugin

To run a GUI app as plugin, add a `|` after `_`. For example:

```sh
export NNN_PLUG='m:-_|mousepad $nnn'
```

Notes:

1. Use single quotes for `$NNN_PLUG` so `$nnn` is not interpreted
2. `$nnn` should be the last argument (IF used)
3. (_Again_) add `_` before the command
4. To disable directory refresh after running a _command as plugin_, prefix with `-_`

#### Some useful key-command examples

| Key:Command | Description |
|---|---|
| `g:-_git diff` | Show git diff |
| `k:-_fuser -kiv $nnn*` | Interactively kill process(es) using hovered file |
| `l:-_git log` | Show git log |
| `n:-_vi /home/user/Dropbox/dir/note*` | Take quick notes in a synced file/dir of notes |
| `p:-_less -iR $nnn*` | Page through hovered file in less |
| `s:-_\|smplayer -minigui $nnn` | Play hovered media file, even unfinished download |
| `x:_chmod +x $nnn` | Make the hovered file executable |
| `y:-_sync*` | Flush cached writes |

## Access level of plugins

When `nnn` executes a plugin, it does the following:
- Changes to the directory where the plugin is to be run (`$PWD` pointing to the active directory)
- Passes two arguments to the script:
    1. The hovered file's name.
    2. The working directory (might differ from `$PWD` in case of symlinked paths; non-canonical).
- Sets the environment variable `NNN_PIPE` used to control `nnn` active directory.

Plugins can also read the `.selection` file in the config directory.

## Create your own plugins

Plugins can be written in any scripting language. However, POSIX-compliant shell scripts runnable in `sh` are preferred.

Drop the plugin in `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins` and make it executable. Optionally add a hotkey in `$NNN_PLUG` for frequent usage.

#### Send data to `nnn`
`nnn` provides a mechanism for plugins to send data to `nnn` to control its active directory or invoke the list mode.
The way to do so is by writing to the pipe pointed by the environment variable `NNN_PIPE`.
The plugin should write a single string in the format `<ctxcode><opcode><data>` without a newline at the end. For example, `1c/etc`.

The `ctxcode` indicates the context to change the active directory of.

| Context code | Meaning |
|:---:| --- |
| `1`-`4` | context number |
| `0` | current context |
| `+` | smart context (next inactive else current) |

The `opcode` indicates the operation type.

| Opcode | Operation |
|:---:| --- |
| `c` | change directory |
| `l` | list files in list mode |

For convenience, we provided a helper script named `.nnn-plugin-helper` and a function named `nnn_cd` to ease this process. `nnn_cd` receives the path to change to as the first argument, and the context as an optional second argument.
If a context is not provided, it is asked for explicitly. To skip this and choose the current context, set the `CUR_CTX` variable in `.nnn-plugin-helper` to `1`.
Usage examples can be found in the Examples section below.

#### Get notified on file hover

If `NNN_FIFO` is set, `nnn` will open it and write every hovered files. This can be used in plugins and external scripts, e.g. to implement file previews.

The easiest way to set `NNN_FIFO` is to start `nnn` with the `-a` option, to automatically setup a temporary FIFO file for this `nnn` instance.

If a `NNN_FIFO` environment variable is set globally (and `-a` is not passed to `nnn`), each `nnn` instance will write to the same FIFO, and a process reading from the pipe will get hovered path from every instance, interleaved.

Don't forget to fork in the background to avoid blocking `nnn`.

#### Examples
There are many plugins provided by `nnn` which can be used as examples. Here are a few simple selected examples.

- Show the git log of changes to the particular file along with the code for a quick and easy review.
    ```sh
    #!/usr/bin/env sh
    git log -p -- "$1"
    ```

- Change to directory in clipboard using helper script
    ```sh
    #!/usr/bin/env sh
    . $(dirname $0)/.nnn-plugin-helper

    nnn_cd "$(xsel -ob)"
    ```

- Change directory to the location of a link using helper script with specific context (current)
    ```sh
    #!/usr/bin/env sh
    . $(dirname $0)/.nnn-plugin-helper

    nnn_cd "$(dirname $(readlink -fn $1))" 0
    ```

- Change to arbitrary directory without helper script
    ```sh
    #!/usr/bin/env sh
    printf "cd to: "
    read -r dir

    printf "%s" "0c$dir" > "$NNN_PIPE"
    ```

- Send every hovered file to X selection
    ```sh
    #!/usr/bin/env sh
    if [ -z "$NNN_FIFO" ] ; then
        exit 1
    fi

    while read FILE ; do
        printf "%s" "$FILE" | xsel
    done < "$NNN_FIFO" &
    disown
    ```

## Contributing plugins

1. Add informative sections like _Description_, _Notes_, _Dependencies_, _Shell_, _Author_ etc. in the plugin.
2. Add an entry in the table above.
3. Keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.
4. The plugin file should be executable.
5. If your plugin stores data, use `${XDG_CACHE_HOME:-$HOME/.cache}/nnn`. Document it _in-file_.
