/
/	to do
/
/
/	virtual blocks
/
/	more than 32767 blocks on tape.
/
/	phserr recovery.
/
/	links
/
/
/	
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
/
//////////////////////////////////////////////////////////////////////////
//									//
//	this is a version of 'tp' modified by ian j. AGSM		//
//									//
//	enhancements or otherwise:					//
//									//
//	1) new command 's'. this is a selective dump facility.		//
//	   the number following 's' in the key is taken as a number	//
//	   of hours -- if number absent then /etc/last-dump-time	//
//	   contains time last dump  -- any file modified within/since	//
//	   that time will be dumped.					//
//									//
//	2)  new commands  implemented, new meanings, old		//
//	    unused commands are out. see 'swtab' below			//
//									//
//	3)  tape blocksize increased to 8192. bytes, this allows three	//
//	    times more data to be kept on tape than is possible using	//
//	    a blocksize of 512. bytes.					//
//									//
//////////////////////////////////////////////////////////////////////////


.globl	_localtime, _end

/	incore directory structure

namep = 0		/ patname address (in string area)
mode = 2		/ file access bits
uid = 4; gid = 5	/ the owner
size0 = 6		/ file size (in bytes) - high order
size1 = 8		/ file size (in bytes) - low order
tmajmin = size1		/ major/minor numbers for special files.
majmin = size1		/ if spec file the major/minor device no.
time0 = 10.		/ last modified time (secs since 1970)
time1 = 12.		/ last modified time (secs since 1970)
tapea = 14.		/ multiple by 512 to get byte address on tape of fiel
dirsiz = 16.

/	tape directory structure

tpathname = 0		/ file pathname (null terminated)
tmode = 48.		/ file access bits
tuid =50.; tgid = 51.	/ the owner
tsize = 52.		/ file size (bytes)
ttime = 56.		/ last modified time (secs since 1970)
ttapea = 60.		/ multiple by 512 to get byte address on tape of file
tchecksum = 62.
tdirsiz = 64.

mdirent = [128.*10.]		/ number of directory entries
				/ must be multiple of 128.
				/ since there are 128 entries per tape block

//////////////////////////////////////////////////////////////////////////

.text
	jmp	tapeb		/ go initialise - done in buffer to conserve core
.data

//////////////////////////////////////////////////////////////////////////

tapeb:
	sys	time
	mov	r0,startim
	mov	r1,startim+2		/ note program start time
	mov	(sp),rnarg
	mov	(sp)+,narg
	mov	$cmexit,command
	tst	(sp)+
	cmp	narg,$2
	bge	1f
	mov	$2,narg
	br	3f
1:
	mov	(sp)+,r0
	mov	sp,parg
1:
	movb	(r0)+,r1
	beq	3f
	mov	$swtab,r2
2:
	cmp	r1,(r2)+
	beq	2f
	tst	(r2)+
	bne	2b
	br	useerr
2:
	jsr	pc,*(r2)+
	br	1b
3:
	jsr	pc,optap
	mov	$_end,r4 / string pointer
	jsr	pc,setb
	jmp	*command

optap:
	mov	$mdirent,ndirent
	cmp	command,$cmn
	beq	2f
	sys	open; mt; 0
	br	3f
2:
	sys	open; mt; 1
3:
	bes	1f
	mov	r0,fio
	mov	ndirent,r1
	rts	pc
1:
	jsr	r5,mesg
		<Tape open error\n\0>; .even
	jmp	done

setcom:
	cmp	command,$cmexit
	bne	useerr
	mov	(r5)+,command
	rts	r5

noflag:
	mov	(r5)+,r0
	beq	1f
	tstb	(r0)
	beq	noflag
	br	useerr
1:
	rts	r5

useerr:
	jsr	r5,mesg
		<Bad usage\n\0>; .even
	jmp	done

swtab:
	'0; dcof	/ m a g t a p e   unit 0
	'1; dcof	/ m a g t a p e   unit 1
/
	'x; dcx		/ e x t r a c t  named files.
	'n; dcn		/ n e w  dump tape to be created
	'l; dcl		/ l i s t   contents of dump tape 
/
	's; dcs		/ i n c r e m e n t a l l y  dump files to dump tape
	'c; dcc		/ c r e a t e   as needed directories/special-files (full extract only)
	'u; dcu		/ u p d a t e   last dump time in /etc/last-dump-time (new dump only)
/
	'v; dcv		/ v e r b o s e   dont be silent, give all info
	'q; dcq		/ q u e s t i o n   after each file selected for user approval(y-yes n-no x-exit)
	'i; dci		/ i g n o r e  tape read/write errors
	 0; 0

dcof:
	movb	r1,mtx
	rts	pc

dci:
	incb	fli
	rts	pc

dcn:
	jsr	r5,setcom; cmn
	rts	pc

dcl:
	jsr	r5,setcom; cml
	rts	pc

dcu:
	incb 	flu			/ cause start time to be written to /etc/last-dump-time
	rts	pc

dcv:
	incb	flv
	rts	pc

dcq:
	incb	flq
	rts	pc

dcx:
	jsr	r5,setcom; cmx
	rts	pc
dcc:
	cmp	$2,narg
	bne	useerr			/ only allow with full extract.
	inc	flc
	rts	pc

dcs:
	incb 	fls			/ use selective file dump
	clr	r1
0:
	movb	(r0),r2			/ pick next char from 1st argument.
	sub	$'0,r2
	blt	1f			/ not a valid digit so branch
	cmp	$'9,r2
	blt	1f			/ not a valid digit so branch
	inc	r0
	mul	$10.,r1
	add	r2,r1			/ build no. of hours.
	br	0b
1:
	tst	r1
	bne	4f			/ if non-zero then use hour interval specified
	mov	r0,-(sp)
	sys	open; ldt; 0
	bec	3f
2:
	jsr	r5,mesg
		< last-dump-time unable to be opened\n\0>; .even
	jmp	done
3:
	mov	r0,r1
	sys	read; seltim; 4		/ dump all files modded after this time
	bes	2b
	cmp	$4,r0
	bne	2b
	mov	r1,r0
	sys	close
	mov	(sp)+,r0
	rts	pc
4:
	mov	r0,-(sp)
	mov	r1,r0
	mpy	$-3600.,r0		/ get seconds(-ve) not hours.
	add	startim+2,r1		/ subtract
	adc	r0			/  time
	add	startim,r0		/   specified.
	mov	r0,seltim
	mov	r1,seltim+2		/ all files modified after this time dumped
	mov	(sp)+,r0
	rts	pc

//////////////////////////////////////////////////////////////////////////

cmexit:
	jsr	r5,mesg
		<no function specified\n\0>; .even
	jmp 	done

//////////////////////////////////////////////////////////////////////////

	. = tapeb+8192.	/*******************************************
.text

//////////////////////////////////////////////////////////////////////////

cmn:
	jsr	r5,noflag; flc; 0
	jsr	pc,clrdir		/ clear incore directory structure
	jsr	pc,getfiles		/ determine those file to be dumped
	jsr	pc,make
	tst	flu
	beq	usage		/ no update of last-dump-time reqd.
	sys	creat; ldt; 0600
	bec	2f
1:
	jsr	r5,mesg
		<can't update last-dump-time\n\0>; .even
	jbr	done
2:
	mov	r0,r1
	sys	write; startim; 4
	bes	1b
	cmp	r0,$4
	bne	1b
	br	usage

//////////////////////////////////////////////////////////////////////////

cml:
	jsr	r5,noflag; flc; flq; 0
	jsr	pc,rddir		/ read tape directory into core and format it
	tstb	flv
	beq	1f
	jsr	r5,mesg
	<\n    mode    uid gid tapa    size   date    time name\n\n\0>; .even
1:
	jsr	r5,gettape; taboc
	jbr	usage

//////////////////////////////////////////////////////////////////////////

cmx:
	jsr	r5,noflag; 0
	tstb	flc
	beq	1f
	tstb	flq
	beq	1f
	jmp	useerr			/ don't allow directory create with questions
1:
	jsr	pc,rddir		/ read tape directory into core and format it
	jsr	pc,tread		/ get first tape block (tape posn now aggrees with tapad)
	jsr	r5,gettape; xtract
	br	done

//////////////////////////////////////////////////////////////////////////

usage:
	mov	$dir,r2
	clr	r0		/ count no. of dir entries here
1:
	tst	(r2)		/ end of the incore directory ??
	beq	2f		/ --> yes.
	inc	r0
	add	$dirsiz,r2
	cmp	r2,edir
	blo	1b
2:
	jsr	r5,numb; 4
	jsr	r5,mesg
		< files on tape\n\0>; .even
	mov	nused,r0
	jsr	r5,numb; 4
	jsr	r5,mesg
		< tape blocks\n\0>; .even

/////////////////////////////////////////////////////////////////////////

done:
	jsr	r5,mesg
		<END\n\0>; .even
	sys	exit

/////////////////////////////////////////////////////////////////////////

/	use:	copy addressed string into the string area

/	call:	jsr	r5,encode
/			source-string-address

encode:
	mov	r2,-(sp)
	mov	r4,(r1)
	mov	(r5)+,r2
1:
	movb	(r2),(r4)+
	jsr	pc,setb		/ check core allocation
	tstb	(r2)+
	bne	1b
	mov	(sp)+,r2
	rts	r5

/////////////////////////////////////////////////////////////////////////

/	use:	move pathname addressed from currently addressed(r1)
/		directory entry to destination address.

/	call:	jsr	r5,decode
/			destination-string-address

decode:
	mov	r2,-(sp)
	mov	r1,-(sp)
	mov	(r1),r1
	mov	(r5)+,r2
1:
	movb	(r1)+,(r2)+
	bne	1b
	mov	(sp)+,r1
	mov	(sp)+,r2
	rts	r5

/////////////////////////////////////////////////////////////////////////

/	use:	check for depletion of string area. 
/		grab more as necessary

setb:
	mov	r0,-(sp)
	mov	r4,r0		/ where upto in string area.
	add	$513.,r0
	cmp	r0,sp
	bhis	2f
	bic	$777,r0
	cmp	r0,0f
	beq	1f
	mov	r0,0f
	sys	0; 9f
.data
9:
	sys	break; 0:..	/ allocate more core
.text
	bes	2f
1:
	mov	(sp)+,r0
	rts	pc
2:
	jsr	r5,mesg
		<Out of core\n\0>; .even
	jbr	done


/////////////////////////////////////////////////////////////////////////

pstr:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putc
	br	pstr
1:
	rts	pc

/////////////////////////////////////////////////////////////////////////

mesg:
	movb	(r5)+,r0
	beq	1f
	jsr	pc,putc
	br	mesg
1:
	inc	r5
	bic	$1,r5
	rts	r5

/////////////////////////////////////////////////////////////////////////

putc:
	movb	r0,ch
	mov	$1,r0
	sys	write; ch; 1
	rts	pc

/////////////////////////////////////////////////////////////////////////

getc:
	clr	r0
	sys	read; ch; 1
	movb	ch,r0
	rts	pc

/////////////////////////////////////////////////////////////////////////

clrdir:
	mov	$dir,r1
	mov	ndirent,r2
1:
	jsr	pc,clrent
	sob	r2,1b
	rts	pc

clrent:
	mov	r1,-(sp)
	add	$dirsiz,(sp)
1:
	clr	(r1)+
	cmp	r1,(sp)
	blo	1b
	tst	(sp)+
	rts	pc

/////////////////////////////////////////////////////////////////////////

rddir:
	clr	sum
	jsr	pc,clrdir		/ clear incore directory structure
	mov	$dir,r1
	mov	ndirent,r2
1:
	bit	$177,r2
	bne	2f
	jsr	pc,tread	/ get next block of the tape
	mov	$tapeb,r3
2:
	mov	r1,-(sp)
	mov	r3,-(sp)
	mov	$[tdirsiz\/2],r0
	clr	-(sp)
2:
	add	(r3)+,(sp)		/ form checksum
	sob	r0,2b
	bis	(sp),sum		/ save checksum indication
	tst	(sp)+			/ checksum good ??
	bne	2f			/ --> no.
	mov	(sp),r3
	tst	(r3)
	beq	2f			/ --> null entry.
	mov	r3,0f
	jsr	pc,9f
.data
9:
	jsr	r5,encode; 0:..	/ get string out of tape entry in string area
	rts	pc
.text
	add	$tmode,r3		/ point beyond pathname in tape entry
	mov	r1,-(sp)
	add	$dirsiz,(sp)
	tst	(r1)+		/ step over pathname pointer in directory entry
9:
	mov	(r3)+,(r1)+	/ transfer file information
	cmp	r1,(sp)
	blo	9b
	tst	(sp)+
2:
	mov	(sp)+,r3
	add	$tdirsiz,r3		/ point to next tape entry
	mov	(sp),r1
	jsr	pc,sizeit		/ calculate tape size to date
	mov	(sp)+,r1
	add	$dirsiz,r1	/ point to next directory entry
	sob	r2,1b		/ go for more (possibly)
	tst	sum
	beq	1f
	jsr	r5,mesg
		<Directory checksum\n\0>; .even
	tstb	fli
	jeq	done		/ don't ignore the problem, stop
1:
	rts	pc

sizeit:
	mov	tapea(r1),r5		/ pick up tape address this file
	beq	1f
	bit	$60000,mode(r1)
	bne	0f			/ --> special file.
	mov	size0(r1),r0
	mov	size1(r1),r1
	add	$511.,r1
	adc	r0
	ashc	$-9.,r0			/ size of file in blocks.
	add	r1,r5
0:
	add	$15.,r5
	ash	$-4,r5			/ no. of tape blocks thus far used.
	mov	r5,nused		/ no. of blocks used on tape to date.
1:
	rts	pc

/////////////////////////////////////////////////////////////////////////

wrdir:
	mov	$dir,r1
	mov	ndirent,r2
1:
	bit	$177,r2
	bne	2f
	mov	$4096.,r0
	mov	$tapeb,r3
3:
	clr	(r3)+		/ create a nice clean fresh tape buffer
	sob	r0,3b
	mov	$tapeb,r3
2:
	mov	r3,-(sp)
	tst	(r1)
	beq	2f		/ --> no file here, go gen tape entry(no name)
	mov	r3,0f
	jsr	pc,9f
.data
9:
	jsr	r5,decode; 0:..	/ transfer pathname out of string area
	rts	pc
.text
2:
	add	$tmode,r3	/ point to file info part of tape
	mov	r1,-(sp)
	add	$dirsiz,(sp)
	tst	(r1)+
9:
	mov	(r1)+,(r3)+	/ transfer file information to tape entry
	cmp	r1,(sp)
	blo	9b
	tst	(sp)+
	mov	(sp)+,r3
	clr	-(sp)
	mov	$31.,r0
2:
	sub	(r3)+,(sp)	/ generate checksum
	sob	r0,2b
	mov	(sp)+,(r3)+	/ store it in tape entry
	dec	r2
	bit	$177,r2
	bne	1b
	jsr	pc,twrite	/ output tape block
	tst	r2
	bne	1b
	rts	pc

/////////////////////////////////////////////////////////////////////////

tread:
	mov	fio,r0
	sys	read; tapeb; 8192.
	bes	trderr
	cmp	r0,$8192.
	bne	trderr
	rts	pc

trderr:
	jsr	r5,mesg
		<Tape read error\n\0>; .even
	tstb	fli
	jeq 	done		/ don't ignore the error, stop
	mov	$tapeb,r0
2:
	clr	(r0)+
	cmp	r0,$tapeb+8192.
	blo	2b
	rts	pc

/////////////////////////////////////////////////////////////////////////

twrite:
	inc	nused		/ keep track of number blocks written to tape
	mov	fio,r0
	sys	write; tapeb; 8192.
	bes	twrerr
	cmp	r0,$8192.
	bne	twrerr
	rts	pc

twrerr:
	jsr	r5,mesg
		<Tape write error\n\0>; .even
	jbr	done

/////////////////////////////////////////////////////////////////////////

verify:
	movb	(r5)+,0f
	inc	r5
	tstb	flq
	bne	1f
	tstb	flv
	beq	2f
1:
	jsr	pc,9f
.data
9:
	jsr	r5,mesg
		0:<x \0>; .even
	rts	pc
.text
	mov	r1,-(sp)
	mov	$name,r1
	jsr	pc,pstr
	mov	(sp)+,r1
	tstb	flq
	beq	1f
	jsr	r5,mesg
		< \0>
	jsr	pc,getc
	cmp	r0,$'x
	bne	3f
	jsr	pc,getc
	jbr	done
3:
	cmp	r0,$'\n
	beq	3f
	cmp	r0,$'y
	bne	4f
	jsr	pc,getc
	cmp	r0,$'\n
	beq	2f
4:
	jsr	pc,getc
	cmp	r0,$'\n
	bne	4b
	br	1b
1:
	jsr	r5,mesg
		<\n\0>
2:
	tst	(r5)+
3:
	rts	r5

/////////////////////////////////////////////////////////////////////////

getfiles:
	cmp	narg,$2
	bne	1f
	mov	$".\0,name
	jsr	pc,callout
1:
	cmp	narg,$2
	ble	1f
	dec	narg
	mov	*parg,r1
	add	$2,parg
	mov	$name,r2
2:
	movb	(r1)+,(r2)+
	bne	2b
	jsr	pc,callout
	br	1b
1:
	rts	pc

/////////////////////////////////////////////////////////////////////////

expand:
	sys	open; name; 0
	bes	fserr
	mov	r0,-(sp)
1:
	mov	(sp),r0
	sys	read; catlb; 16.
	bes	fserr
	tst	r0
	beq	1f
	tst	catlb
	beq	1b
	mov	$name,r0
	mov	$catlb+2,r1
	cmpb	(r1),$'.
	beq	1b
2:
	tstb	(r0)+
	bne	2b
	dec	r0
	mov	r0,-(sp)
	cmpb	-1(r0),$'/
	beq	2f
	movb	$'/,(r0)+
2:
	movb	(r1)+,(r0)+
	bne	2b
	jsr	pc,callout
	clrb	*(sp)+
	br	1b
1:
	mov	(sp)+,r0
	sys	close
	rts	pc

/////////////////////////////////////////////////////////////////////////

fserr:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- Cannot open file\n\0>; .even
	jbr	done

/////////////////////////////////////////////////////////////////////////

callout:
	sys	stat; name; statb
	bes	fserr
	mov	$dir,r1
	clr	-(sp)
1:
	tst	(r1)
	bne	3f
	tst	(sp)
	bne	2f
	mov	r1,(sp)
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
	mov	(sp)+,r1
	bne	4f
	jsr	r5,mesg
		<Directory overflow\n\0>; .even
	jbr	done
4:
	tstb	fls		/ selective file dump ??
	beq	5f
	cmp	seltim,statb+32.
	blo	5f		/ will dump
	bhi	1f		/ dont dump
	cmp	seltim+2,statb+34.
	bhis	1f		/ dont dump
5:
	jsr	r5,verify; 'a
		rts pc
	jsr	r5,encode; name
	br	2f
3:
	jsr	r5,decode; name1
	mov	$name,r2
	mov	$name1,r3
3:
	cmpb	(r2)+,(r3)
	bne	2b
	tstb	(r3)+
	bne	3b
	tst	(sp)+
3:
	jsr	r5,verify; 'r
		rts pc
2:
	mov	statb+4,mode(r1)
	movb	statb+7,uid(r1)
	movb	statb+8,gid(r1)
	clr	size0(r1)		/ only 24 bits for the moment
	movb	statb+9.,size0(r1)	/ only 24 bits for the moment
	mov	statb+10.,size1(r1)
	mov	statb+32.,time0(r1)
	mov	statb+34.,time1(r1)
	mov	statb+4,r0
	bic	$!60000,r0
	beq	0f		/ plain file so branch
	clr	size0(r1)
	clr	size1(r1)
	cmp	r0,$40000
	jeq	expand		/ directory so see whats in it
	mov	statb+12.,tmajmin(r1)	/ identify the special file
0:
	rts	pc
1:
	bic	$!60000,statb+4
	cmp	$40000,statb+4
	jeq	expand
	rts	pc


/////////////////////////////////////////////////////////////////////////

gettape:
	mov	$dir,r1
	clr	-(sp)
1:
	tst	(r1)		/ end of the incore directory ??
	beq	5f		/ --> yes.
	jsr	r5,decode; name
	cmp	rnarg,$2
	ble	4f
	mov	$name,r2
	mov	*parg,r3
3:
	tstb	(r3)
	beq	3f
	cmpb	(r2)+,(r3)+
	beq	3b
	br	2f
3:
	tstb	(r2)
	beq	4f
	cmpb	(r2),$'/
	bne	2f
4:
	mov	r1,-(sp)
	mov	r5,-(sp)
	jsr	pc,*(r5)		/ call named routine to do the work
	mov	(sp)+,r5
	mov	(sp)+,r1
	inc	(sp)
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
5:
	tst	(sp)+
	bne	2f
	cmp	rnarg,$2
	ble	2f
	mov	*parg,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< not found\n\0>; .even
2:
	dec	narg
	add	$2,parg
	cmp	narg,$2
	bgt	gettape
	tst	(r5)+
	rts	r5

/////////////////////////////////////////////////////////////////////////

numb:
	mov	r1,-(sp)
	mov	r0,-(sp)
	clr	r0
	br	1f

numbx:
	mov	r1,-(sp)
	mov	r0,-(sp)
	mov	size0(r1),r0
1:
	mov	$catlb,r2
1:
	mov	$"  ,(r2)+
	cmp	r2,$catlb+12.
	blo	1b
	cmp	(r5),$2
	bne	1f
	mov	$"00,-2(r2)
1:
	mov	(sp)+,r1
	jsr	pc,numb2
	mov	(r5)+,r0
	sub	r0,r2
	mov	r2,0f
	mov	r0,0f+2
	mov	$1,r0
	sys	0; 9f
.data
9:
	sys	write; 0:..; ..
.text
	mov	(sp)+,r1
	rts	r5

numb1:
	clr	r0
numb2:
	div	$10.,r0
	mov	r1,-(sp)
	mov	r0,r1
	beq	1f
	jsr	pc,numb1
1:
	mov	(sp)+,r0
	add	$'0,r0
	movb	r0,(r2)+
	rts	pc

/////////////////////////////////////////////////////////////////////////

make:
	mov	$dir,r1
1:
	tst	(r1)		/ end of the incore directory ??
	beq	0f		/ --> yes.
	mov	tapad,tapea(r1)	/ store tape address in tape dir entry
	bit	$60000,mode(r1)
	bne	2f		/ --> not a plain file
	mov	size0(r1),r2
	mov	size1(r1),r3
	add	$511.,r3
	adc	r2
	ashc	$-9,r2		/ file size (next highest multiple of 512 bytes)
	add	r3,tapad	/ increment by size of file - ready for next file
2:
	add	$dirsiz,r1
	cmp	r1,edir
	blo	1b
0:
	jsr	pc,wrdir

make1:
	mov	$dir,r1
1:
	tst	(r1)		/ end of the incore directory ??
	beq	1f		/ --> yes.
	bit	$60000,mode(r1)
	bne	2f		/ --> not a plain file.
	tst	size0(r1)
	bne	9f
	tst	size1(r1)
	beq	2f
9:
	mov	size0(r1),r2
	mov	size1(r1),r3
	add	$511.,r3
	adc	r2
	ashc	$-9,r2		/ file size (next highest multiple of 512 bytes)
	jsr	r5,decode; name	/ get name from tape entry
	sys	open; name; 0	/ open it for reading.
	bes	phserro
	mov	r0,r2		/ save file descriptor for reads
6:
	mov	r2,r0
	jsr	pc,0f
.data
0:
	sys	read;  7: tapeb  ;  8:  8192.
	rts	pc
.text
	bes	phserrr
	add	r0,7b		/ point next available location in tape buffer
	cmp	$tapeb+8192.,7b	/ file finished ??
	bhi	5f		/ --> yes.
	jsr	pc,twrite
	mov	8b,r0		/ bytes to be read last read.
	ash	$-9,r0		/ get as block count
	mov	$tapeb,7b	/ re-init
	mov	$8192.,8b	/ re-init
	sub	r0,r3		/ adjust file size counter
	bne	6b		/ --> more to read.
	br	4f
5:
	mov	r0,r5		/ save length
	add	$511.,r0
	ash	$-9,r0		/ blocks just read
	sub	r0,r3		/ adjust file size
	bne	phserrs		/ should be zero
	cmpb	r5,size1(r1)	/ check that file same length now
	bne	phserrs
	sub	$tapeb,7b	/ make relative
	add	$511.,7b
	bic	$511.,7b	/ round up next read start address.
	add	$tapeb,7b	/ make absolute.
	mov	$tapeb+8192.,8b
	sub	7b,8b		/ max length next record
	cmp	7b,$tapeb+8192.	/ any room left ??
	bne	4f		/ --> yes, so continue
	jsr	pc,twrite
	mov	$tapeb,7b	/ re-init
	mov	$8192.,8b	/ re-init
4:
	mov	r2,r0
	sys	close
2:
	add	$dirsiz,r1	/ next file
	cmp	r1,edir
	blo	1b		/ loop if not end of directory.
1:
	cmp	7b,$tapeb
	beq	0f		/ only write last block if have to
	jsr	pc,twrite
	mov	$tapeb,7b	/ re-init
	mov	$8192.,8b	/ re-init
0:
	rts	pc

/////////////////////////////////////////////////////////////////////////

phserro:	movb	$'O,phsedes;	br	phserr
phserrs:	movb	$'S,phsedes;	br	phserr
phserrr:	movb	$'R,phsedes;	br	phserr
phserr:
	mov	r1,-(sp)
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- Phase error >
phsedes:	< \n\0>; .even
	mov	(sp)+,r1
	clr	time0(r1) / time
	beq	2b
	sys	close
	br	2b

/////////////////////////////////////////////////////////////////////////

taboc:
	tstb	flv
	jeq	4f
	mov	mode(r1),r0
	mov	r0,-(sp)
	movb	$'-,r5
	mov	$tabpf,mss
	bic	$!60000,r0	/ look at file type bits
	beq	0f		/ --> plain file
	movb	$'d,r5
	mov	$tabdf,mss
	cmp	$40000,r0
	beq	0f		/ --> directory
	movb	$'c,r5
	mov	$tabsf,mss
	cmp	$20000,r0
	beq	0f		/ --> character special
	movb	$'b,r5		/     block special
0:	mov	r5,r0
	jsr	pc,putc
	mov	(sp),r0
	ash	$-6,r0
	bit	$40,r0
	jsr	pc,pmod		/ print access bits for owner
	mov	(sp),r0
	ash	$-3,r0
	bit	$200,r0
	jsr	pc,pmod		/ print access bits for group
	mov	(sp)+,r0
	bit	$1000,r0
	jsr	pc,pmod		/ print access bits for others
	clr	r0
	bisb	uid(r1),r0
	jsr	r5,numb; 4	/ user id.
	clr	r0
	bisb	gid(r1),r0	/ group id
	jsr	r5,numb; 4
	mov	tapea(r1),r0	/ tape address
	jsr	r5,numb; 5
	jmp	*mss		/ jump on file type
tabpf:	mov	size1(r1),r0	/ file size
	jsr	r5,numbx; 9.	/ 32-bit number
	br 	0f
tabdf:	jsr	r5,mesg
		<         \0>; .even
	br	0f
tabsf:	movb	majmin(r1),r0
	jsr	r5,numb; 5
	movb	$',,r0
	jsr	pc,putc
	movb	majmin+1(r1),r0
	jsr	r5,numb; 3
0:	mov	time1(r1),-(sp)
	mov	time0(r1),-(sp)
	jsr	pc,_localtime
	tst	(sp)+
	mov	r0,(sp)
	mov	6(r0),r0		/ day of the month(1-31)
	jsr	r5,numb; 3
	mov	$'/,r0
	jsr	pc,putc
	mov	(sp),r0
	mov	8.(r0),r0		/  month(0-11)
	inc	r0
	jsr	r5,numb; 2
	mov	$'/,r0
	jsr	pc,putc
	mov	(sp),r0
	mov	10.(r0),r0		/ year - 1900
	jsr	r5,numb; 2
	mov	(sp),r0
	mov	4(r0),r0		/ hours(0-23)
	jsr	r5,numb; 3
	mov	$':,r0
	jsr	pc,putc
	mov	(sp)+,r0
	mov	2(r0),r0		/ minutes(0-59)
	jsr	r5,numb; 2
	mov	$' ,r0
	jsr	pc,putc
4:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		<\n\0>
	rts	pc

/////////////////////////////////////////////////////////////////////////

pmod:
	beq	1f
	mov	$'s,-(sp)
	br	2f
1:
	bit	$1,r0
	beq	1f
	mov	$'x,-(sp)
	br	2f
1:
	mov	$'-,-(sp)
2:
	bit	$2,r0
	beq	1f
	mov	$'w,-(sp)
	br	2f
1:
	mov	$'-,-(sp)
2:
	bit	$4,r0
	beq	1f
	mov	$'r,r0
	br	2f
1:
	mov	$'-,r0
2:
	jsr	pc,putc
	mov	(sp)+,r0
	jsr	pc,putc
	mov	(sp)+,r0
	jsr	pc,putc
	rts	pc

/////////////////////////////////////////////////////////////////////////

/	if the file exists delete it iff not a directory.
/	this is necessary otherwise a filesys mess will result

xunlnk:
	sys	stat; name; statb	/ get info on the file.
	bes	0f			/ --> file don't exist
	bic	$!60000,statb+4		/ isolate file type bits
	cmp	$40000,statb+4		/ directory ??
	beq	0f
	sys	unlink; name		/ remove it
0:
	rts	pc

/////////////////////////////////////////////////////////////////////////

xtract:
	tstb	flc		/ create directory mode ??
	beq	0f		/ --> no.
	mov	mode(r1),r0
	mov	r0,9f
	bic	$!60000,r0
	beq	0f		/ --> plain file
	clr	8f		/ assume directory.
	cmp	r0,$40000
	beq	4f		/ --> directory.
	mov	majmin(r1),8f	/ special file.
4:
	jsr	pc,xunlnk
	jsr	r5,verify; 'c
		rts	pc
	bic	$!60777,9f
	sys	0; 7f
.data
7:
	sys	14.; name; 9: 0; 8: 0	/ create directory or special file
.text
	jes	crterr
	tst	8b
	bne	5f		/ --> special file( 0,0 will slip thru don't matter)
	clr	r5
	mov	$name,r0
8:
	cmpb	(r0),$'/
	bne	9f
	mov	r0,r5		/ save  the location of this slash -- it may be the last
9:
	tstb	(r0)+
	bne	8b		/ end of filename not yet encountered.
	movb	$'/,-1(r0)
	movb	$'.,(r0)+
	clrb	(r0)		/ 'name' is dot in directory just made
	mov	(r1),8f		/ set up pathname address in link
	sys	0; 9f
.data
9:
	sys	link; 8: 0; name
.text
	movb	$'.,(r0)+
	clrb	(r0)		/ 'name' is dotdot in current directory.
	mov	$dotname,8f	/ default prior file name
	sub	$name,r5
	blt	9f		/ --> no slash in the name
	add	(r1),r5		/ modify it in the string area
	clrb	(r5)		/ 'name' is previous dir name
	mov	(r1),8f		/ set up pathname address in link
9:
	sys	0;9f
.data
9:
	sys	link; 8: 0; name
.text
	br	5f
0:
	bit	$60000,mode(r1)
	bne	1f		/ --> not a plain file(ignore since flc=0)
	jsr	r5,verify; 'x
		rts pc
	mov	tapad,r5	/ current tape posn
4:
	mov	tapea(r1),r0	/ tape address of file
	cmp	r0,r5		/ how goes the position ??
	beq	0f		/ we are ther already.
	add	$16.,r5		/ address the start of
	bic	$17,r5		/ the next tape block.
	cmp	r0,r5
	blo	0f		/ what we want is in this block
	jsr	pc,tread	/ get another block
	mov	r5,tapad	/ update current position.
	br	4b
0:
	jsr	pc,xunlnk
	mov	mode(r1),0f
	sys	0; 9f
.data
9:
	sys	creat; name; 0:..
.text
	bes	crterr1		/ unable to create the file ??

	mov	r0,r2		/ save file descriptor
	mov	tapea(r1),r5	/ tape address of the file
2:
	bic	$!17,r5		/ get block displacement in tape buffer
	ash	$9,r5		/ get as byte displacement
	mov	r5,9f
	add	$tapeb,9f	/ start writing from here.
	add	$-8192.,r5
	neg	r5		/ bytes left in buffer
	mov	r5,8f
	sub	r5,size1(r1)
	sbc	size0(r1)	/ is this all the file ??
	bge	0f		/ --> no.
	add	size1(r1),r5
	mov	r5,8f
	beq	2f		/ --> if null file/multiple of ....
	clr	r5
0:
	mov	r2,r0
	sys	0; 7f
.data
7:
	sys	write; 9: 0; 8: 0
.text
	bes	crterr1		/ write error
	cmp	r0,8b
	bne	crterr1		/ write error (out of space probably)
	tst	r5
	ble	2f		/ all done
	jsr	pc,tread
	add	$16.,tapad
	bic	$17,tapad
	mov	tapad,r5
	br	2b		/ go again
2:
	mov	r2,r0
	sys	close
5:
	movb	gid(r1),0f+1
	movb	uid(r1),0f
	sys	0; 9f
.data
9:
	sys	chown; name; 0:..
.text
	mov	time0(r1),r0
	mov	r1,-(sp)
	mov	time1(r1),r1
/	sys	0; 9f
.data
9:
	sys	smdate; name
.text
	mov	(sp)+,r1
1:
	rts	pc
dbgg:
	mov r2,-(sp);jsr r5,numb; 5;mov (sp)+,r2
	mov	$'\n,r0;jsr pc,putc
	rts	pc

crterr1:
	clr	r0
	mov	r1,-(sp)
	clr	r1
/	sys	smdate; name
	mov	(sp)+,r1
	mov	r2,r0
	sys	close

crterr:
	mov	$name,r1
	jsr	pc,pstr
	jsr	r5,mesg
		< -- create error\n\0>; .even
	rts	pc


//////////////////////////////////////////////////////////////////////////

.data
tapad:	[mdirent\/8]
edir:	dirend
ldt:	</etc/last-dump-time\0>
dotname:	<.\0>
mt:	</dev/rmt>
/	mt:	</big/mag>
mtx:	<0\0>
mboot:	</usr/mdec/mboot\0>
.even
.bss
dir:	. = .+[mdirent*dirsiz]
dirend:
ch:	.=.+1
fli:	.=.+1
flu:	.=.+1
fls:	.=.+1
flc:	.=.+1
flv:	.=.+1
flq:	.=.+1
.even

seltim: .=.+4	/ all files modified after this time dumped
startim: .=.+4	/ itp commenced at this time.
command:.=.+2
sum:	.=.+2
size:	.=.+2
nused:	.=.+2
nfree:	.=.+2
catlb:	.=.+20.
narg:	.=.+2
rnarg:	.=.+2
parg:	.=.+2
fio:	.=.+2
mss:	.=.+2
ndirent: .=.+2
name:	.=.+64.
name1:	.=.+64.
statb:	.=.+40.

smdate = 30.
