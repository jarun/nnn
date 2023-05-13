<h1 align="center">nnn plugins</h1>

<p align="center"><img src="https://i.imgur.com/SpT0L2W.png" /></p>
<p align="center"><i>read ebooks with plugin gutenread (Android)</i></p>

## Introduction

Plugins extend the capabilities of `nnn`. They are _executable_ scripts (or binaries) `nnn` can communicate with and trigger. This mechanism fits perfectly with the fundamental design to keep the core file manager lean and fast, by delegating repetitive (but not necessarily file manager-specific) tasks to the plugins which can be run with custom hotkeys.

`nnn` is _**language-agnostic**_ when it comes to plugins. You can write a plugin in any (scripting) language you are comfortable in!

## List of plugins

| Plugin (a-z) | Description [Clears selection<sup>1</sup>] | Lang | Dependencies |
| --- | --- | --- | --- |
| [autojump](autojump) | Navigate to dir/path | sh | [jump](https://github.com/gsamokovarov/jump)/autojump/<br>zoxide/z/[z.lua](https://github.com/skywind3000/z.lua) |
| [boom](boom) | Play random music from dir | sh | [moc](http://moc.daper.net/) |
| [bulknew](bulknew) | Create multiple files/dirs at once | bash | sed, xargs, mktemp |
| [cdpath](cdpath) | `cd` to the directory from `CDPATH` | sh | fzf |
| [chksum](chksum) | Create and verify checksums [✓] | sh | md5sum,<br>sha256sum |
| [cmusq](cmusq) | Queue/play files/dirs in cmus player [✓] | sh | cmus, pgrep |
| [diffs](diffs) | Diff for selection (limited to 2 for directories) [✓] | sh | vimdiff, mktemp |
| [dragdrop](dragdrop) | Drag/drop files from/into nnn | sh | [dragon](https://github.com/mwh/dragon) |
| [dups](dups) | List non-empty duplicate files in current dir | bash | find, md5sum,<br>sort uniq xargs |
| [finder](finder) | Run custom find command (**stored in histfile**) and list | sh | - |
| [fixname](fixname) | Clean filename to be more shell-friendly [✓] | bash | sed |
| [fzcd](fzcd) | Fuzzy search multiple dirs (or `$PWD`) and visit file | sh | fzf, (find) |
| [fzhist](fzhist) | Fuzzy-select a cmd from history, edit in `$EDITOR` and run | sh | fzf, mktemp |
| [fzopen](fzopen) | Fuzzy find file(s) in subtree to edit/open/pick | sh | fzf, xdg-open/open |
| [fzplug](fzplug) | Fuzzy find, preview and run other plugins | sh | fzf |
| [getplugs](getplugs) | Update plugins to installed `nnn` version | sh | curl |
| [gitroot](gitroot) | Cd to the root of current git repo | sh | git |
| [gpge](gpge) | Encrypt/decrypt files using GPG [✓] | sh | gpg |
| [gutenread](gutenread) | Browse, download, read from Project Gutenberg | sh | curl, unzip, w3m<br>[epr](https://github.com/wustho/epr) (optional) |
| [gsconnect](gsconnect) | GNOME's implementation of kdeconnect [✓] | sh | gsconnect |
| [imgresize](imgresize) | Batch resize images in dir to screen resolution | sh | [imgp](https://github.com/jarun/imgp) |
| [imgur](imgur) | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) | bash | - |
| [imgview](imgview) | View (thumbnail)images, set wallpaper, [rename](https://github.com/jarun/nnn/wiki/Basic-use-cases#browse-rename-images) and [more](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts)| sh | _see in-file docs_ |
| [ipinfo](ipinfo) | Fetch external IP address and whois information | sh | curl, whois |
| [kdeconnect](kdeconnect) | Send selected files to an Android device [✓] | sh | kdeconnect-cli |
| [launch](launch) | GUI application launcher | sh | fzf |
| [mimelist](mimelist) | List files by mime in subtree | sh | - |
| [moclyrics](moclyrics) | Show lyrics of the track playing in moc | sh | [ddgr](https://github.com/jarun/ddgr), [moc](http://moc.daper.net/) |
| [mocq](mocq) | Queue/play selection/dir/file in moc [✓] | sh | [moc](http://moc.daper.net/) |
| [mp3conv](mp3conv) | Extract audio from multimedia as mp3 | sh | ffmpeg |
| [mtpmount](mtpmount) | Toggle mount of MTP device (eg. Android) | sh | gvfs-mtp |
| [nbak](nbak) | Backs up `nnn` config | sh | tar, awk, mktemp |
| [nmount](nmount) | Toggle mount status of a device as normal user | sh | pmount, udisks2 |
| [nuke](nuke) | Sample file opener (CLI-only by default) | sh | _see in-file docs_ |
| [oldbigfile](oldbigfile) | List large files by access time | sh | find, sort |
| [openall](openall) | Open selected files together or one by one [✓] | bash | - |
| [organize](organize) | Auto-organize files in directories by file type [✓] | sh | file |
| [pdfread](pdfread) | Read a PDF or text file aloud | sh | pdftotext, mpv,<br>pico2wave |
| [preview-tabbed](preview-tabbed) | Preview files with Tabbed/xembed | bash | _see in-file docs_ |
| [preview-tui](preview-tui) | Preview with Tmux/kitty/[QuickLook](https://github.com/QL-Win/QuickLook)/xterm/`$TERMINAL` | sh | _see in-file docs_ |
| [pskill](pskill) | Fuzzy list by name and kill process or zombie | sh | fzf, ps, sudo/doas |
| [renamer](renamer) | Batch rename selection or files in dir [✓] | sh | [qmv](https://www.nongnu.org/renameutils/)/[vidir](https://joeyh.name/code/moreutils/) |
| [ringtone](ringtone) | Create a variable bitrate mp3 ringtone from file | sh | date, ffmpeg |
| [rsynccp](rsynccp) | Gives copy-paste verbose progress percentage [✓] | sh | rsync |
| [splitjoin](splitjoin) | Split file or join selection [✓] | sh | split, cat |
| [suedit](suedit) | Edit file using superuser permissions | sh | sudoedit/sudo/doas |
| [togglex](togglex) | Toggle executable mode for selection [✓] | sh | chmod |
| [umounttree](umounttree) | Unmount a remote mountpoint from within | sh | fusermount |
| [upload](upload) | Upload to Firefox Send or ix.io (text) or file.io (bin) | sh | [ffsend](https://github.com/timvisee/ffsend), curl, jq, tr |
| [wallpaper](wallpaper) | Set wallpaper or change colorscheme | sh | nitrogen/pywal |
| [x2sel](x2sel) | Copy file list from system clipboard to selection | sh | _see in-file docs_ |
| [xdgdefault](xdgdefault) | Set the default app for the hovered file type | sh | xdg-utils, fzf/dmenu |

Notes:

1. A plugin has to explicitly request `nnn` to clear the selection e.g. after operating on the selected files.
2. Files starting with a dot in the `plugins` directory are internal files and should not be used as plugins.

### Table of contents

- [Installation](#installation)
- [Configuration](#configuration)
  - [Skip directory refresh after running a plugin](#skip-directory-refresh-after-running-a-plugin--)
- [Running commands as plugin](#running-commands-as-plugin-)
  - [Skip user confirmation after command execution](#skip-user-confirmation-after-command-execution-)
  - [Run a GUI app as plugin](#run-a-gui-app-as-plugin-)
  - [Page non-interactive command output](#page-non-interactive-command-output-)
  - [Some useful key-command examples](#some-useful-key-command-examples)
- [Access level of plugins](#access-level-of-plugins)
- [Create your own plugins](#create-your-own-plugins)
  - [Send data to `nnn`](#send-data-to-nnn)
  - [Get notified on file hover](#get-notified-on-file-hover)
- [Examples](#examples)
- [Contributing plugins](#contributing-plugins)

## Installation

The following command installs or updates (after backup) all plugins:

```sh
sh -c "$(curl -Ls https://raw.githubusercontent.com/jarun/nnn/master/plugins/getplugs)"
```

Plugins are installed to `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`.

## Configuration

Set environment variable `NNN_PLUG` to assign keybinds and invoke plugins directly using the plugin shortcut (<kbd>;</kbd>) followed by the assigned key character. E.g., with the below config:

```sh
export NNN_PLUG='f:finder;o:fzopen;p:mocq;d:diffs;t:nmount;v:imgview'
```

plugin `finder` can be invoked with the keybind <kbd>;f</kbd>, `fzopen` can be run with <kbd>;o</kbd> and so on... The key vs. plugin pairs are shown in the help and config screen.

Alternatively, combine with <kbd>Alt</kbd> (i.e. <kbd>Alt+key</kbd>).

To pick and run an unassigned plugin, press <kbd>Enter</kbd> (to _enter_ the plugin dir) at the plugin prompt.

To run a plugin at startup, use the option `-P` followed by the plugin key.

If the plugins list gets too long, try breaking them up into sections:

```
NNN_PLUG_PERSONAL='g:personal/convert2zoom;p:personal/echo'
NNN_PLUG_WORK='j:work/prettyjson;d:work/foobar'
NNN_PLUG_INLINE='e:!go run "$nnn"*'
NNN_PLUG_DEFAULT='1:ipinfo;p:preview-tui;o:fzz;b:nbak'
NNN_PLUG="$NNN_PLUG_PERSONAL;$NNN_PLUG_WORK;$NNN_PLUG_DEFAULT;$NNN_PLUG_INLINE"
export NNN_PLUG
```

Note:
- `'g:personal/convert2zoom'` will look in the personal sub-folder inside the plugin folder.
- `'b:boom;b:bulknew` will result in only the first definition of *b* (`b:boom`) being used.
- A keybinding definition of more than 1 character will prevent nnn from starting.


#### Skip directory refresh after running a plugin [`-`]

`nnn` refreshes the directory after running a plugin to reflect any changes by the plugin. To disable this add a `-` before the plugin name:

```sh
export NNN_PLUG='p:-plugin'
```

## Running commands as plugin [`!`]

To assign keys to arbitrary non-background cli commands and invoke like plugins, add `!` before the command.

```sh
export NNN_PLUG='x:!chmod +x "$nnn";g:!git log;s:!smplayer "$nnn"'
```

Now <kbd>;x</kbd> can be used to make a file executable, <kbd>;g</kbd> can be used to the git log of a git project directory, <kbd>;s</kbd> can be used to preview a partially downloaded media file.

#### Skip user confirmation after command execution [`*`]

`nnn` waits for user confirmation (the prompt `Press Enter to continue`) after it executes a command as plugin (unlike plugins which can add a `read` to wait). To skip this, add a `*` after the command.

```sh
export NNN_PLUG='s:!smplayer "$nnn"*;n:-!vim /home/vaio/Dropbox/Public/synced_note*'
```

Now there will be no prompt after <kbd>;s</kbd> and <kbd>;n</kbd>.

Note: Do not use `*` with programs that run and exit e.g. cat.

#### Run a GUI app as plugin [`&`]

To run a GUI app as plugin, add a `&` after `!`.

```sh
export NNN_PLUG='m:-!&mousepad "$nnn"'
```

#### Page non-interactive command output [`|`]

To show the output of run-and-exit commands which do not need user input, add `|` (pipe) after `!`.

```sh
export NNN_PLUG='m:-!|mediainfo "$nnn";t:-!|tree -ps;l:-!|ls -lah --group-directories-first'
```

This option is incompatible with `&` (terminal output is masked for GUI programs) and ignores `*` (output is already paged for user).

Notes:
1. Place `$nnn` (or exported variables) in double quotes (**`"$nnn"`**)
2. Use single quotes for `$NNN_PLUG` so `"$nnn"` is not interpreted
3. (_Again_) add `!` before the command
4. To disable directory refresh after running a _command as plugin_, prefix with `-!`

#### Some useful key-command examples

| Key:Command | Description |
|---|---|
| `c:!convert "$nnn" png:- \| xclip -sel clipboard -t image/png*` | Copy image to clipboard |
| `C:!cp -rv "$nnn" "$nnn".cp` | Create a copy of the hovered file |
| `e:-!sudo -E vim "$nnn"*` | Edit file as root in vim |
| `g:-!git diff` | Show git diff |
| `h:-!hx "$nnn"*` | Open hovered file in [hx](https://github.com/krpors/hx) hex editor |
| `k:-!fuser -kiv "$nnn"*` | Interactively kill process(es) using hovered file |
| `l:-!git log` | Show git log |
| `n:-!vi /home/user/Dropbox/dir/note*` | Take quick notes in a synced file/dir of notes |
| `p:-!less -iR "$nnn"*` | Page through hovered file in less |
| `s:-!&smplayer -minigui "$nnn"` | Play hovered media file, even unfinished download |
| `x:!chmod +x "$nnn"` | Make the hovered file executable |
| `y:-!sync*` | Flush cached writes |

## Access level of plugins

When `nnn` executes a plugin, it does the following:
- Changes to the directory where the plugin is to be run (`$PWD` pointing to the active directory)
- Passes three arguments to the script:
    1. `$1`: The hovered file's name.
    2. `$2`: The working directory (might differ from `$PWD` in case of symlinked paths; non-canonical).
    3. `$3`: The picker mode output file (`-` for stdout) if `nnn` is executed as a file picker.
- Sets the environment variable `NNN_PIPE` used to control `nnn` active directory.
- Sets the environment variable `NNN_INCLUDE_HIDDEN` to `1` if hidden files are active, `0` otherwise.
- Sets the environment variable `NNN_PREFER_SELECTION` to `1` if user prefers to use selection (see nnn's `-u` flag), `0` otherwise.
- Exports the [special variables](https://github.com/jarun/nnn/wiki/Concepts#special-variables).

Plugins can also read the `.selection` file in the config directory.

## Create your own plugins

Plugins can be written in any scripting language. However, POSIX-compliant shell scripts runnable in `sh` are preferred.

**Make the file executable**. Drop it in the plugin directory. Optionally add a hotkey in `$NNN_PLUG` for frequent usage.

#### Send data to `nnn`
`nnn` provides a mechanism for plugins to send data to `nnn` to control its active directory or invoke the list mode.
The way to do so is by writing to the pipe pointed by the environment variable `NNN_PIPE`.
The plugin should write a single string in the format `(<->)<ctxcode><opcode><data>` without a newline at the end. For example, `1c/etc`.

The optional `-` at the **beginning of the stream** instructs `nnn` to clear the selection.
In cases where the data transfer to `nnn` has to happen while the selection file is being read (e.g. in a loop), the plugin should
create a tmp copy of the selection file, inform `nnn` to clear the selection and then do the subsequent processing with the tmp file.
A paged [`|`] or GUI [`&`] cmd run as plugin cannot clear selection.

The `ctxcode` indicates the context to change the active directory of.

| Context code | Meaning |
|:---:| --- |
| `+` | smart context (next inactive else current) |
| `0` | current context |
| `1`-`4` | context number |

The `opcode` indicates the operation type.

| Opcode | Operation |
|:---:| --- |
| `c` | change directory |
| `l` | list files in list mode |
| `p` | picker file overwritten |

For convenience, we provided a helper script named `.nnn-plugin-helper` and a function named `nnn_cd` to ease this process. `nnn_cd` receives the path to change to as the first argument, and the context as an optional second argument.
If a context is not provided, it is asked for explicitly. To skip this and choose the current context, set the `CUR_CTX` variable in `.nnn-plugin-helper` (or in the specific plugin after sourcing `.nnn-plugin-helper`) to 1.
Usage examples can be found in the Examples section below.

#### Get notified on file hover

If `NNN_FIFO` is set, `nnn` will open it and write every hovered files. This can be used in plugins and external scripts, e.g. to implement file previews.

Don't forget to fork in the background to avoid blocking `nnn`.

For more details on configuration and usage of the preview plugins, visit [Live Previews](https://github.com/jarun/nnn/wiki/Live-previews).

## Examples

There are many plugins provided by `nnn` which can be used as examples. Here are a few simple selected examples.

#### Show the git log of changes to the particular file along with the code for a quick and easy review.

```sh
#!/usr/bin/env sh

git log -p -- "$1"
```

#### Change to directory in clipboard using helper script

```sh
#!/usr/bin/env sh

. $(dirname $0)/.nnn-plugin-helper

nnn_cd "$(xsel -ob)"
```

#### Change directory to the location of a link using helper script with specific context (current)

```sh
#!/usr/bin/env sh

. $(dirname $0)/.nnn-plugin-helper

nnn_cd "$(dirname $(readlink -fn $1))" 0
```

#### Change to arbitrary directory without helper script

```sh
#!/usr/bin/env sh

printf "cd to: "
read -r dir

printf "%s" "0c$dir" > "$NNN_PIPE"
```

#### Send every hovered file to X selection

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

#### Quick `find` the first match in subtree and open in `nuke`

```sh
#!/usr/bin/env sh

NUKE="${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins/nuke"

printf "file name: "
read -r pattern

entry=$(find . -type f -iname "$pattern" -print -quit 2>/dev/null)

if [ -n "$entry" ]; then
    "$NUKE" "$entry"
fi
```

#### Quick find (using `fd`)

```sh
#!/usr/bin/env sh

. "$(dirname "$0")"/.nnn-plugin-helper

printf "pattern: "
read -r pattern

if [ -n "$pattern" ]; then
    printf "%s" "+l" > "$NNN_PIPE"
    eval "fd -HI $pattern -0" > "$NNN_PIPE"
fi
```

#### Quick grep (using `rg`)

```sh
#!/usr/bin/env sh

. "$(dirname "$0")"/.nnn-plugin-helper

printf "pattern: "
read -r pattern

if [ -n "$pattern" ]; then
    printf "%s" "+l" > "$NNN_PIPE"
    eval "rg -l0 --hidden -S $pattern" > "$NNN_PIPE"
fi
```

## Contributing plugins

1. Add informative sections like _Description_, _Notes_, _Dependencies_, _Shell_, _Author_ etc. in the plugin.
2. Add an entry in the table above. Note that the list is alphabetically ordered by plugin name.
3. Keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.
4. The plugin file should be executable.
5. If your plugin stores data, use `${XDG_CACHE_HOME:-$HOME/.cache}/nnn`. Document it _in-file_.
