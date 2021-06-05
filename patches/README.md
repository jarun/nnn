<h1 align="center">User Patch Framework</h1>

This directory contains sizable user submitted patches that were rejected from mainline as they tend to be more subjective in nature.

The patches will be adapted on each release when necessary (v4.1 onwards). Each patch can be applied through its respective make variable during compilation. In case inter-patch merge conflicts occur, a compatability patch is provided and will automatically be applied.

## List of patches
| Patch (a-z) | Description | Make var |
| --- | --- | --- |
| gitstatus | Add git status column to the detail view. Provides command line flag `-G`. Requires [libgit2](https://github.com/libgit2/libgit2). | `O_GISTATUS` |
| namefirst | Print filenames first in the detail view. Print user/group columns when a directory contains different users/groups. | `O_NAMEFIRST` |

To apply a patch, use the corresponding make variable, e.g.:

    make O_NAMEFIRST=1
