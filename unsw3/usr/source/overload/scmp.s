.globl	_scmp
.text

_scmp:	mov	r2,-(sp)
	mov	4(sp),r0
	mov	6(sp),r1
	mov	$8.,r2
1:	cmpb	(r0)+,(r1)+
	blt	2f
	bgt	3f
	tstb	-1(r1)
	beq	4f
	sob	r2,1b
4:	clr	r0
5:	mov	(sp)+,r2
	rts	pc
2:	mov	$-1,r0
	br	5b
3:	mov	$1,r0
	br	5b

.globl	_xw,_xt,_xs
.globl	_find

_find:	mov	r2,-(sp)
	mov	r3,-(sp)	/ save some registers
	clr	_xw		/ null tree indicator
	mov	6(sp),_xt	/ segment pointer
	add	$42.,_xt		/ to &s_st
	mov	*_xt,r0		/ to p (alias q)
	beq	1f		/ null tree

	mov	r0,_xs		/ pivot point
4:	mov	10(sp),r1	/ name pointer
	mov	r0,r2		/ name of entry
	mov	$8.,r3		/ max length of symbol name
2:	cmpb	(r1)+,(r2)+	/ string comparison
	bhi	2f		/ move to right
	blo	3f		/ move to left
	tstb	-1(r1)		/ quick null terminated?
	beq	1f		/ yes.
	sob	r3,2b		/ for each byte
1:	mov	(sp)+,r3	/ restore registers
	mov	(sp)+,r2
	rts	pc
3:	tst	(r0)+		/ lson after rson
2:	add	$12.,r0		/ to child pointer
	mov	r0,_xw		/ save for insertion
	mov	(r0),r0		/ get child
	beq	1b		/ none - return
	bitb	$060,9.(r0)	/ check balance
	beq	4b		/ o.k.
	mov	_xw,_xt
	mov	r0,_xs		/ jimmy pointers
	br	4b
