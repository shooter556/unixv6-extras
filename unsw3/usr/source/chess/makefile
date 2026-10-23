c:	agen.o att.o bgen.o bheur.o bmove.o book.o bplay.o ctrl.o data.o init.o io.o mater.o play.o qsort.o savres.o setup.o stat.o stdin.o wgen.o wheur.o wmove.o wplay.o
	cc -n -o c agen.o att.o bgen.o bheur.o bmove.o book.o bplay.o ctrl.o data.o init.o io.o mater.o play.o qsort.o savres.o setup.o stat.o stdin.o wgen.o wheur.o wmove.o wplay.o

pio.op:	agen.o att.o bgen.o bheur.o bmove.o book.o bplay.o ctrl.o data.o init.o io.o mater.o play.o qsort.o savres.o setup.o stat.o stdin.o wgen.o wheur.o wmove.o wplay.o
	cc -n -o p agen.o att.o bgen.o bheur.o bmove.o book.o bplay.o ctrl.o data.o init.o pio.o mater.o play.o qsort.o savres.o setup.o stat.o stdin.o wgen.o wheur.o wmove.o wplay.o
agen.o:
	cc -c -O agen.c
att.o:
	-as -o att.o att.s
bgen.o:
	-as -o bgen.o bgen.s
bheur.o:
	cc -c -O bheur.c
bmove.o:
	-as -o bmove.o bmove.s
book.o:
	cc -c -O book.c
bplay.o:
	cc -c -O bplay.c
ctrl.o:
	-as -o ctrl.o ctrl.s
data.o:
	cc -c data.c
init.o:
	cc -c -O init.c
io.o:
	cc -c -O io.c
mater.o:
	cc -c -O mater.c
play.o:
	cc -c -O play.c
qsort.o:
	-as -o qsort.o qsort.s
savres.o:
	cc -c -O savres.c
setup.o:
	cc -c -O setup.c
stat.o:
	cc -c -O stat.c
stdin.o:
	cc -c -O stdin.c
wgen.o:
	-as -o wgen.o wgen.s
wheur.o:
	cc -c -O wheur.c
wmove.o:
	-as -o wmove.o wmove.s
wplay.o:
	cc -c -O wplay.c
