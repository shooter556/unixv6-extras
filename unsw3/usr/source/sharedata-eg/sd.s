.text
/ define entry sequence
/
	.globl	_main
begin:
	mov	sp,r0
	mov	(r0),-(sp)
	tst	(r0)+
	mov	r0,2(sp)
	jsr	pc,_main
	sys	exit
/ declare necessary globals
/
	.globl	_lock1,_lock2,_lock3,_lock4,_sdbuf

_lock1:	0
_lock2:	0
_lock3:	0
_lock4:	0

/ area used to pass data between processes
/
_sdbuf:	.=.+200.

/ cause printf,read,write to be included in common area
/
	.globl	_printf,_read,_write
