/	Non-balanced tree version of find and add

.globl	_find,_add,_getcore

.data
ff:	0
.text

_find:	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	add	$42.,r0
4:	mov	r0,ff
	mov	(r0),r0
	beq	1f
	mov	10(sp),r1
	mov	r0,r2
	mov	$8.,r3
2:	cmpb	(r1)+,(r2)+
	bhi	2f
	blo	3f
	tstb	-1(r1)
	beq	1f
	sob	r3,2b
1:	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
3:	tst	(r0)+
2:	add	$12.,r0
	br	4b

_add:	mov	$16.,-(sp)
	jsr	pc,_getcore
	mov	r0,*ff
	mov	r0,(sp)
	mov	4(sp),r1
	mov	(r1)+,(r0)+
	mov	(r1)+,(r0)+
	mov	(r1)+,(r0)+
	mov	(r1),(r0)
	mov	(sp)+,r0
	rts	pc
