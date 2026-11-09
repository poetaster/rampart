let SessionLoad = 1
if &cp | set nocp | endif
let s:so_save = &g:so | let s:siso_save = &g:siso | setg so=0 siso=0 | setl so=-1 siso=-1
let v:this_session=expand("<sfile>:p")
silent only
silent tabonly
cd ~/src/arduino/rampart/firmware/BraidsEnginesRampart
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
let s:shortmess_save = &shortmess
if &shortmess =~ 'A'
  set shortmess=aoOA
else
  set shortmess=aoO
endif
badd +0 BraidsEnginesRampart.ino
badd +0 control.h
argglobal
%argdel
$argadd BraidsEnginesRampart.ino
set stal=2
tabnew +setlocal\ bufhidden=wipe
tabrewind
edit BraidsEnginesRampart.ino
argglobal
setlocal fdm=syntax
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
59
normal! zo
81
normal! zo
100
normal! zo
157
normal! zo
160
normal! zo
161
normal! zo
229
normal! zo
254
normal! zo
356
normal! zo
504
normal! zo
542
normal! zo
543
normal! zo
566
normal! zo
572
normal! zo
578
normal! zo
583
normal! zo
let s:l = 166 - ((6 * winheight(0) + 24) / 48)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 166
normal! 03|
lcd ~/src/arduino/rampart/firmware/BraidsEnginesRampart
tabnext
edit ~/src/arduino/rampart/firmware/BraidsEnginesRampart/control.h
argglobal
balt ~/src/arduino/rampart/firmware/BraidsEnginesRampart/BraidsEnginesRampart.ino
setlocal fdm=syntax
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
let s:l = 1 - ((0 * winheight(0) + 24) / 48)
if s:l < 1 | let s:l = 1 | endif
keepjumps exe s:l
normal! zt
keepjumps 1
normal! 0
lcd ~/src/arduino/rampart/firmware/BraidsEnginesRampart
tabnext 1
set stal=1
if exists('s:wipebuf') && len(win_findbuf(s:wipebuf)) == 0
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20
let &shortmess = s:shortmess_save
let s:sx = expand("<sfile>:p:r")."x.vim"
if filereadable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &g:so = s:so_save | let &g:siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
