/ C library -- time

/ tvec = time(tvec);
/ long tvec;
/

.globl	_time

_time:
	sys	time
	rts	pc
