/ file= ld8.s

/
/ 8 bit loader for gt40
/  loaded by GTL through the 6-bit bootstrap loader
/  to allow 8 bit loading.
/
memsiz=	040000

	.globl	_ld8siz		/minimum size
	.globl	_ld8fix		/relocating subroutine (c callable)
	.globl	_ld8,	_ld8l	/loading information
	.globl	_ld8adr		/
	.globl	_ld8adx


/////////////////////////////////

	.globl	csv,	cret	/c routines

/ C interface to the assembler bootstrap
/ used to relocate the routines.

_ld8fix:
	jsr	r5,csv		/c linkage
	mov	4(r5),r0	/get argument
	mov	r0,r1
	sub	_ld8adr,r0	/any previous relocations?
	mov	r1,_ld8adr
	add	r0,za		/relocate the loader.
	add	r0,zb
	jmp	cret		/c type exit

.data

_ld8adr:	[0]		/address that loader was last relocated to

/////////////////////////////////

/
/ constants
/
dsr	=	172002
rcsr	=	175610
rbuf	=	175612
/
halt	=	000000
/

/ register usage

/r0 byte buffer
/r1 temp reg. for word joining
/r2 byte count
/r3 adr
/r4 input subroutine
/r5 checksum

_ld8:	mov	$[1f-_ld8],sp	/set up stack
za =	.-2
	mov	$[9f-_ld8],r4	/input routine
zb =	.-2

zc =	.-4	/necessary stack space

1:	jsr	pc,(r4)		/1st char.
2:	dec	r0		/is it a one?
	bne	1b		/no. keep on trying
	jsr	pc,(r4)		/get null
	bne	2b		/not null. try 1.
	clr	r5		/init checksum.
				/note: have to add $1 to checksum
				/      from now on, (biased)
	jsr	pc,8f		/get word. byte count
	mov	r0,r2		/byte count
	jsr	pc,8f		/get adr.
	mov	r0,r3		/adr loaded
	jsr	pc,(r4)		/read ahead for checksum
	sub	$6,r2		/test count
	ble	5f		/"go to" part
3:	movb	r0,(r3)+	/store byte
	jsr	pc,(r4)		/next byte
	dec	r2		/all in?
	bne	3b
	incb	r5		/how is checksum?
	beq	1b		/o.k. - next record
4:	clr	*$dsr		/error. ring bell!!
	br	4b
5:	blt	4b		/record size .lt. 2
	incb	r5		/checksum?
	bne	4b
/
/ do we really need the next 5 instructions?
/ it may be possible to count on gtl itself
/
	bit	$1,r3		/odd?
	beq	7f		/real "go to"
6:	halt
	br	6b
7:	jmp	(r3)		/off we go.
/
/ word read routine
/
8:	jsr	pc,(r4)		/byte 1
	mov	r0,r1
	jsr	pc,(r4)		/byte 2
	swab	r0		/get it into right position
	bis	r1,r0		/join them
	rts	pc
/
/ byte read routine
/
9:	tstb	*$rcsr
	bpl	9b		/ready?
	movb	*$rbuf,r0	/char.
	add	r0,r5		/checksum
	bic	$!377,r0	/keep bottom 8 bits
	rts	pc

_ld8l	=	.-_ld8		/load size
_ld8siz	=	.-zc		/run size

_ld8adx	=	memsiz-_ld8l	/possible preferred address
