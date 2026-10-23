.data
/
/ this code is invoked via the q command its sole purpose being to 
/ display the queues.
/ if invoked via 'dq' then does it once and exits.
/ if invoked via 'autoq' then displays the queues every 30 seconds,
/ but only if batch active.
/
/ also prints out a simple summary of the status of the batch system:
/
/	batch/local,
/	cyber reading enabled/disabled,
/	submitting enable/disabled,
/	local enabled/disabled,
/	etc,
/	last broadcast message from cyber,
/	whether cyber is telling us 'too many jobs - try later'.
/
/ it also prints an argument, if specified.
/
/					ian j.	march '76
/					daveh.	march '78

sleep=		35.
indir=		0
qmax=		7.	/ qtype of last local daemon
qtype=		23.
qentlen=	32.
altq=		100
altd=		200
/
/	r5    output message pointer
/	r4    loop counter
/	r3    qscan 
/
.globl	_qhdr, _main, _njobs, _actflag, _queuend, _queue, _chkptl, _chkptp
.globl	_local, _nocyb, _norec, _tmjtl, _nosub, _noloc, _fullckp, _cybmsg
/
_main:
	movb	*6(sp),auto		/ see how invoked
again:
	mov	ba,r5
	tst	_actflag
	bge	1f		/ if batch active
	cmpb	$'a,auto		/ autoq ??
	jeq	exitt		/ exit if autoq & batch not active
	sys	open; _fullckp; 0
	bec	0f		/ opened ok
2:	mov	$m3,ba
	mov	$m3l,bal
	jmp	wrt		/ some error on open or read
0:	mov	_chkptl,5f
	sys	read; _chkptp; 5: ..
	bes	2b		/ error
1:	mov	$qmax,r4
	cmpb	$'a,auto		/ autoq ??
	bne	loop			/ --> no.
	tst	first		/ first time thru ??
	bne	loop		/ no. skip this bit.
	sys	signal; 1; 0
	sys	signal; 2; 0
	sys	signal; 3; 0
/
	mov	10(sp),r0		/ ptr to next arg
	cmp	r0,$-1.		/ does it exist ?
	beq	loop
	mov	r0,flen-2
1:	tstb	(r0)+
	bne	1b
	movb	$12,-1(r0)
	sub	10(sp),r0		/ length of string
	mov	r0,flen
	mov	10(sp),flen-2		/ address of string
	inc	first		/ been this way
loop:	ash	$2,r4			/ shift loop counter for index
	mov	_qhdr(r4),r3		/ first job elt this queue.
	beq	2f			/ queue empty.
	mov	$m1,r0			/ header id /
0:	movb	(r0)+,(r5)+
	bne	0b
	dec	r5
	asr	r4
	mov	qn-2(r4),r0
	asl	r4
0:	movb	(r0)+,(r5)+
	bne	0b
	dec	r5
	mov	$5,r2
1:	mov	r3,r0
	tst	(r0)+
	mov	$7,r1
0:	movb	(r0)+,(r5)+
	sob	r1,0b
	mov	$7,r1
	tstb	(r0)
	beq	5f
	movb	$'/,(r5)+
	movb	(r0)+,(r5)+
	movb	(r0)+,(r5)+
	movb	(r0)+,(r5)+
	mov	$3,r1
	bitb	$altq|altd,10.(r0)	/ special flags ?
	beq	5f			/ nope.
	bitb	$altq,10.(r0)		/ alternate queue ?
	beq	4f			/ no, must be alt dest.
	movb	$'+,(r5)+		/ flag altq
	br	3f
4:	movb	$'-,(r5)+		/ flag altd
3:	dec	r1
5:	movb	$' ,(r5)+
	sob	r1,5b
	dec	r2			/ check for full line
	bne	0f
	mov	$5,r2
	movb	$'\n,(r5)+
0:	mov	0(r3),r3		/ step to next elt
	bne	1b
	cmp	r2,$7
	beq	2f
	movb	$'\n,(r5)+
2:	ash	$-2,r4			/ reduce after above shift
	dec	r4
	jgt	loop		/ loop for all queues (can't use "sob")
/
	tst	_njobs			/ any jobs in system  ??
	beq	fin			/ branch if no.
	mov	$m0,r1
0:	movb	(r1)+,(r5)+
	bne	0b
	dec	r5
	movb	$'\n,(r5)+
	mov	$_queue,r0
1:	cmp	$1,(r0)
	bne	2f
	mov	$5f,r1; tstb qtype(r0)   ; beq 0f
	mov	$6f,r1; cmpb qtype(r0),$1; beq 0f
	mov	$7f,r1; cmpb qtype(r0),$3; beq 0f
	mov	$8f,r1; cmpb qtype(r0),$2; beq 0f
	mov	$9f,r1;
0:	movb	(r1)+,(r5)+
	bne	0b
	dec	r5
	mov	$7,r2
	mov	r0,r1
	tst	(r1)+
0:	movb	(r1)+,(r5)+
	sob	r2,0b
	movb	$'\n,(r5)+
2:	add	$qentlen,r0
	cmp	r0,$_queuend
	ble	1b
fin:
/
	sub	ba,r5
	mov	r5,bal
	bne	wrt
	mov	$m2,ba
	mov	$m2l,bal
wrt:	mov	$1,r0
	sys	write;  ba: buffer ; bal: ..
/
/ various status tests ...
/
	tst	_local
	beq	0f		/ if not "local"
	mov	$1,r0
	sys	write; loc; locl
0:
	tst	_nocyb
	beq	0f		/ if cyber allowed
	mov	$1,r0
	sys	write; nocyb; nocybl
0:
	tst	_nosub
	beq	0f		/ if submit allowed
	mov	$1,r0
	sys	write; nosub; nosubl
0:
	tst	_noloc
	beq	0f		/ if local allowed
	mov	$1,r0
	sys	write; noloc; nolocl
0:
	tst	_norec
	beq	0f		/ if accepting output.
	mov	$1,r0
	sys	write; norec; norecl
0:
/ more tests go here
/ make these the last in the list so they are easy to spot
/
	tst	_tmjtl		/ see if cyber keeps telling us this
	beq	0f		/ nope
	clr	_tmjtl		/ clear it (ear will set it again)
	mov	$1,r0
	sys	write; tmjm; tmjml
0:
/	test for any broadcast message
/
	tstb	_cybmsg
	beq	0f		/ no message
	mov	$_cybmsg,r0
	mov	r0,r1
1:
	tstb	(r0)+		/ end of message ?
	bne	1b		/ nope.
	dec	r0		/ adjust.
	sub	r1,r0		/ find message length.
	mov	r0,wstr+4	/ length.
	mov	$1,r0
	sys	indir; wstr
	clrb	(r1)		/ reset (ear will set again)
0:
	tst	_actflag
	bge	0f		/ if batch active
	mov	$1,r0
	sys	write; bna; bnal
0:
	tst	flen
	beq	2f
	mov	$1,r0
	sys	write; 0
flen:	0
2:
	cmpb	$'a,auto		/ autoq ??
	bne	exitt			/ --> no.
	mov	$1,r0
	sys	write;  m4;  m4l
	mov	$30.,r0			/ sleep for 30 seconds.
	sys	sleep
	jmp	again			/ go again.

exitt:
	sys	exit
/
5:	<reading   \0>
6:	<sending   \0>
7:	<receiving \0>
8:	<spooling  \0>
9:	<executing \0>

m0:	<active jobs >

m1:	< --- \0>

m2:	< --- queues empty\n>
m2l = . - m2

m3:	<dq: checkpoint file ??\n>
m3l = . - m3

m4:	<\t\t\t --- AUTOQ ---\n>
m4l = . - m4

loc:	<*** local running\n>
locl = .-loc

nocyb:	<*** no cyber jobs allowed\n>
nocybl = .-nocyb

nosub:	<*** no submitting allowed\n>
nosubl = . - nosub

noloc:	<*** no local jobs\n>
nolocl = . - noloc

norec:	<*** output disabled by "n" command\n>
norecl = . - norec

tmjm:	<\n\t\t*** too many jobs at cyber - try later\n>
tmjml=	.-tmjm

bna:	<\n** batch not active\n>
bnal=	.-bna

.even

qn:	0f;1f;2f;3f;4f;5f;6f

0: <for cyber\n\0>
1: <print\n\0>
2: <at cyber\n\0>
3: <macro\n\0>
4: <fortran\n\0>
5: <list\n\0>
6: <shell\n\0>

auto:	. = . + 1

.even

wstr:	sys	write; _cybmsg; ..

chkpf:	0
first:	0
buffer:	. = . + 3000.	/ ought to be enough for whole display
