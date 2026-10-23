mtcma = 172526
halt = 0
reset = 5
core = 24.

.. = [core*2048.] - 512.

start:	reset
	mov	$.., sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	clr	r0
	cmp	(r0),$407
	bne	1f
	add	$20,r0
1:	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)
2:	clr	r0
1:	clr	(r0)+
	cmp	r0,sp
	blo	1b
	clr	r0
3:	mov	$mtcma,r1
	mov	r0,(r1)
	mov	$-512.,-(r1)
	mov	$60003,-(r1)
2:	tstb	(r1)
	bpl	2b
	tst	(r1)
	bmi	1f
	add	$512.,r0
	br	3b
1:	bit	$40000,-(r1)
	beq	1f

	clr	r0
	cmp	(r0),$407
	bne	4f
5:	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	5b
4:	jsr	pc,*$0
	mov	$70,r2
	jmp	*$165562

1:	bit	$10000,12(r1)
	bne	1b
	cmp	(r1)+,(r1)+
	mov	$-1,(r1)
	mov	$60013,-(r1)

1:	tstb	(r1)
	bpl	1b
	tst	(r1)
	bpl	3b
	halt

end:
