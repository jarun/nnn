<h1 align="center">User Patch Framework</h1>

This directory contains user submitted patches that were rejected from mainline as they tend to be more subjective in nature. The patches will be adapted on each release when necessary (v4.1 onwards). Each patch can be applied through its respective make variable during compilation.

## List of patches
| Patch (a-z) | Description | Make variable |
| --- | --- | --- |
| gitstatus | Add gitstatus column to the gitstatus view. Requires [libgit2](https://github.com/libgit2/libgit2). | O_GISTATUS |
| namefirst | Prints filenames first in the detail view. Prints user/group columns when a directory contains different users/groups. | O_NAMEFIRST |

To apply the patches, use the corresponding make variables, e.g.:

	make O_NAMEFIRST=1

