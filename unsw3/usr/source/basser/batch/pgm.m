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
if $1 = purge goto purge
cc -O -c $1.c ; lked 1 text.o $1.o -lc -la
goto xyzz
:
: sdump
rm -f /spool/batch/.work/.sdump
cc -O sdump.c;mv a.out /spool/batch/.work/.sdump
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
cc a.out ja.o ; rm ja.o
goto tr3
:
: asmdq
as dq.s ; lked 1 text.o a.out -lc -la
rm -f /bin/dq /bin/autoq ; mv a.out /bin/dq ; ln /bin/dq /bin/autoq
chown root /bin/dq ; chmod 4701 /bin/dq
goto cont
:
: asmtell
as tell.s
goto tr4
:
: qlook
cc -O -s qlook.c conv.o; rm /spool/batch/qlook
mv a.out /spool/batch/qlook
chmod 4701 /spool/batch/qlook; chown root /spool/batch/qlook
goto cont
:
: purge
cc -c -O purge.c; lked 1 text.o purge.o -lc -la
rm /spool/batch/purge; cp a.out /spool/batch/purge
chmod 4701 /spool/batch/purge; chown root /spool/batch/purge
goto rm
:
: xyzz
if $1 = batch goto move
:
: transfer the a.out. removing the text. this saves space.
if $1 = stop   goto tr2
if $1 = cancel goto tr2
if $1 = submit goto tr5
if $1 = nocyber goto tr2
transfer <a.out >/spool/batch/.work/.$1
chown root /spool/batch/.work/.$1 ; chmod 700 /spool/batch/.work/.$1
goto rm
:
: tr2
if $1 = cancel rm /spool/batch/restart
if $1 = nocyber rm /spool/batch/oncyber
transfer <a.out >/spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1 
if $1 = cancel ln /spool/batch/cancel /spool/batch/restart
if $1 = nocyber ln /spool/batch/nocyber /spool/batch/oncyber
goto rm
:
: tr3
mv a.out /spool/batch/$1
chown root /spool/batch/$1 ; chmod 4701 /spool/batch/$1
goto cont
:
: tr4
mv a.out /spool/batch/a
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
goto cont
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
rm /usr/bin/batch; ln /spool/batch/batch /usr/bin/batch
rm $1.o 
sync
shift
goto loop
