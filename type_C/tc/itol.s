/ itol(hi, lo): build a long from two ints
/ long result is returned in r0 (high word) and r1 (low word)
/ needed by c1 (c11.c), absent from the V6 C library
.globl	_itol
_itol:
	mov	2(sp),r0
	mov	4(sp),r1
	rts	pc
