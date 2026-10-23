echo " "
:
: loop
if $1x = x exit
echo "------------------------- " $1
if $1 = sdump goto sdump
if $1 = macsup goto macsup
if $1 = dq    goto asmdq
if $1 = ja    goto java
if $1 = print goto ctim
if $1 = tell	goto tell
if $1 = qlook goto qlook
if $1 = count goto count
if $1 = purge goto purge
if $1 = logon goto logon
if $1 = enter goto enter
if $1 = enable goto enable
if $1 = status goto status
cc -O -c $1.c ; lked 1 text.o $1.o -lc -la
goto xyzz
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
macro -ls macsup.mac; mv macsup.obj /spool/batch/.work/m.obj
chmod 704 /spool/batch/.work/m.obj
sync;shift
goto loop
:
: ctim
cc -O -c $1.c ; lked 1 text.o $1.o -lc -la
goto xyzz
:
: java
cc -O -c ja.c; as jas.s 
cc -s a.out ja.o
goto tr3
:
: asmdq
as dq.s ; lked 1 text.o a.out -lc -la
rm -f /bin/dq /bin/autoq ; mv a.out /bin/dq ; ln /bin/dq /bin/autoq
chown root /bin/dq ; chmod 4701 /bin/dq
goto cont
:
: tell
cc -c -O tell.c; lked 1 text.o tell.o -lc -la
goto tr4
:
: qlook
as -o conv.o conv.s
cc -O -s qlook.c conv.o; rm /spool/batch/qlook
cp a.out /spool/batch/qlook
chmod 4701 /spool/batch/qlook; chown root /spool/batch/qlook
goto rm
:
: logon
cc -O logon.c; cp a.out /spool/batch/.work/.logon
chmod 700 /spool/batch/.work/.logon; chown root /spool/batch/.work/.logon
rm a.out
goto cont
:
: count
cc -O -s count.c; rm /spool/batch/count
mv a.out /spool/batch/count
chmod 4701 /spool/batch/count; chown root /spool/batch/count
goto cont
:
: purge
cc -c -O purge.c; lked 1 text.o purge.o -lc -la
rm /spool/batch/purge; cp a.out /spool/batch/purge
chmod 4701 /spool/batch/purge; chown root /spool/batch/purge
goto rm
:
: enter
cc -c -O enter.c; lked 1 text.o enter.o -lc -la
rm /spool/batch/entl /spool/batch/encl /spool/batch/enjl /spool/batch/enrl /spool/batch/ensl
cp a.out /spool/batch/entl
ln /spool/batch/entl /spool/batch/encl
ln /spool/batch/entl /spool/batch/enjl
ln /spool/batch/entl /spool/batch/enrl
ln /spool/batch/entl /spool/batch/ensl
chmod 4701 /spool/batch/entl
goto rm
:
: status
as -o conv.o conv.s; cc -c -O status.c
lked 1 text.o status.o conv.o -lc -la
cp a.out /spool/batch/status; chmod 4701 /spool/batch/status
rm conv.o
goto rm
:
: enable
cc -c -O enable.c; lked 1 text.o enable.o -lc -la
rm /spool/batch/enable /spool/batch/disable
cp a.out /spool/batch/enable
ln /spool/batch/enable /spool/batch/disable
chmod 4701 /spool/batch/enable
goto rm
:
: xyzz
if $1 = batch goto move
:
: transfer the a.out. removing the text. this saves space.
if $1 = stop   goto tr3
if $1 = cancel goto cancel
if $1 = submit goto tr5
transfer <a.out >/spool/batch/.work/.$1
chown root /spool/batch/.work/.$1 ; chmod 700 /spool/batch/.work/.$1
goto rm
:
: cancel
cp a.out /spool/batch/cancel; chmod 4701 /spool/batch/cancel
goto rm
:
: tr2
transfer <a.out >/spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1 
goto rm
:
: tr3
cp a.out /spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1
goto rm
:
: tr4
cp a.out /spool/batch/a
chown root /spool/batch/a ; chmod 4701 /spool/batch/a
rm /spool/batch/b /spool/batch/c /spool/batch/d /spool/batch/e \
   /spool/batch/f /spool/batch/g /spool/batch/i /spool/batch/l \
   /spool/batch/m /spool/batch/n /spool/batch/q /spool/batch/r \
   /spool/batch/s /spool/batch/u /spool/batch/o /spool/batch/p
ln /spool/batch/a /spool/batch/b ; ln /spool/batch/a /spool/batch/c
ln /spool/batch/a /spool/batch/d ; ln /spool/batch/a /spool/batch/e
ln /spool/batch/a /spool/batch/f ; ln /spool/batch/a /spool/batch/g
ln /spool/batch/a /spool/batch/i ; ln /spool/batch/a /spool/batch/l
ln /spool/batch/a /spool/batch/m ; ln /spool/batch/a /spool/batch/n
ln /spool/batch/a /spool/batch/o ; ln /spool/batch/a /spool/batch/p
ln /spool/batch/a /spool/batch/q ; ln /spool/batch/a /spool/batch/r
ln /spool/batch/a /spool/batch/s ; ln /spool/batch/a /spool/batch/u
goto rm
:
: tr5
cp  a.out /bin/$1
chown root /bin/$1 ; chmod 4701 /bin/$1 
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
mv a.out /spool/batch/batch
chown root /spool/batch/batch ; chmod 4701 /spool/batch/batch
rm /spool/batch/local; ln /spool/batch/batch /spool/batch/local
rm $1.o 
sync
shift
goto loop
