qentlen = 30.
qnum	= 200.
/
getpid=20.
sleep=35.
sync=36.
vvv=54.
ppp=53.
/
.text
begin:
/
/	ignore hangup,quit,interrupt
/
	sys	signal;1;1
	sys	signal;2;1
	sys	signal;3;1
/
	sys	getpid
	mov	r0,_mypid	/ save the current process id
	jmp	start
/
.data
.globl _mypid; _mypid: 0
.globl _errno; _errno: 0
.globl _cbuf;  _cbuf: . = . + 32
/
.globl	_main
start:	setd	/ leave this in !!!
	mov	sp,r0
	mov	(r0),-(sp)
	tst	(r0)+
	mov	r0,2(sp)
	jsr	pc,_main
	sys	exit
/
.text
.globl _close	; _close
.globl _dup	; _dup
.globl _execl	; _execl
.globl _fork	; _fork
.globl _seek	; _seek
.globl _link	; _link
.globl _unlink	; _unlink
.globl _exit	; _exit
.globl _open	; _open
.globl _read	; _read
.globl _sleep	; _sleep
.globl _write	; _write
.globl _wait	; _wait
/
/
/####	the area which follows is to be check pointed
/
.globl _chkptp
_chkptp     =.
.globl _filen;	_filen:	1	/*** should not be zero
.globl _nspj;	_nspj:	qnum
.globl _njobs;	_njobs:	0.	/ # jobs not in queues
.globl _ncybjobs  ;    _ncybjobs: 125.   / max number of cyber jobs in system.
.globl _nblks;	_nblks:	2000.
.globl _basetime ;    _basetime:  .=.+4
.globl _queue    ;    _queue:  . = . + [qentlen*qnum]
.globl _queuend	;	_queuend   =   . - qentlen
.globl _qhdr     ;    _qhdr:   _queue;qnum ;	 . =  . + 52.
/
/ *** end of checkpoint area
/
qhdrend=	.-2
.globl _chkptl
chkptl=   .-_chkptp
_chkptl:   chkptl
.globl _procs    ;    _procs:
/
/####	initialise the proc array
/
.globl _statfd    ;           f0; 0; .byte 1, 0.;_statfd
.globl _jupfd    ;            f1; 0; .byte 1, 0.;_jupfd
.globl _jdnfd    ;            f2; 0; .byte 0, 0.;_jdnfd
.globl _cdnfd    ;            f3; 0; .byte 0,0.;_cdnfd
.globl _crdfd     ;           f4; 0; .byte 0, 0.;_crdfd
	                      f5;d5; .byte 0, 5.; 0.
	                      f6;d6; .byte 0, 4.; 0.
	                      f7;d7; .byte 0, 6.; 0.
	                      f8;d8; .byte 0, 7.; 0.
				f9;d9; .byte 0,8.; 0.
				f10;d10; .byte 0,9.; 0.
				f11;d11; .byte 0,10.; 0.
				f12;d12; .byte 0,11.; 0.
	                                                        0
f0:     <.print\0>
f1:     <.jtd\0>
f2:     <.spl\0>
f3:     <.ear\0>
f4:     <.crd\0>
f5:     <.mac\0>  ;d5 = f5+1
f6:     <.mid\0>  ;d6 = f6+1
f7:     <.ftn\0>  ;d7 = f7+1
f8:     <.ccc\0>  ;d8 = f8+1
f9:	<.pascal\0>	;d9 = f9+1
f10:	<.list\0>	;d10 = f10+1
f11:	<.file\0>	;d11 = f11+1
f12:	<.login\0>	;d12 = f12+1
/
/
/
.even
.globl _qlock;     _qlock:   1
.globl _listenf;     _listenf:   0
.globl _actflag;     _actflag:   -1
.globl _local;		_local:	0
.globl _nocyb;		_nocyb:	0
.globl _scrdflag;     _scrdflag:   0
.globl _monid;     _monid:   1
.globl _lpdid;     _lpdid:   1
.globl _procids;   _procids:   .=.+30.
.globl _jupfd;     _jupfd:   .=.+2
.globl _jdnfd;     _jdnfd:   .=.+2
.globl _cupfd;     _cupfd:   .=.+2
.globl _cdnfd;     _cdnfd:   .=.+2
.globl _crdfd;     _crdfd:   .=.+2
.globl _statfd;     _statfd:   .=.+2
/
/    p*p
/
.text
.globl _pp
/
_pp:				   / try for resource.
	mov	2(sp),r0
	sys	ppp
	bec	1f		/ if ok
	clr	r0		/ error
1:
     	rts     pc
/
/    v*v
/
.text
.globl _vv
/
_vv:   mov     2(sp),r0         / free resource.
	sys	vvv
       rts     pc
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
.globl _crfnm; _crfnm:
/
	mov	$crlock,r0
	sys	ppp
	inc     _filen                   / allocate file sequence number
	bgt	0f			 / -ve is special
	mov	$1,_filen		 / reset counter.
0:	mov     _filen,r0                / save
	mov	r0,-(sp)
	jsr     pc,_gfnm                 / obtain chars @r1 derived from stack
	sys     0;crfnm                  / create the file.
	jcc	1f
	mov	r0,_errno
	mov	$-1,(sp)
1:	mov	$crlock,r0
	sys	vvv
	mov	(sp)+,r0		/ file sequence number 
	rts     pc
crlock:	1
/
/    n*o*p*e*n
/
/    funk    1.    use file sequence number passed to perform
/                  an open.
/
/    args    1.    seqence #
/            2.    mode
/
/    retn    1.    file descriptor (r0)
/
.text
.globl _nopen; _nopen:
/
	mov     4(sp),opmd
	jsr     pc,gfnm
	sys     0;opfnm
	jcc	1f
	mov	r0,_errno
	mov	$-1,r0
1:	rts     pc
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
.globl _nunlink; _nunlink:
/
	jsr     pc,gfnm
	sys     0;ulfnm
	bcc	0f
	mov	$2,r0
	sys	write;nunm;nunl
0:	rts     pc
nunm:   <\nunlink: error\n>
nunl = . - nunm
.even
/
/	p*r*i*n*t*s
/
.globl _prints; _prints:
	mov	$prlock,r0
	sys	ppp		/ don't mangle messages
	mov	2(sp),r0	/ string address
	mov	r0,wstring+2	/ save its start.
0:	tstb	(r0)+		/ search for end
	bne	0b
	dec	r0
	sub	2(sp),r0	/ length of it
	beq	1f
	mov	r0,wstring+4	/ save its length
	mov	$2,r0
	sys	0;wstring
1:
	mov	$prlock,r0
	sys	vvv
	rts	pc
prlock:	1
/
.data
wstring: sys	write;..;..
crfnm: sys   creat;  _fnm;0666
opfnm: sys   open;   _fnm; opmd: .=.+2
ulfnm: sys   unlink; _fnm
.globl _fnm,_fnme,_ffnm
_ffnm:	</spool/batch/.work/>
_fnm:	<$??????>
_fnme:	<.???\0>
/
/    c*h*k*l
/
/    func    1.    check disk allocation to see if limit reached
/                  wait if so.
/
/    arg     1.    size to be written this time.
/
/    retn    1.    size to be written (r0).
/
.text
.globl _chkl; _chkl:
/
	add     2(sp),chktot
	movb    chktot+1,r1
	beq     2f
	clrb    chktot+1
0:	sub     r1,_nblks
	bgt     1f
	mov     $_nblks,r0
	mov	$15.,r0
	sys	sleep		/ wait for 15 seconds and try again!
	add     r1,_nblks
	br     0b
1:
2:     mov     2(sp),r0
.data
chktot:	0
.text
	rts     pc
/
/    f*r*e*e*l
/
/    func    1.    to increase disk allocation by amount passed
/
/    args    1.    size to be freed.
/
/
.text
.globl _freel; _freel:
/
	rts     pc
/
.globl _chkpt; _chkpt:
/
	mov	$3,r0
	sys	seek;0;0		/ locate chkpt in file
	jcs	chkerror
	mov	$3,r0
	sys	write;_chkptp;chkptl	/ write the checkpoint record
	jcs	chkerror
	sys	sync
	rts     pc
chkerror:	
	mov	$2,r0
	sys	write;chkmes;chkmesl
	rts	pc
chkmes:	<checkpoint: write error\n>
chkmesl = . - chkmes
.even
/
/
.globl _qinit; _qinit:
/
	mov	$_queue,r0
0:	mov	r0,r1
	add	$qentlen,r0
	mov	r0,(r1)+		/ set link to next field
	mov	$[qentlen-2\/2],r2	/ no. words left in qelt
1:
	clr	(r1)+			/ clear out any crap
	sob	r2,1b
	cmp	r0,$_queuend
	bne	0b
	clr	(r0)
/
/ clear the q headers
/
	mov	$_qhdr,r0
1:	mov	r0,r1
	add	$2,r0
	clr	(r1)
	cmp	r0,$qhdrend
	bne	1b
/
	mov	$_queue,_qhdr
	mov	$qnum,_qhdr+2
	rts	pc
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
/
/
