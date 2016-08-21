# noice

A fork of noice file browser: http://git.2f30.org/noice/

Added some customizations to make it more friendly towards major distros.

## Modifications

- File associations
    - Associate plain text files with vim (using `file` command)
    - Associate video files with mpv
    - Associate common audio file types with fmedia (http://fmedia.firmdev.com/)
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
