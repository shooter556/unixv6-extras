/ C library - allocate on stack

.globl	_spush
_spush:
	mov	(sp)+,r1
	inc	(sp)
	bic	$1,(sp)
	sub	(sp),sp
	mov	sp,r0
	tst	(r0)+
	jmp	(r1)
