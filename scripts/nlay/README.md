## nlay

`nlay` (*NnnpLAY*) is a customizable media type or action handler

### Usage

`nlay` is not used by `nnn` now. However, the bash script can be used to run desktop search utility or screensaver:

    nlay file type
    file: absolute path to file ("" for an action)
    type: type of media or action

### Default apps

* gnome-search-tool, catfish - file search
* vlock - terminal screensaver (alternatives - cmatrix, termsaver)

### Perks

- simple to modify (extensive in-file notes, comments and indicative code)
- handle files by category (e.g. plaintext, search, screensaver)
- support for multiple apps by order of preference
- run app in the foreground or silently detached in the background
- optionally add app arguments

### Tips to modify

- set `app=` in any category to change the player
- set `opts=` to use app options
- toggle `bg=` to enable or disable running app silently (`stdout` and `stderr` redirected to `/dev/null`) in background. E.g., vim (CLI) should be verbose and in the foreground while Sublime Text (GUI) can be started silently in the background.
- enable the commented out code under `ENABLE_FILE_TYPE_HANDLING` to handle specific file extensions e.g. use a different app than the one used for the category.
