.data
/
/ this code is invoked via the q command its sole purpose being to 
/ display the queues.
/ if invoked via 'dq' then does it once and exits.
/ if invoked via 'autoq' then displays the queues every 30 seconds,
/		only if batch active.
/
sleep = 35.	/ sleep system call id.
qmax=11.
qtype=23.
qentlen=30.
/
/	r5    output message pointer
/	r4    loop counter
/	r3    qscan 
/
.globl _qhdr,_main,_njobs,_actflag,_queuend,_queue,_chkptl,_chkptp
.globl _local,_nocyb		/ daveh
/
_main:
	movb	*6(sp),auto		/ see how invoked
again:
	mov	ba,r5
	inc	_actflag
	bne	1f
	cmpb	$'a,auto		/ autoq ??
	jeq	exitt
	sys	open;fchk;0
	bcc	0f
2:	mov	$m3,ba
	mov	$m3l,bal
	jmp	wrt
0:	mov	_chkptl,5f
	sys	read;_chkptp;5: ..
	bcs	2b
1:	mov	$qmax,r4
	cmpb	$'a,auto		/ autoq ??
	bne	0f			/ --> no.
	dec	_actflag
	tst	(pc)+
9:	0
	bne	loop
	sys	signal; 1; 0
	sys	signal; 2; 0
	sys	signal; 3; 0
/
	mov	10(sp),r0		/ pointer to next arg(message)
	cmp	r0,$177777		/does it exist?
	beq	loop
	mov	r0,flen-2
1:	tstb	(r0)+
	bne	1b
	movb	$12,-1(r0)
	sub	10(sp),r0		/ length of string
	mov	r0,flen
	mov	10(sp),flen-2		/ address of string
	inc	9b
	br	loop
0:
	mov	$2,r0
	sys	close
	mov	$1,r0
	sys	dup			/ must give 2
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
	sob	r4,loop			/ loop till all queues examined
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
wrt:	mov	$2,r0
	sys	write;  ba: buffer ; bal: ..
/
/	daveh   print message if "local" and/or no cyber jobs allowed
/
	tst	_local
	beq	0f		/ if not "local"
	mov	$2,r0
	sys	write; loc; locl
0:
	tst	_nocyb
	beq	1f		/ if cyber allowed
	mov	$2,r0
	sys	write; nocyb; nocybl
1:
	tst	flen
	beq	2f
	mov	$2,r0
	sys	write; 0
flen:	0
2:

	cmpb	$'a,auto		/ autoq ??
	bne	exitt			/ --> no.
	mov	$2,r0
	sys	write;  m4;  m4l
	mov	$30.,r0			/ sleep for 30 seconds.
	sys	sleep
	jmp	again			/ go again.

exitt:
	dec	_actflag
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
fchk:	</spool/batch/.work/.checkpoint\0>
m3:	< dq: checkpoint file ??\n>
m3l = . - m3
m4:	<\t\t\t --- autoq ---\n>
m4l = . - m4
loc:	<*** local running ***\n>
locl = .-loc
nocyb:	<*** no cyber jobs allowed ***\n>
nocybl = .-nocyb
/
.even
qn:	a0;a1;a2 / ;a3;a4;a5
a0: <for cyber \n\0>
a1: <spool \n\0>
a2: <at cyber \n\0>
/ a3: <macro \n\0>
/ a4: <fortran \n\0>
/ a5: <list \n\0>
auto:	<?>
/
	.even
chkpf:	0
buffer:	. = . + 2000.
