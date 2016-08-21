# noice

A fork of noice file browser: http://git.2f30.org/noice/

Additional customization to make it more friendly towards major distros (which `suck more` by some standards).

## Modifications

- File associations
    - Associate plain text files with vim (using `file` command)
    - Remove video file associations (to each his own favourite video player)
    - Associate common audio file types with lightweight fmedia (http://fmedia.firmdev.com/)
    - Associate PDF files with zathura
    - Removed less
    - Use `xdg-open` to open other unrecognised files
- Compilation
    - Use `-O3` for compilation, fixed warnings
    - Added compilation flag `-march=native` (compile only, no plans to package).
    - Remove generated config.h on `make clean`.

## Installation

    $ make
    $ sudo make install

## Help

    $ man noice
