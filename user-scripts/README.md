| Script (a-z) | Lang | Deps | Description |
| --- | --- | --- | --- |
| copier | sh | OS-specific | Copy selection to clipboard |
| edit | sh | fzy | Fuzzy find a file in directory subtree and edit in vim |
| fzy | sh | fzy | Fuzzy find a file in directory subtree and open using xdg-open |
| hexview | sh | xxd, $PAGER | view a file in hex |
| imgresize | sh | [imgp](https://github.com/jarun/imgp) | Resize images in directory to screen resolution |
| imgur | bash | [imgur](https://github.com/jomo/imgur-screenshot) | Upload an image to imgur |
| kdeconnect | sh | kdeconnect-cli | Send selected files to an Android device |
| ndiff | sh | vimdiff | File and directory diff for selection |
| nlaunch | sh | fzy | Drop-down app launcher. Copy in `$PATH`; fallback regular prompt |
| nwal | sh | nitrogen | Set the selected image as wallpaper using nitrogen |
| paste | sh | [pastebinit](https://launchpad.net/pastebinit) | Paste contents of current (text) file to paste.ubuntu.com |
| picker | sh | nnn | Pick files and pipe the newline-separated list to another utility |
| pywal | sh | pywal | Set selected image as wallpaper, change terminal color scheme |
| sxiv | sh | sxiv | Open images in a dir in sxiv, set wallpaper, copy path ([tips](https://wiki.archlinux.org/index.php/Sxiv#Assigning_keyboard_shortcuts))|
| transfer | sh | curl | Upload current file to transfer.sh |
| upgrade | sh | wget | Upgrade to latest nnn version manually on Debian 9 Stretch |

### File access from scripts

The design is flexible so a script can access:
- all files in the directory (`nnn` switches to the dir where the script is to be run so the dir is `$PWD` for the script)
- the currently highlighted file (the file name is passed as the first argument to a script)
- the current selection (by reading the file .nnncp, see the script `copier`)

### Contributing scripts

All scripting languages should work. However, POSIX-compliant shell scripts runnable in `sh` are preferred. If that's too rudimentary for your use case, use Python, Perl or Ruby. Please keep non-portable commands (like `notify-send`) commented so users from any other OS/DE aren't surprised.

The scripts should be executable. Please add an entry in the table above.
