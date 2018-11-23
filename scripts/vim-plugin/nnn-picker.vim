" vim plugin to use nnn as a file picker
" Closely follows and inspired by the vim_file_chooser plugin for ranger.
"
" Author: Arun Prakash Jana
" Email: engineerarun@gmail.com
" Homepage: https://github.com/jarun/nnn
" Copyright © 2018 Arun Prakash Jana
"
" Usage:
" Copy this file to the vim plugin directory.
" To open nnn as a file picker in vim, use the command ":NnnPicker" or ":Np"
" or the key-binding "<leader>n". Once you select one or more files and quit
" nnn, vim will open the first selected file and add the remaining files to
" the arg list/buffer list.
" If no file is explicitly selected, the last selected entry is picked.

function! NnnPicker()
    let temp = tempname()
    if has("gui_running")
        exec 'silent !xterm -e nnn -p ' . shellescape(temp)
    else
        exec 'silent !nnn -p ' . shellescape(temp)
    endif
    if !filereadable(temp)
        redraw!
        " Nothing to read.
        return
    endif
    let names = readfile(temp)
    if empty(names)
        redraw!
        " Nothing to open.
        return
    endif
    " Edit the first item.
    exec 'edit ' . fnameescape(names[0])
    " Add any remaining items to the arg list/buffer list.
    for name in names[1:]
        exec 'argadd ' . fnameescape(name)
    endfor
    redraw!
endfunction
command! -bar NnnPicker call NnnPicker()
nnoremap <leader>n :<C-U>NnnPicker<CR>
command! -nargs=* -complete=file Np :call NnnPicker()
