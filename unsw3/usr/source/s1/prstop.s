/ abort/restart/repeat line printer daemon

kill=	37.	/ (not in assembler)

	mov	2(sp),r1	/ save how called
	sys	open; lock; 0	/ pid of lpd kept in this file
	bec	0f		/ if opened ok
	mov	$2,r0
	sys	write; m1; m1l	/ "Lpd not active"
	sys	exit
0:
	sys	read; pid; 2	/ collect pid
	bes	1f		/ if error
	cmp	r0,$2		/ read 2 bytes ?
	beq	0f		/ yep. ok.
1:	mov	$2,r0
	sys	write; m2; m2l	/ "Read failure"
	sys	exit
0:
	mov	pid,r0
	cmpb	(r1),$'p	/ called as "prstop" ?
	beq	0f		/ yep. skip this bit.
	inc	signo		/ make into "restart" sig.
	cmpb	2(r1),$'s	/ restart not repeat
	beq	0f
	inc	signo		/ make into "repeat" sig.
	cmpb	2(r1),$'p	/ repeat not relinquish
	beq	0f
	inc	signo		/ make into "relinquish" sig.
0:	sys	indir; kil	/ send signal
	bec	0f		/ if sent ok
	mov	$2,r0
	sys	write; m3; m3l	/ "Unable to signal lpd"
	sys	exit
0:
	mov	$2,r0
	sys	write; m4; m4l	/ "Done"
	clr	r0
	sys	exit

.data

kil:	sys	kill; signo: 5	/ increment for restart

lock:	</tmp/lpd/lock\0>

m1:	<Lpd not active\n>
m1l=	.-m1
m2:	<Read failure\n>
m2l=	.-m2
m3:	<Unable to signal lpd\n>
m3l=	.-m3
m4:	<Done\n>
m4l=	.-m4

.bss

pid:	.=.+2
