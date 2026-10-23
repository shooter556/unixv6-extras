: 'list -w136 slup*.h slup*.c ^ lpr -i"update/archiver"'
list -w136 slup*.h slup*.c > slupout
lpr -i"update/archiver" -r @craig/slup/slupout
cref -cx slup*.c >crefout
lpr -i"Cref of slup" -r @craig/slup/crefout
