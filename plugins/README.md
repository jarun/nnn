To extend the capabilities of `nnn`, plugins are introduced. Plugins are scripts which `nnn` can communicate with and trigger. This mechanism fits perfectly with the fundamental design to keep the core file manager lean and fast, by delegating repetitive (but not necessarily file manager-specific) tasks to the plugins.

The currently available plugins are listed below.

| Plugin (a-z) | Lang | Deps | Description |
| --- | --- | --- | --- |
| boom | sh | [moc](http://moc.daper.net/) | Play random music from dir |
| dups | sh | find, md5sum,<br>sort uniq xargs | List non-empty duplicate files in current dir |
| checksum | sh | md5sum,<br>sha256sum | Create and verify checksums |
| fzy-open | sh | fzy, xdg-open | Fuzzy find a file in dir subtree and edit or xdg-open |
| getplugs | sh | wget | Update plugins |
| hexview | sh | xxd | View a file in hex in `$PAGER` |
| imgresize | sh | [imgp](https://github.com/jarun/imgp) | Resize images in dir to screen resolution |
| imgur | bash | - | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) |
| ipinfo | sh | curl, whois | Fetch external IP address and whois information |
| kdeconnect | sh | kdeconnect-cli | Send selected files to an Android device |
| mediainf | sh | mediainfo | Show media information |
| moclyrics | sh | [ddgr](https://github.com/jarun/ddgr), [moc](http://moc.daper.net/) | Show lyrics of the track playing in moc |
| mocplay | sh | [moc](http://moc.daper.net/) | Appends (and plays, see script) selection/dir/file in moc|
| ndiff | sh | vimdiff | Diff for selection (limited to 2 for directories) |
| nmount | sh | pmount, udisks2 | Toggle mount status of a device as normal user |
| nwal | sh | nitrogen | Set image as wallpaper using nitrogen |
| oldbigfile | sh | find, sort | List large files by access time |
| organize | sh | file | Auto-organize files in directories by file type |
| pastebin | sh | [pastebinit](https://launchpad.net/pastebinit) | Paste contents of (text) file to paste.ubuntu.com |
| pdfview | sh | pdftotext/<br>mupdf-tools | View PDF file in `$PAGER` |
| picker | sh | nnn | Pick files and list one per line (to pipe) |
| pywal | sh | pywal | Set image as wallpaper, change terminal colorscheme |
| readit | sh | pdftotext, mpv,<br>pico2wave | Read a PDF or text file aloud |
| ringtone | sh | date, ffmpeg | Create a variable bitrate mp3 ringtone from file |
| splitjoin | sh | split, cat | Split file or join selection |
| sxiv | sh | sxiv | View images in dir, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts))|
| thumb | sh | [lsix](https://github.com/hackerb9/lsix) | View thumbnail of an image or dir of images |
| transfer | sh | curl | Upload file to transfer.sh |
| upgrade | sh | wget | Upgrade nnn manually on Debian 9 Stretch |
| vidthumb | sh | [ffmpegthumbnailer](https://github.com/dirkvdb/ffmpegthumbnailer),<br>[lsix](https://github.com/hackerb9/lsix) | Show video thumbnails in terminal |
| viuimg | sh | [viu](https://github.com/atanunq/viu), less | View an image or images in dir |

## Installing plugins

Download the `getplugs` plugin and execute it anywhere to get all the plugins installed to `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`. You can run it again later to update the plugins. It backs up earlier plugins.

**NOTE:** `getplugs` also downloads the launcher `nlaunch` and tries to place it at `/usr/local/bin/` using `sudo`. If it fails you have to place `nlauch` manually somewhere in your `$PATH`.

## File access from plugins

Plugins can access:
- all files in the directory (`nnn` switches to the dir where the plugin is to be run so the dir is `$PWD` for the plugin)
- the current file under the cursor (the file name is passed as the argument to a plugin)
- the current selection (by reading the file `.selection` in config dir, see the plugin `ndiff`)

Each script has a _Description_ section which provides more details on what the script does, if applicable.

## Usage

There are 2 ways to run plugins:

1. Directly with <kbd>x-key</kbd>:

       export NNN_PLUG='o:fzy-open;p:mocplay;d:ndiff;m:nmount;t:thumb'

   With this, plugin `fzy-open` can be run with the keybind <kbd>xo</kbd>, `mocplay` can be run with <kbd>xp</kbd> and so on... The key vs. plugin pairs are shown in the help and config screen. Up to 10 plugins can have such keybinds.

2. Use the _pick plugin_ shortcut to visit the plugin directory and execute a plugin. Repeating the same shortcut cancels the operation and puts you back in the original directory.

## Create your own plugins

Plugins are scripts and all scripting languages should work. However, POSIX-compliant shell scripts runnable in `sh` are preferred. If that's too rudimentary for your use case, use Python, Perl or Ruby.

You can create your own plugins by putting them in `${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins`.

For example, you could create a executable shell script `git-changes`:

    #!/usr/bin/env sh
    git log -p -- "$@"

And then trigger it by hitting the pick plugin key and selecting `git-changes` which will conveniently show the git log of changes to the particular file along with the code for a quick and easy review.

## Contributing plugins

Please keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.

The plugins should be executable. Please add an entry in the table above.
