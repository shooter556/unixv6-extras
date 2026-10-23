/ pef - process export file
/
/ Insert spaces and zeroes where required.
/ Strip off banner page.
/ Strip off dayfile (one day ...)
/
/ Write results to std output.

VT=	13	/ vertical tab
FF=	14	/ form feed
BLANK=	377	/ blank compress
ZERO=	376	/ zero compress
COMP=	376	/ just test for <= ...

.globl	getc, fopen, putc, flush

/ Initialization stuff.  Check arg count, open files etc.

	cmp	(sp),$2		/ arg count
	beq	1f		/ if argc == 2
	mov	$2,r0
	sys	write; mu; lmu	/ usage
	sys	exit
1:
	mov	4(sp),r0	/ file name
	jsr	r5,fopen; inbuf	/ open it
	bec	1f		/ if ok
	mov	$2,r0
	sys	write; mo; lmo	/ open fail
	sys	exit
1:
	mov	$1,outbuf	/ preset output buffer

/ States 1,2:	skip 2 vertical tabs

	mov	$2,r1		/ count
0:
	jsr	r5,getc; inbuf	/ get next char
	bes	9f		/ if eof or err
	cmp	r0,$COMP	/ compress ?
	blt	1f		/ no
	jsr	r5,getc; inbuf	/ get byte count
	bes	9f		/ eof or err
	br	0b		/ ignore that byte
1:
	cmp	r0,$VT		/ vertical tab ?
	bne	0b		/ no
	sob	r1,0b		/ another one

/ State 3:	look for form feed

0:	jsr	r5,getc; inbuf
	bes	9f
	cmp	r0,$COMP
	blt	1f
	jsr	r5,getc; inbuf
	bes	9f
	br	0b
1:
	cmp	r0,$FF		/ form feed ?
	bne	0b		/ no

/ State 4:	copy over chars expanding everything

0:	jsr	r5,getc; inbuf
	bes	9f
	cmp	r0,$COMP
	blt	2f
	mov	r0,r1		/ save escape code
	jsr	r5,getc; inbuf	/ collect count
	bes	9f
	mov	r0,r2		/ save count
	mov	$' ,r0		/ assume blank escape
	cmp	r1,$ZERO	/ and check
	bne	1f		/ we were right
	mov	$'0,r0		/ oh well
1:
	jsr	r5,putc; outbuf	/ write out one char
	sob	r2,1b		/ and keep it up
	br	0b		/ more
2:
	jsr	r5,putc; outbuf	/ write ordinary char
	br	0b
9:
	jsr	r5,flush; outbuf/ flush out buffer
	sys	exit
.data
mu:	<Usage: pef file\n>; lmu = .-mu
mo:	<Open failure\n>; lmo = .-mo
.bss
inbuf:	.=.+518.
outbuf:	.=.+518.
