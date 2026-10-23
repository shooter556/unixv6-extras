/ tell  - sends its args to cyber
/
/ does not rely on arguments being stored in any particular fashion
/
.globl	_main
indir=	0.
sleep=	35.
_main:
	sys	signal; 2; 0	/ allow rubouts
	tst	(sp)		/ argc
	bne	0f		/ if not zero
	mov	$2,r0
	sys	write; m1; m1l	/ no args
	sys	exit
0:
	sys	open; cmdsu; 1	/ commands up
	bec	0f		/ okay
	mov	$2,r0
	sys	write; m5; m5l	/ line busy - retrying
	mov	$5,r0
	sys	sleep		/ busy, so try again
	br	0b		/ in five seconds
0:
/ retain only the last component of the path name
/
	mov	2(sp),r1	/ argv[0]
0:
	tstb	(r1)		/ end of string ?
	beq	0f		/ yes
	cmpb	(r1)+,$'/	/ well, was it '/' ?
	bne	0b		/ no, so next char
	mov	r1,2(sp)	/ update arg ptr
	br	0b		/ and take next component
0:
/ pack args into buf
/
	mov	$buf,r2		/ where args get shoved
	mov	sp,r5		/ save arg ptr
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
	movb	$',,-1(r2)	/ replace null with comma
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
0:
	sys	exit		/ all done
.data
m1:	<no args\n>
m1l=	.-m1
m3:	<message too long\n>
m3l=	.-m3
m4:	<previous command not yet sent\n>
m4l=	.-m4
m5:	<line busy - retrying\n>
m5l=	.-m5
cmdsu:	</dev/cyb.cmdsu\0>
.even
mess:	sys	write; buf; l: 0
.bss
buf:	.=.+23.
