.data
/ tell  - sends its args to cyber
/
.globl	_main
.globl	_bsw, _actflag, _procids
indir=	0.
sleep=	35.
enodev=	19.
_main:
	sys	signal; 2; 0	/ allow rubouts
	tst	2(sp)		/ argc
	bne	0f		/ if not zero
	mov	$2,r0
	sys	write; m1; m1l	/ no args
	sys	exit
0:
	tst	_actflag		/ batch active ?
	beq	0f		/ yep
	mov	$2,r0
	sys	write; m2; m2l	/ batch not active
	sys	exit
0:
	sys	open; cmdsu; 1	/ commands up
	bec	0f		/ okay
	cmp	r0,$enodev	/ driver present ?
	bne	1f		/ nope. different message
	mov	$2,r0
	sys	write; m6; m6l	/ export driver not in system
	sys	exit
1:
	mov	$2,r0
	sys	write; m5; m5l	/ line busy - retrying
	mov	$5,r0
	sys	sleep		/ busy, so try again
	br	0b		/ in five seconds
0:
/ retain only the last component of the path name
/
	mov	6(sp),r1	/ argv[0]
0:
	tstb	(r1)		/ end of string ?
	beq	0f		/ yes
	cmpb	(r1)+,$'/	/ well, was it '/' ?
	bne	0b		/ no, so next char
	mov	r1,6(sp)	/ update arg ptr
	br	0b		/ and take next component
0:
/ pack args into buf
/
	mov	$buf,r2		/ where args get shoved
	mov	sp,r5		/ save arg ptr
	add	$4,r5
0:
	add	$2,r5		/ next arg
	cmp	(r5),$-1	/ end of list ?
	beq	2f		/ yas
	mov	(r5),r3		/ argv[i]
1:
	movb	(r3),(r2)+	/ next char moved
	cmp	r2,$buf+23.	/ run out of buffer ?
	beq	0f		/ yas
	tstb	(r3)+		/ terminator ?
	bne	1b		/ next char
	movb	$' ,-1(r2)	/ replace null with blank
	br	0b		/ next arg
0:
	mov	$2,r0
	sys	write; m3; m3l	/ message too long
	sys	exit
2:
	sub	$buf+1,r2	/ work out length
	mov	r2,l		/ length
	sys	indir; mess	/ write message
	bec	0f		/ ok
	mov	$2,r0
	sys	write; m4; m4l	/ last command not processed
	br	2f
0:
	cmpb	buf,$'b		/ bye command ?
	bne	1f		/ nope
	inc	_bsw		/ record that it is 'bye'
	br	2f
1:
	cmpb	buf,$'l		/ login ?
	bne	2f		/ nope
	clr	_bsw		/ reset switch
	mov	_procids+2,r0	/ jtd pid
	sys	kill;1		/ re-init
	mov	_procids+4,r0	/ spl pid
	sys	kill;1		/ re-init
2:
	sys	exit		/ all done
.data
m1:	<no args\n>
m1l=	.-m1
m2:	<batch not active\n>
m2l=	.-m2
m3:	<message too long\n>
m3l=	.-m3
m4:	<previous command not yet sent\n>
m4l=	.-m4
m5:	<line busy - retrying\n>
m5l=	.-m5
m6:	<\n\n\n*** export driver not in system ***\n\n>
m6l=	.-m6
cmdsu:	</dev/cyb.cmdsu\0>
.even
mess:	sys	write; buf; l: 0
.bss
buf:	.=.+23.
