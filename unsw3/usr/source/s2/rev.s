/	rev - reverse lines in a file
/

.globl	getc, putc, flush

	clr	inbuf
	mov	$1,outbuf
5:	clr	r4		/ no. characters on line

0:	jsr	r5,getc; inbuf	/ grab a char
	bes	3f		/ exit on eof
	cmpb	r0,$'\n		/ break on nl
	beq	1f
	mov	r0,-(sp)	/ stack char
	inc	r4		/ incr count
	br	0b		/ back for more

1:	tst	r4		/ end of line. test for empty line
	beq	2f		/ if empty
4:	mov	(sp)+,r0	/ pop next char
	jsr	r5,putc; outbuf	/ and print it
	sob	r4,4b		/ loop for more to print

2:	movb	$'\n,r0		/ terminate line
	jsr	r5,putc; outbuf
	br	5b		/ read next line

3:	jsr	r5,flush; outbuf	/ flush buffer
	sys	exit

.bss
inbuf:	.=.+518.
outbuf:	.=.+518.
