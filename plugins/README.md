<h1 align="center">nnn plugins</h1>

<p align="center"><img src="https://i.imgur.com/SpT0L2W.png" /></p>
<p align="center"><i>read ebooks with plugin gutenread (Android)</i></p>

<p align="center"><img src="https://i.imgur.com/14iPDIq.png" /></p>
<p align="center"><i>image preview with plugin imgthumb</i></p>

## Introduction

Plugins extend the capabilities of `nnn`. They are _executable_ scripts (or binaries) which `nnn` can communicate with and trigger. This mechanism fits perfectly with the fundamental design to keep the core file manager lean and fast, by delegating repetitive (but not necessarily file manager-specific) tasks to the plugins.

## List of plugins

| Plugin (a-z) | Description | Lang | Deps |
| --- | --- | --- | --- |
| boom | Play random music from dir | sh | [moc](http://moc.daper.net/) |
| dups | List non-empty duplicate files in current dir | sh | find, md5sum,<br>sort uniq xargs |
| chksum | Create and verify checksums | sh | md5sum,<br>sha256sum |
| diffs | Diff for selection (limited to 2 for directories) | sh | vimdiff |
| dragdrop | Drag/drop files from/into nnn | sh | [dragon](https://github.com/mwh/dragon) |
| exetoggle | Toggle executable status of hovered file | sh | chmod |
| fzcd | Change to the directory of a fuzzy-selected file/dir | sh | fzf/fzy<br>fd/fdfind/find |
| fzhist | Fuzzy-select a cmd from history, edit in `$EDITOR` and run | sh | fzf/fzy |
| fzopen | Fuzzy find a file in dir subtree and edit or open | sh | fzf/fzy, xdg-open |
| getplugs | Update plugins | sh | curl |
| gutenread | Browse, download, read from Project Gutenberg | sh | curl, unzip, w3m<br>[epr](https://github.com/wustho/epr) (optional) |
| hexview | View a file in hex in `$PAGER` | sh | xxd |
| imgresize | Resize images in dir to screen resolution | sh | [imgp](https://github.com/jarun/imgp) |
| imgsxiv | Browse images, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts)), [rename](https://github.com/jarun/nnn/wiki/Basic-use-cases#browse-rename-images)| sh | sxiv |
| imgthumb | View thumbnail of an image or dir of images | sh | [lsix](https://github.com/hackerb9/lsix) |
| imgur | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) | bash | - |
| imgviu | View an image or images in dir in `$PAGER` | sh | [viu](https://github.com/atanunq/viu), less |
| ipinfo | Fetch external IP address and whois information | sh | curl, whois |
| jump | Navigate to dir/path (**autojump stores navigation patterns**) | sh | autojump |
| kdeconnect | Send selected files to an Android device | sh | kdeconnect-cli |
| launch | GUI application launcher | sh | fzf/fzy |
| mediainf | Show media information | sh | mediainfo |
| moclyrics | Show lyrics of the track playing in moc | sh | [ddgr](https://github.com/jarun/ddgr), [moc](http://moc.daper.net/) |
| mocplay | Append (and/or play) selection/dir/file in moc | sh | [moc](http://moc.daper.net/) |
| nmount | Toggle mount status of a device as normal user | sh | pmount, udisks2 |
| notes | Open a quick notes file/dir in `$EDITOR` | sh | - |
| nuke | Sample file opener (CLI-only by default) | sh | various |
| oldbigfile | List large files by access time | sh | find, sort |
| organize | Auto-organize files in directories by file type | sh | file |
| pastebin | Paste contents of a text a file ix.io | sh | - |
| pdfread | Read a PDF or text file aloud | sh | pdftotext, mpv,<br>pico2wave |
| pdfview | View PDF file in `$PAGER` | sh | pdftotext/<br>mupdf-tools |
| picker | Pick files and list one per line (to pipe) | sh | nnn |
| pskill | Fuzzy list by name and kill process or zombie | sh | fzf/fzy, ps,<br>sudo/doas |
| renamer | Batch rename selection or files in dir | sh | [qmv](https://www.nongnu.org/renameutils/)/[vidir](https://joeyh.name/code/moreutils/) |
| ringtone | Create a variable bitrate mp3 ringtone from file | sh | date, ffmpeg |
| splitjoin | Split file or join selection | sh | split, cat |
| suedit | Edit file using superuser permissions | sh | sudoedit/sudo/doas |
| transfer | Upload file to transfer.sh | sh | curl |
| treeview | Informative tree output in `$EDITOR` | sh | tree |
| uidgid | List user and group of all files in dir | sh | ls, less |
| upgrade | Upgrade nnn manually on Debian 9 Stretch | sh | curl |
| vidthumb | Show video thumbnails in terminal | sh | [ffmpegthumbnailer](https://github.com/dirkvdb/ffmpegthumbnailer),<br>[lsix](https://github.com/hackerb9/lsix) |
| wall | Set wallpaper or change colorscheme | sh | nitrogen/pywal |

## Installing plugins

The following command installs all plugins:

    curl -Ls https://raw.githubusercontent.com/jarun/nnn/master/plugins/getplugs | sh

Plugins are installed to `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`. You can run the `getplugs` plugin later to update the plugins. It backs up earlier plugins.

## Executing plugins

**Method 1:** Directly with <kbd>;key</kbd> or <kbd>xkey</kbd>:

    export NNN_PLUG='o:fzopen;p:mocplay;d:diffs;m:nmount;n:notes;v:imgviu;t:imgthumb'

Now plugin `fzopen` can be run with the keybind <kbd>;o</kbd>, `mocplay` can be run with <kbd>;p</kbd> and so on... The key vs. plugin pairs are shown in the help and config screen.

**Method 2:** Use the _pick plugin_ shortcut to visit the plugin directory and execute a plugin. Repeating the same shortcut cancels the operation and puts you back in the original directory.

## Running commands as plugin

To assign keys to arbitrary non-background cli commands (non-shell-interpreted) and invoke like plugins, add `_` (underscore) before the command.

For example:

    export NNN_PLUG='x:_chmod +x $nnn;g:_git log;s:_smplayer $nnn;o:fzopen'

Now <kbd>;x</kbd> can be used to make a file executable, <kbd>;g</kbd> can be used to the git log of a git project directory, <kbd>;s</kbd> can be used to preview a partially downloaded media file.

`nnn` waits for user confirmation (the prompt `Press Enter to continue`) when it executes a command as plugin (unline plugins which can add a `read` to wait). If you do not need to wait for user confirmation after the command has executed, add a `*` after the command. For example:

    export NNN_PLUG='x:_chmod +x $nnn;g:_git log;s:_smplayer $nnn*;o:fzopen'

Now there will be no prompt after <kbd>;s</kbd>.

Notes:

1. Use single quotes for `$NNN_PLUG` so `$nnn` is not interpreted
2. `$nnn` should be the last argument (IF you want to pass the hovered file name)
3. (_Again_) add `_` before the command

## Access level of plugins

When `nnn` executes a plugin, it does the following:
- Change to the directory where the plugin is to be run (`$PWD` pointing to the active directory)
- Passes two arguments to the script:
    1. The hovered file's name.
    2. The working directory (might differ from `$PWD` in case of symlinked paths; non-canonical). Note that the second argument is not passed in case of commands starting with `_`.
- Sets the environment variable `NNN_PIPE` used to control `nnn` active directory.

Plugins can also access the current selections by reading the `.selections` file in the config directory (See the `diffs` plugin for example).

## Create your own plugins

Plugins are scripts that can be written in any scripting language. However, POSIX-compliant shell scripts runnable in `sh` are preferred.

Once it's ready, drop the plugin in `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins` and make it executable. Optionally add a custom keybind in `$NNN_PLUG` if you intend to use the plugin frequently.

#### Controlling `nnn`'s active directory
`nnn` provides a mechanism for plugins to control its active directory.
The way to do so is by writing to the pipe pointed by the environment variable `NNN_PIPE`.
The plugin should write a single string in the format `<number><path>` without a newline at the end. For example, `1/etc`.
The number indicates the context to change the active directory of (0 is used to indicate the current context).

For convenience, we provided a helper script named `.nnn-plugin-helper` and a function named `nnn_cd` to ease this process. `nnn_cd` receives the path to change to as the first argument, and the context as an optional second argument.
If a context is not provided, it is asked for explicitly.
Usage examples can be found in the Examples section below.

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

- Change direcory to the location of a link using helper script with specific context (current)
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

    printf "%s" "0$dir" > "$NNN_PIPE"
    ```

## Contributing plugins

Add informative sections like _Description_, _Notes_, _Dependencies_, _Shell_, _Author_ etc. in the plugin as applicable. Add an entry in the table above. Please keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.

The plugins should be executable.
