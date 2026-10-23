/ C library -- wait

/ pid = waitx(&status);
/ pid == -1 if error
/ status indicates fate of process, if given

.globl	_waitx, cerror

_waitx:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	jmp	cerror
1:
	mov	r1,*4(r5)	/ status return
2:
	mov	(sp)+,r5
	rts	pc
