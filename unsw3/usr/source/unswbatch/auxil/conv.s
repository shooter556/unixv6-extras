/ convert file seq no. on stack to string
/
.globl	_conv

_conv:
	mov	2(sp),r1
	beq	1f
	mov	r2,-(sp)
	mov	$6,r2
0:
	clr	r0
	ashc	$3,r0
	movb	ctab(r0),fn(r2)
	sob	r2,0b
	mov	(sp)+,r2
	mov	$fn,r0
	rts	pc
1:
	mov	$na,r0
	rts	pc
.data
ctab:	<cimsagro>	/o r g a s m i c
fn:	<$......\0>
na:	<.......\0>
