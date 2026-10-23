qentlen	=	32.
qnum	=	100.
/
sleep	=	35.
sync	=	36.
vvv	=	54.
ppp	=	53.
/
.text
begin:
/
/	ignore hangup,quit,interrupt
/
	sys	signal; 1; 1
	sys	signal; 2; 1
	sys	signal; 3; 1
	jmp	start
/
.data
.globl	_errno;	_errno:	0
.globl	_main
start:	setd
	mov	sp,r0
	mov	(r0),-(sp)
	tst	(r0)+
	mov	r0,2(sp)
	jsr	pc,_main
	sys	exit
/
.text
/
/####	the area which follows is to be check pointed
/
.globl	_chkptp		; _chkptp=	.
.globl	_filen		; _filen:	0	/ should not be < 0
.globl	_njobs		; _njobs:	0	/ jobs in q's
.globl	_ncybjobs	; _ncybjobs:	0	/ cyber jobs
.globl	_basetime	; _basetime:	.=.+4
.globl	_queue		; _queue:	.=. + [qentlen * qnum]
.globl	_queuend	; _queuend=	. - qentlen
.globl	_qhdr		; _qhdr:	_queue; qnum; .=. + 28.
/
/ *** end of checkpoint area
/
qhdrend=	.-2

chkptl=		.-_chkptp
.globl	_chkptl		; _chkptl:	chkptl

.globl	_procs		; _procs:
/
/####	initialise the proc array
/
.globl	_statfd		;	f0;  0; .byte 1, 0; _statfd
.globl	_jupfd		;	f1;  0; .byte 1, 0; _jupfd
.globl	_jdnfd		;	f2;  0; .byte 0, 0; _jdnfd
.globl	_cdnfd		;	f3;  0; .byte 0, 0; _cdnfd
.globl	_crdfd		;	f4;  0; .byte 0, 0; _crdfd
				f5; d5; .byte 0, 4; 0
				f6; d6; .byte 0, 5; 0
				f7; d7; .byte 0, 6; 0
				f8; d8; .byte 0, 7; 0

				0	/*** terminator
f0:	<.print\0>
f1:	<.jtd\0>
f2:	<.spl\0>
f3:	<.ear\0>
f4:	<.crd\0>
f5:	<.mac\0>	;d5 = f5+1
f6:	<.ftn\0>	;d6 = f6+1
f7:	<.list\0>	;d7 = f7+1
f8:	<.shell\0>	;d8 = f8+1
/
.even
.globl	_bsw		; _bsw:		0	/ set when logged off
.globl	_qlock		; _qlock:	1	/ semaphore for queues
.globl	_actflag	; _actflag:	-1	/ >= zero -> batch active
.globl	_local		; _local:	0	/ set when called as 'local'
.globl	_nocyb		; _nocyb:	0	/ set to disable reading cyber decks
.globl	_nosub		; _nosub:	0	/ set to disable submitting
.globl	_norec		; _norec:	0	/ set to indicate "n" sent.
.globl	_noloc		; _noloc:	0	/ set to disable local jobs
.globl	_nomac		; _nomac:	0	/ set to flush macro jobs
.globl	_noftn		; _noftn:	0	/ set to flush fortran jobs
.globl	_noshl		; _noshl:	0	/ set to flush shell jobs
.globl	_nocrd		; _nocrd:	0	/ don't start crd
.globl	_noprt		; _noprt:	0	/ same for print
.globl	_scrdflag	; _scrdflag:	0	/ set to flush job on card reader
.globl	_reserved	; _reserved:	0; 0; 0; 0; 0; 0; 0; 0
.globl	_lprocs		; _lprocs:	0; 0; 0	/ pids of user local jobs
.globl	_cybmsg		; _cybmsg:	0; .=.+48.
.globl	_timelimit	; _timelimit:	120.
.globl	_cardlimit	; _cardlimit:	1000.
.globl	_recvlimit	; _recvlimit:	100.
.globl	_sbmtlimit	; _sbmtlimit:	1000.
.globl	_maxcjobs	; _maxcjobs:	75.
.globl	_procids	; _procids:	.=.+18.
.globl	_jupfd		; _jupfd:	.=.+2
.globl	_jdnfd		; _jdnfd:	.=.+2
.globl	_cdnfd		; _cdnfd:	.=.+2
.globl	_crdfd		; _crdfd:	.=.+2
.globl	_statfd		; _statfd:	.=.+2
.globl	_workdir	; _workdir:	</spool/batch/.work\0>
.globl	_fullckp	; _fullckp:	</spool/batch/.work/>
.globl	_chkpf		; _chkpf:	<.checkpoint\0>
.globl	_comtty		; _comtty:	<?>

.even
/
/    p*p
/
.text
.globl	_pp
/
_pp:				/ try for resource.
	mov	2(sp),r0
	sys	ppp
	bec	1f		/ if ok
	clr	r0		/ error
1:
	rts	pc
/
/    v*v
/
.text
.globl	_vv
/
_vv:	mov	2(sp),r0	/ free resource
	sys	vvv
	rts	pc
/
/    c*r*f*n*m
/
/    funk    1.    allocate next file sequence number.
/            2.    create the file.
/
/    args    none
/
/    retn    1.    file sequence number (r0).
/            2.    file descriptor (implied).
/
.text
.globl	_crfnm
/
_crfnm:	mov	$crlock,r0
	sys	ppp
	inc	_filen		/ allocate file seq no
	bgt	0f		/ -ve is special
	mov	$1,_filen	/ reset counter
0:	mov	_filen,r0	/ save
	mov	r0,-(sp)
	jsr	pc,_gfnm	/ get name
	sys	0; crfnm	/ create file
	bcc	1f
	mov	r0,_errno
	mov	$-1,(sp)
1:	mov	$crlock,r0
	sys	vvv
	mov	(sp)+,r0	/ file sequence number 
	rts	pc
crlock:	1
/
/    n*o*p*e*n
/
/    funk    1.    use file sequence number passed to perform
/                  an open.
/
/    args    1.    sequence #
/            2.    mode
/
/    retn    1.    file descriptor (r0)
/
.text
.globl	_nopen
/
_nopen:	mov	4(sp),opmd
	jsr	pc,gfnm
	sys	0; opfnm
	bcc	1f
	mov	r0,_errno
	mov	$-1,r0
1:	rts	pc
/
/    n*u*n*l*i*n*k
/
/    funk    1.    to delete the file
/
/    args    1.    file sequence number
/
/    retn
/
.text
.globl	_nunlink
/
_nunlink:
	tst	2(sp)
	beq	0f		/ if zero seq no  -  ignore
	jsr	pc,gfnm
	sys	0; ulfnm
	bcc	0f
	mov	$2,r0
	sys	write; _fnm; 7
	mov	$2,r0
	sys	write; nunm; nunl
0:	rts	pc
nunm:	<: unlink: error\n>
nunl = . - nunm
.even
/
/	p*r*i*n*t*s
/
.globl	_prints
_prints:
	mov	2(sp),r0	/ string address
	mov	r0,wstring+2	/ save its start.
0:	tstb	(r0)+		/ search for end
	bne	0b
	dec	r0
	sub	2(sp),r0	/ length of it
	beq	1f
	mov	r0,wstring+4	/ save its length
	mov	$2,r0
	sys	0; wstring
1:
	rts	pc
/
.data

wstring:sys	write;	..;	..
crfnm:	sys	creat;	_fnm;	600
opfnm:	sys	open;	_fnm;	opmd: .=.+2
ulfnm:	sys	unlink;	_fnm

.globl	_fnm, _ffnm
_ffnm:	</spool/batch/.work/>
_fnm:	<$??????\0>
.even
/
.text
.globl _chkpt; _chkpt:
/
	mov	$3,r0
	sys	seek; 0; 0		/ locate chkpt in file
	bcs	chkerror
	mov	$3,r0
	sys	write; _chkptp; chkptl	/ write the checkpoint record
	bcs	chkerror
	sys	sync
	rts     pc
chkerror:	
	mov	$2,r0
	sys	write; chkmes; chkmesl
	rts	pc
chkmes:	<checkpoint: seek/write error\n>
chkmesl = . - chkmes
.even
/
.globl _dtime; _dtime:
/
	sys	time
	sub	_basetime+2,r1
	sbc	r0
	sub	_basetime,r0
	div	$60.,r0		/ return the minutes
	cmp	r1,$30.
	blt	1f
	inc	r0
1:	rts     pc
/
/
/
/	_gfnm		to convert the file sequence number on stack
/			to a filename in 'fnm'
.globl _gfnm; _gfnm:
	jsr	pc,gfnm
	rts	pc
/
gfnm:
	mov	r2,-(sp)
	mov	$6,r2
	mov	6(sp),r1
	clrb	_fnm+1.(r2)
0:	clr	r0
	ashc	$3,r0
	movb	ctab(r0),_fnm(r2)
	sob	r2,0b
	mov	(sp)+,r2
	rts	pc
ctab:	<cimsagro>	/ o r g a s m i c
/
/	_comok		check to see if command is legal
/			from this terminal i.e. is command
/			terminal or user is super user.
/
.globl _comok, ttyn
_comok:	jsr	pc,ttyn		/ get terminal name
	cmpb	r0,_comtty	/ same terminal ?
	beq	1f		/ yes.
	sys	getuid		/ check super user.
	tstb	r0		/ real uid.
	beq	1f		/ is super user.
	clr	r0		/ error.
	rts	pc
1:
	mov	$1,r0		/ ok.
	rts	pc
