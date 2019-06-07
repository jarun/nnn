| Plugin (a-z) | Lang | Deps | Description |
| --- | --- | --- | --- |
| boom | sh | SMPlayer | Play random music from dir (modify `PLAYER`) |
| fzy-edit | sh | fzy | Fuzzy find a file in directory subtree and edit in vim |
| fzy-open | sh | fzy | Fuzzy find a file in directory subtree and open using xdg-open |
| getplugs | sh | wget | Update plugins |
| hexview | sh | xxd | View a file in hex in `$PAGER` |
| imgresize | sh | [imgp](https://github.com/jarun/imgp) | Resize images in directory to screen resolution |
| imgur | bash | - | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) |
| ipinfo | sh | curl, whois | Fetch external IP address and whois information |
| kdeconnect | sh | kdeconnect-cli | Send selected files to an Android device |
| mocplay | sh | [moc](http://moc.daper.net/) | Appends (and plays, see script) selection/dir/file in moc|
| ndiff | bash | vimdiff | Diff for selection (limited to 2 for directories) |
| nmount | sh | pmount, udisks2 | Toggle mount status of a device as normal user |
| nwal | sh | nitrogen | Set the selected image as wallpaper using nitrogen |
| pastebin | sh | [pastebinit](https://launchpad.net/pastebinit) | Paste contents of (text) file to paste.ubuntu.com |
| pdfview | sh | pdftotext/mupdf-tools | View PDF file in `$PAGER` |
| picker | sh | nnn | Pick files and pipe the newline-separated list to another utility |
| pywal | sh | pywal | Set selected image as wallpaper, change terminal color scheme |
| splitjoin | bash | split, cat | Split file or join selection |
| sxiv | sh | sxiv | Browse images in a dir in sxiv, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts))|
| transfer | sh | curl | Upload file to transfer.sh |
| upgrade | sh | wget | Upgrade to latest nnn version manually on Debian 9 Stretch |
| viuimg | sh | [viu](https://github.com/atanunq/viu), less | View an image or images in a directory |

#### Installing plugins

Download the `getplugs` plugin and execute it anywhere to get all the plugins installed to `~/.config/nnn/plugins`. You can run it again later to update the plugins. It backs up earlier plugins.

**NOTE:** `getplugs` also downloads the launcher `nlaunch` and tries to place it at `/usr/local/bin/` using `sudo`. If it fails you have to place `nlauch` manually somewhere in your `$PATH`.

#### File access from plugins

Plugins can access:
- all files in the directory (`nnn` switches to the dir where the plugin is to be run so the dir is `$PWD` for the plugin)
- the current file under the cursor (the file name is passed as the argument to a plugin)
- the current selection (by reading the file `~/.config/nnn/.selection`, see the plugin `ndiff`)

Each script has a _Description_ section which provides more details on what the script does, if applicable.

#### Usage

Use the _pick plugin_ shortcut to visit the plugin directory and execute a plugin. Repeating the same shortcut cancels the operation and puts you back in the original directory.

#### Contributing plugins

Plugins are scripts and all scripting languages should work. However, POSIX-compliant shell scripts runnable in `sh` are preferred. If that's too rudimentary for your use case, use Python, Perl or Ruby. Please keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.

The plugins should be executable. Please add an entry in the table above.
