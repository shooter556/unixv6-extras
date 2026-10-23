/ C library -- connect

/  connect(master-fd, slave-fd, master-cntrl-char);
/
/  connect == 0 means success
/  connect == -1 means error

.globl	_connect, cerror

_connect:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	8(r5),0f+4
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

/
/ C library -- disconnect

/  disconnect( fd )	where fd is either master-fd or slave-fd
/
/  disonnect == 0 means success
/  disconnect == -1 means error

.globl	_disconnect, cerror

_disconnect:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	4(r5),0f+2	/ indicate disconnect
	mov	$-1,0f+4	/ indicate disconnect
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	59. ; 0:.. ; .. ; ..
