/ UNIX overlay linker, minimal run-time support
/ J. N. Rottman

	.globl	__o_aut, __o_file, __o_name, __o_segs
	.globl	__o_sege, __o_segl

.data

__o_aut:
	mov	r5, -(sp)
	mov	(r4)+,r5
	jsr	pc,segload
	mov	(sp)+,r5
	mov	(r4),r4
	rts	r4

segload:
	mov	r0,-(sp)
	mov	(r5)+, seek1+2
	bmi	3f
	mov	(r5)+,seek2+2
	mov	(r5)+,read1+4
	mov	(r5),read1+2
	mov	fd,r0
	bpl	seek1
	sys	open; __o_file: __o_name; 0
	mov	r0,fd
seek1:	sys	seek; 0; 0
	mov	fd,r0
seek2:	sys	seek; 0; 4
	mov	fd,r0
read1:	sys	read; 0; 0

	mov	$__o_segs+6,r0
2:	cmp	(r0),2(r5)
	bhis	1f
	cmp	(r5),2(r0)
	bhis	1f
	bic	$100000,-6(r0)
1:	add	$12.,r0
	cmp	r0,$__o_sege
	blo	2b
	bis	$100000,-6(r5)
	mov	(r5)+, r0
	add	-4(r5),r0
4:	cmp	r0,(r5)
	bhis	3f
	clr	(r0)+
	br	4b
3:	mov	(sp)+, r0
	rts	pc

fd:	-1

__o_segl:
	mov	$__o_segs+10.,r0
2:	cmp	r0,2(sp)
	beq	1f
	add	$12.,r0
	cmp	r0,$__o_sege
	blo	2b
	mov	$-1,r0
	rts	pc
1:	mov	r5,-(sp)
	sub	$10.,r0
	mov	r0,r5
	jsr	pc,segload
	mov	(sp)+,r5
	rts	pc

