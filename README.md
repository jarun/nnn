# noice

A fork of noice file browser: http://git.2f30.org/noice/

Additional customization to make it more friendly towards major distros (which `suck more` by some standards).

## Modifications

- File associations
    - Environment variable **NOICE_OPENER** to override all associations and open all files with your desktop environments default file opener. Examples:

            $ export NOICE_OPENER=xdg-open
            $ export NOICE_OPENER=gnome-open
            $ export NOICE_OPENER=gvfs-open
    All the following associations are ignored if **NOICE_OPENER** is exported.
    - Associate plain text files with vim (using `file` command)
    - Remove video file associations (to each his own favourite video player)
    - Associate common audio file types with lightweight fmedia (http://fmedia.firmdev.com/)
    - Associate PDF files with zathura
    - Removed less
    - Use environment variable `NOICE_FALLBACK_OPENER` to open other non-associated files
- Compilation
    - Use `-O3` for compilation, fixed warnings
    - Added compilation flag `-march=native` (compile only, no plans to package).
    - Remove generated config.h on `make clean`.

## Installation

    $ make
    $ sudo make install

## Help

    $ man noice

## Change associations

If you want to set custom applications for certain mime types, or change the ones set already (e.g. vim, fmedia, zathura), modify the `assocs` structure in **config.def.h** (it's easy). Then run the following commands to re-compile and install:

    $ make clean
    $ make
    $ sudo make install
