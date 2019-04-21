| Plugin (a-z) | Lang | Deps | Description |
| --- | --- | --- | --- |
| boom | sh | SMPlayer | Play random music from current dir (modify `PLAYER`) |
| fzy-edit | sh | fzy | Fuzzy find a file in directory subtree and edit in vim |
| fzy-open | sh | fzy | Fuzzy find a file in directory subtree and open using xdg-open |
| getplugs | sh | wget | Update plugins |
| hexview | sh | xxd, `$PAGER` | View a file in hex |
| imgresize | sh | [imgp](https://github.com/jarun/imgp) | Resize images in directory to screen resolution |
| imgur | bash | - | Upload an image to imgur (from [imgur-screenshot](https://github.com/jomo/imgur-screenshot)) |
| ipinfo | sh | curl, whois | Fetch IP address and whois information |
| kdeconnect | sh | kdeconnect-cli | Send selected files to an Android device |
| ndiff | sh | vimdiff | Show diff for selection |
| nmount | sh | pmount | Toggle mount status of a device as normal user |
| nwal | sh | nitrogen | Set the selected image as wallpaper using nitrogen |
| pastebin | sh | [pastebinit](https://launchpad.net/pastebinit) | Paste contents of current (text) file to paste.ubuntu.com |
| picker | sh | nnn | Pick files and pipe the newline-separated list to another utility |
| pywal | sh | pywal | Set selected image as wallpaper, change terminal color scheme |
| sxiv | sh | sxiv | Browse images in a dir in sxiv, set wallpaper, copy path ([config](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts))|
| transfer | sh | curl | Upload current file to transfer.sh |
| upgrade | sh | wget | Upgrade to latest nnn version manually on Debian 9 Stretch |

#### File access from plugins

Plugins can access:
- all files in the directory (`nnn` switches to the dir where the plugin is to be run so the dir is `$PWD` for the plugin)
- the currently highlighted file (the file name is passed as the argument to a plugin)
- the current selection (by reading the file .nnncp, see the plugin `ndiff`)

Each script has a _Description_ section which provides more details on what the script does, if applicable.

#### Contributing plugins

Plugins are scripts and all scripting languages should work. However, POSIX-compliant shell scripts runnable in `sh` are preferred. If that's too rudimentary for your use case, use Python, Perl or Ruby. Please keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.

The plugins should be executable. Please add an entry in the table above.
