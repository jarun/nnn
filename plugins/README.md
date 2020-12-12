<h1 align="center">nnn plugins</h1>

<p align="center"><img src="https://i.imgur.com/SpT0L2W.png" /></p>
<p align="center"><i>read ebooks with plugin gutenread (Android)</i></p>

<p align="center"><a href="https://asciinema.org/a/336443"><img src="https://asciinema.org/a/336443.svg" width="734"/></a></p>
<p align="center"><i>Live Previews</i></p>

## Introduction

Plugins extend the capabilities of `nnn`. They are _executable_ scripts (or binaries) `nnn` can communicate with and trigger. This mechanism fits perfectly with the fundamental design to keep the core file manager lean and fast, by delegating repetitive (but not necessarily file manager-specific) tasks to the plugins which can be run with custom hotkeys.

`nnn` is _**language-agnostic**_ when it comes to plugins. You can write a plugin in any (scripting) language you are comfortable in!

## List of plugins

| Plugin (a-z) | Description | Lang | Dependencies |
| --- | --- | --- | --- |
| [autojump](autojump) | Navigate to dir/path | sh | [jump](https://github.com/gsamokovarov/jump)/autojump/zoxide |
| [bookmarks](bookmarks) | Use named bookmarks managed with symlinks | sh | fzf |
| [boom](boom) | Play random music from dir | sh | [moc](http://moc.daper.net/) |
| [bulknew](bulknew) | Create multiple files/dirs at once | bash | sed, xargs, mktemp |
| [dups](dups) | List non-empty duplicate files in current dir | bash | find, md5sum,<br>sort uniq xargs |
| [chksum](chksum) | Create and verify checksums | sh | md5sum,<br>sha256sum |
| [diffs](diffs) | Diff for selection (limited to 2 for directories) | sh | vimdiff, mktemp |
| [dragdrop](dragdrop) | Drag/drop files from/into nnn | sh | [dragon](https://github.com/mwh/dragon) |
| [finder](finder) | Run custom find command and list | sh | - |
| [fzcd](fzcd) | Change to the directory of a fuzzy-selected file/dir | sh | fzf |
| [fzhist](fzhist) | Fuzzy-select a cmd from history, edit in `$EDITOR` and run | sh | fzf, mktemp |
| [fzopen](fzopen) | Fuzzy find a file in dir subtree and edit or open | sh | fzf, xdg-open |
| [fzz](fzz) | Change to any directory in the z database with fzf | sh | fzf, z |
| [getplugs](getplugs) | Update plugins to installed `nnn` version | sh | curl |
| [gutenread](gutenread) | Browse, download, read from Project Gutenberg | sh | curl, unzip, w3m<br>[epr](https://github.com/wustho/epr) (optional) |
| [gpg\*](gpg\*) | Encrypt/decrypt files using GPG | sh | gpg |
| [hexview](hexview) | View a file in hex in `$PAGER` | sh | [hx](https://github.com/krpors/hx)/xxd |
| [imgresize](imgresize) | Resize images in dir to screen resolution | sh | [imgp](https://github.com/jarun/imgp) |
| [imgthumb](imgthumb) | View thumbnail of an image or dir of images | sh | [lsix](https://github.com/hackerb9/lsix) |
| [imgur](imgur) | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) | bash | - |
| [imgview](imgview) | Browse images, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts)), [rename](https://github.com/jarun/nnn/wiki/Basic-use-cases#browse-rename-images)| sh | [imv](https://github.com/eXeC64/imv)/[sxiv](https://github.com/muennich/sxiv)/[viu](https://github.com/atanunq/viu), less|
| [ipinfo](ipinfo) | Fetch external IP address and whois information | sh | curl, whois |
| [kdeconnect](kdeconnect) | Send selected files to an Android device | sh | kdeconnect-cli |
| [launch](launch) | GUI application launcher | sh | fzf |
| [mediainf](mediainf) | Show media information | sh | mediainfo |
| [mimelist](mimelist) | List files by mime in subtree | sh | - |
| [moclyrics](moclyrics) | Show lyrics of the track playing in moc | sh | [ddgr](https://github.com/jarun/ddgr), [moc](http://moc.daper.net/) |
| [mocplay](mocplay) | Append (and/or play) selection/dir/file in moc | sh | [moc](http://moc.daper.net/) |
| [mp3conv](mp3conv) | Extract audio from multimedia as mp3 | sh | ffmpeg |
| [nbak](nbak) | Backs up `nnn` config | sh | tar, awk, mktemp |
| [nmount](nmount) | Toggle mount status of a device as normal user | sh | pmount, udisks2 |
| [nuke](nuke) | Sample file opener (CLI-only by default) | sh | _see in-file docs_ |
| [oldbigfile](oldbigfile) | List large files by access time | sh | find, sort |
| [organize](organize) | Auto-organize files in directories by file type | sh | file |
| [pdfread](pdfread) | Read a PDF or text file aloud | sh | pdftotext, mpv,<br>pico2wave |
| [pdfview](pdfview) | View PDF file in `$PAGER` | sh | pdftotext/<br>mupdf-tools |
| [picker](picker) | Pick files and list one per line (to pipe) | sh | nnn |
| [preview-tabbed](preview-tabbed) | Tabbed/xembed based file previewer | bash | _see in-file docs_ |
| [preview-tui](preview-tui) | Tmux/kitty/xterm/`$TERMINAL` based file previewer | sh | _see in-file docs_ |
| [pskill](pskill) | Fuzzy list by name and kill process or zombie | sh | fzf, ps, sudo/doas |
| [renamer](renamer) | Batch rename selection or files in dir | sh | [qmv](https://www.nongnu.org/renameutils/)/[vidir](https://joeyh.name/code/moreutils/) |
| [ringtone](ringtone) | Create a variable bitrate mp3 ringtone from file | sh | date, ffmpeg |
| [splitjoin](splitjoin) | Split file or join selection | sh | split, cat |
| [suedit](suedit) | Edit file using superuser permissions | sh | sudoedit/sudo/doas |
| [togglex](togglex) | Toggle executable mode for selection | sh | chmod |
| [treeview](treeview) | Informative tree output in `$EDITOR` | sh | tree |
| [uidgid](uidgid) | List user and group of all files in dir | sh | ls, less |
| [upgrade](upgrade) | Upgrade nnn manually on Debian 9 Stretch | sh | curl |
| [upload](upload) | Upload to Firefox Send or ix.io (text) or file.io (bin) | sh | [ffsend](https://github.com/timvisee/ffsend), curl, jq, tr |
| [vidthumb](vidthumb) | Show video thumbnails in terminal | sh | [ffmpegthumbnailer](https://github.com/dirkvdb/ffmpegthumbnailer),<br>[lsix](https://github.com/hackerb9/lsix) |
| [wall](wall) | Set wallpaper or change colorscheme | sh | nitrogen/pywal |
| [x2sel](x2sel) | Copy `\n`-separated file list from system clipboard to sel | sh | _see in-file docs_ |

## Installation

The following command installs or updates (after backup) all plugins:

```sh
curl -Ls https://raw.githubusercontent.com/jarun/nnn/master/plugins/getplugs | sh
```

Plugins are installed to `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`.

## Configuration

Set environment variable `NNN_PLUG` to assign keybinds and invoke plugins directly using the plugin shortcut (<kbd>;</kbd>) followed by the assigned key character. E.g., with the below config:

```sh
export NNN_PLUG='f:finder;o:fzopen;p:mocplay;d:diffs;t:nmount;v:imgview'
```

plugin `finder` can be invoked with the keybind <kbd>;f</kbd>, `fzopen` can be run with <kbd>;o</kbd> and so on... The key vs. plugin pairs are shown in the help and config screen.

Alternatively, combine with <kbd>Alt</kbd> (i.e. <kbd>Alt+key</kbd>).

To pick and run an unassigned plugin, press <kbd>Enter</kbd> (to _enter_ the plugin dir) at the plugin prompt.

To run a plugin at startup, use the option `-P` followed by the plugin key.

If the plugins list gets too long, try breaking them up into sections:

```
NNN_PLUG_PERSONAL='g:personal/convert2zoom;p:personal/echo'
NNN_PLUG_WORK='j:work/prettyjson;d:work/foobar'
NNN_PLUG_INLINE='e:_go run $nnn*'
NNN_PLUG_DEFAULT='1:bookmarks;2:ipinfo;p:preview-tui;o:fzz;b:nbak'
NNN_PLUG="$NNN_PLUG_PERSONAL;$NNN_PLUG_WORK;$NNN_PLUG_DEFAULT;$NNN_PLUG_INLINE"
export NNN_PLUG
```

Note:
- `'g:personal/convert2zoom'` will look in the personal sub-folder inside the plugin folder.
- `'b:boom;b:bookmarks` will result in only the first definition of *b* (`b:boom`) being used.
- A keybinding definition of more than 1 character will prevent nnn from starting.


#### Skip directory refresh after running a plugin

`nnn` refreshes the directory after running a plugin to reflect any changes by the plugin. To disable this (say while running the `mediainf` plugin on some filtered files), add a `-` before the plugin name:

```sh
export NNN_PLUG='m:-mediainf'
```

Now `nnn` will not refresh the directory after running the `mediainf` plugin.

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

Don't forget to fork in the background to avoid blocking `nnn`.

For more details on configuration and usage of the preview plugins, visit [Live Previews](https://github.com/jarun/nnn/wiki/Live-previews).

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

- Quick find (using `fd`)
    ```sh
    #!/usr/bin/env sh

    . "$(dirname "$0")"/.nnn-plugin-helper

    printf "pattern: "
    read -r pattern

    if ! [ -z "$pattern" ]; then
        printf "%s" "+l" > "$NNN_PIPE"
        eval "fd -HI $pattern -0" > "$NNN_PIPE"
    fi
    ```

- Quick grep (using `rg`)
    ```sh
    #!/usr/bin/env sh

    . "$(dirname "$0")"/.nnn-plugin-helper

    printf "pattern: "
    read -r pattern

    if ! [ -z "$pattern" ]; then
        printf "%s" "+l" > "$NNN_PIPE"
        eval "rg -l0 --hidden -S $pattern" > "$NNN_PIPE"
    fi
    ```

## Contributing plugins

1. Add informative sections like _Description_, _Notes_, _Dependencies_, _Shell_, _Author_ etc. in the plugin.
2. Add an entry in the table above.
3. Keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.
4. The plugin file should be executable.
5. If your plugin stores data, use `${XDG_CACHE_HOME:-$HOME/.cache}/nnn`. Document it _in-file_.
