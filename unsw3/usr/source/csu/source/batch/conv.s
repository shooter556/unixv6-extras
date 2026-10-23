/ convert file sequence no. on stack to string
/ used by 'qlook'
/
.globl	_conv

_conv:	tst	2(sp)		/ sequence no
	beq	1f		/ if zero i.e. not applic.
	mov	r2,-(sp)	/ save r2
	mov	$6,r2
	mov	4(sp),r1	/ pick up sequence no.
0:
	clr	r0
	ashc	$3,r0
	movb	ctab(r0),fn(r2)
	sob	r2,0b
	mov	(sp)+,r2
	mov	$fn,r0
	rts	pc
1:
	mov	$na,r0	/ alternate string
	rts	pc
.data
ctab:	<cimsagro>
fn:	<$......\0>
na:	<       \0>
