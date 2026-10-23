.globl	_prints
indir = 0.

_prints:
	mov	4(sp),r0
1:	tstb	(r0)+
	bne	1b
	sub	4(sp),r0
	dec	r0
	mov	r0,0f+2
	mov	4(sp),0f
	mov	2(sp),r0
	sys	indir; 9f
	rts	pc
.data
9:	sys	write
0:	0;0
