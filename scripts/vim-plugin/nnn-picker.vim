" vim/neovim plugin to use nnn as a file picker
" Closely follows and inspired by the vim_file_chooser plugin for ranger.
"
" Author: Arun Prakash Jana
" Email: engineerarun@gmail.com
" Homepage: https://github.com/jarun/nnn
" Copyright © 2018 Arun Prakash Jana
"
" Usage:
" Copy this file to the vim/neovim plugin directory.
" To open nnn as a file picker in vim/neovim, use the command ":NnnPicker" or
" ":Np" or the key-binding "<leader>n". Once you select one or more files and
" quit nnn, vim/neovim will open the first selected file and add the remaining
" files to the arg list/buffer list.  If no file is explicitly selected, the
" last selected entry is picked.

let s:temp = ""

fun! s:T_OnExit(job_id, code, event) dict
    if a:code == 0
        bd!
        call s:evaluate_temp()
    endif
endfun

fun! s:evaluate_temp()
    if !filereadable(s:temp)
        echoerr 'Temp file ' . s:temp . 'not readable!'
        redraw!
        " Nothing to read.
        return
    endif
    let names = readfile(s:temp)
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
endfun

function! NnnPicker()
    let s:temp = tempname()
    let l:cmd = 'nnn -p ' . shellescape(s:temp)
    
    if has("nvim")
      enew
      call termopen(l:cmd, {'on_exit': function('s:T_OnExit')}) | startinsert
    elseif has("gui_running")
        exec 'silent !xterm -e ' . l:cmd
        call s:evaluate_temp()
    else
        exec 'silent !' . l:cmd
        call s:evaluate_temp()
    endif
endfunction

command! -bar NnnPicker call NnnPicker()
nnoremap <leader>n :<C-U>NnnPicker<CR>
command! -nargs=* -complete=file Np :call NnnPicker()
