	.globl getc,fopen,putc,flush
	mov	(sp)+,r4	/r4 is argc
	dec	r4
	ble	0f
	tst	(sp)+
	br	eof
0:	jsr	r5,getc;ibuf
	bes	eof
	bic	$177400,r0
	cmp	r0,$177
	bgt	1f
	jsr	r5,putc;obuf
	br	0b
1:	cmp	r0,$377
	bne	1f
	movb	$40,r1
	br	2f
1:	movb	$'0,r1
2:	jsr	r5,getc;ibuf
	bic	$177400,r0
	mov	r0,r2
1:	mov	r1,r0
	jsr	r5,putc;obuf
	sob	r2,1b
	br	0b
eof:
	tst	r4
	ble	0f
	dec	r4
	mov	ibuf,r0
	sys	close
	mov	(sp)+,r0
	jsr	r5,fopen;ibuf
	bcc	0b
	mov	$2,r0
	sys	write;filerr;filend-filerr
	br	eof
0:
	jsr	r5,flush;obuf
	sys	exit
	.bss
ibuf:	.=.+518.
	.data
obuf:	1
	.=.+516.
filerr:	<file not found\n>
filend:
