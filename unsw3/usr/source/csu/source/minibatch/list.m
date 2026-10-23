if $1x = x goto all
if $1 = all goto all
:
: loop
:
if $1x = x exit
list -w136 $1.c
shift
goto loop
:
: all
:
list -w136 *.c
