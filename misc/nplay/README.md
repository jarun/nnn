## nplay

`nplay` (*NnnPLAY*) is a customizable file opener by file extension or mime type.

It is customizable and written for the CLI mode by default. To set GUI mode and use GUI apps by change the line

    GUI=0
to

    GUI=1

### Usage

    nplay filepath

### Integration with `nnn`

1. Export the required config:

       export NNN_OPENER=/path/to/nplay
        # Otherwise, if nplay is in $PATH
       export NNN_OPENER=nplay
2. Run `nnn` with the program option to indicate a CLI opener

       nnn -c
3. `nplay` can use `nnn` plugins (e.g. mocplay is used for audio), $PATH is updated to include `nnn` plugins dir.
