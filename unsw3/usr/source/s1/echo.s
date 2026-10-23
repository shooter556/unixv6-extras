	/echo
	/
	/prints its arguments
	/
	movb	$12,-(r1)		/put nl at end...
	mov	4(sp),sp
	mov	sp,nchar
	neg	nchar
	mov	sp,addr
	tstb	-(r1)			/second last char
	bne	0f
	movb	$12,*r1			/replace with nl
	dec	nchar			/decrement output count
	bne	0f
	inc	nchar			/no args at all
	/eliminate nulls
0:	tstb	*sp
	bne	1f
	movb	$' ,*sp
1:	inc	sp
	bmi	0b
	mov	$1,r0			/klutz !!!
	sys	write;addr: ..;nchar: ..
	sys	exit
