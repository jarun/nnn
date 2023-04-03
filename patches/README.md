<h1 align="center">User Patch Framework</h1>

This directory contains sizable user submitted patches that were rejected from mainline as they tend to be more subjective in nature.

The patches will be adapted on each release when necessary (v4.1 onwards). Each patch can be applied through its respective make variable during compilation. In case inter-patch merge conflicts occur, a compatibility patch is provided and will automatically be applied.

## List of patches

| Patch (a-z) | Description | Make var |
| --- | --- | --- |
| colemak | Key bindings for Colemak-DH keyboard layout | `O_COLEMAK` |
| gitstatus | Add git status column to the detail view. Provides command line flag `-G` to show column in normal mode. | `O_GITSTATUS` |
| namefirst | Print filenames first in the detail view. Print user/group columns when a directory contains different users/groups. | `O_NAMEFIRST` |
| restorepreview | Add pipe to close and restore [`preview-tui`](https://github.com/jarun/nnn/blob/master/plugins/preview-tui) for internal undetached edits (<kbd>e</kbd> key)| `O_RESTOREPREVIEW` |

To apply a patch, use the corresponding make variable, e.g.:

    make O_NAMEFIRST=1

When contributing/adding a new patch, make sure to add the make variable to the patches array in `./misc/test/check-patches.sh` as well so that patch failures can be easily tested.

## Resolving patch conflicts

Patch conflicts can be checked locally by running `make checkpatches` or by running `./patches/check-patches.sh` manually.

Whenever patch conflicts occur on the latest master, pull requests resolving them are welcome. Let's say a conflict occurs in the `restorepreview` patch. The best way to resolve this conflict would be something along the lines of:

- Ensure you're on latest master and run `PATCH_OPTS="--merge" make O_RESTOREPREVIEW=1`. This will generate the conflict markers in `src/nnn.c`.
- Next edit `src/nnn.c`, resolve the conflicts around the conflict markers(`<<<<<<<`), and save.
- Then run `git diff > patch.diff && sed -i -e "/^$/{r patch.diff" -e "q;}" patches/restorepreview/mainline.diff` to update the patch.
