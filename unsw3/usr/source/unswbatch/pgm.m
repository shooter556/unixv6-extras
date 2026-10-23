echo " "
:
: loop
if $1x = x exit
echo "------------------------- " $1
if $1 = sdump goto sdump
if $1 = macsup goto macsup
if $1 = dq    goto asmdq
if $1 = ja    goto java
if $1 = tell	goto asmtell
if $1 = qlook goto qlook
cc -O -c $1.c ; ld -r -x $1.o ; lked 1 text.o -x a.out -lc -la
if $1 = batch goto move
:
: transfer the a.out. removing the text. this saves space.
if '(' $1 = stop ')' -o '(' $1 = cancel ')' -o '(' $1 = nocyber ')' goto tr2
if $1 = submit goto tr5
if $1 = purge goto move
transfer a.out /spool/batch/.work/.$1
chown root /spool/batch/.work/.$1 ; chmod 700 /spool/batch/.work/.$1
goto rm
:
: tr2
transfer a.out /spool/batch/$1
chown root /spool/batch/$1
chmod 4701 /spool/batch/$1
goto rm
:
: tr5
cp  a.out /bin/$1
chown root /bin/$1 ; chmod 4701 /bin/$1 
:
: sdump
rm -f /spool/batch/.work/.sdump
cc -O -s sdump.c;mv a.out /spool/batch/.work/.sdump
chown root /spool/batch/.work/.sdump
chmod 701 /spool/batch/.work/.sdump
sync;shift
goto loop
:
: macsup
rm -f /spool/batch/.work/m.obj
macro macsup.mac; mv macsup.obj /spool/batch/.work/m.obj
chmod 704 /spool/batch/.work/m.obj
sync;shift
goto loop
:
:
: java
cc -O -c ja.c; as jas.s 
cc -s a.out ja.o ; rm ja.o
mv a.out /spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1
goto cont
:
: asmdq
as dq.s ; ld -r -x a.out ; lked 1 text.o -x a.out -lc -la
rm -f /bin/dq /bin/autoq ; mv a.out /bin/dq ; ln /bin/dq /bin/autoq
chown root /bin/dq ; chmod 4701 /bin/dq
goto cont
:
: asmtell
as tell.s; ld -r -x a.out ; lked 1 text.o -x a.out -lc -la
transfer a.out /spool/batch/a
chown root /spool/batch/a ; chmod 4701 /spool/batch/a
echo "check that all cyber commands are linked to 'a'"
goto cont
:
: qlook
cc -O -s qlook.c conv.o; rm /spool/batch/qlook
cp a.out /spool/batch/qlook
chmod 4701 /spool/batch/qlook; chown root /spool/batch/qlook
:
: rm
rm $1.o a.out
:
: cont
sync
shift
goto loop
:
: move the a.out preserving the text segment
cp a.out /spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1
rm $1.o a.out
sync
shift
goto loop
