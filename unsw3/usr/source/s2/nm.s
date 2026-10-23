.comm	_fout,2
.comm	_cflg,2
.comm	_nflg,2
.comm	_uflg,2
.globl	_rflg
.data
_rflg:
1
.comm	_gflg,2
.comm	_pflg,2
.comm	_nlp,2
.comm	_fi,2
.comm	_buf,20
.globl	_main
.text
_main:
~~main:
jsr	r5,csv
~argc=4
~argv=6
jbr	L1
L2:~n=177770
~i=177766
~j=177764
~ls=177760
dec	4(r5)
jle	L4
mov	6(r5),r0
cmpb	$55,*2(r0)
jne	L4
add	$2,6(r5)
L5:add	$1,*6(r5)
mov	*6(r5),r0
tstb	(r0)
jeq	L6
mov	*6(r5),r0
movb	(r0),r0
jbr	L8
L9:inc	_nflg
jbr	L5
L10:inc	_cflg
jbr	L5
L11:inc	_gflg
jbr	L5
L12:inc	_uflg
jbr	L5
L13:mov	$-1,_rflg
jbr	L5
L14:inc	_pflg
jbr	L5
L15:jbr	L5
jbr	L7
L8:sub	$143,r0
cmp	r0,$22
jhi	L15
asl	r0
jmp	*L10001(r0)
.data
L10001:L10
L15
L15
L15
L11
L15
L15
L15
L15
L15
L15
L9
L15
L14
L15
L13
L15
L15
L12
.text
L7:jbr	L5
L6:dec	4(r5)
L4:tst	4(r5)
jne	L16
clr	(sp)
mov	$L17,-(sp)
jsr	pc,*$_open
tst	(sp)+
mov	r0,_fi
jbr	L18
L16:clr	(sp)
add	$2,6(r5)
mov	*6(r5),-(sp)
jsr	pc,*$_open
tst	(sp)+
mov	r0,_fi
L18:tst	_fi
jge	L19
mov	$L20,(sp)
jsr	pc,*$_printf
jsr	pc,_exit
L19:mov	$20,(sp)
mov	$_buf,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_read
cmp	(sp)+,(sp)+
cmp	$407,_buf
jeq	L21
cmp	$410,_buf
jeq	L21
cmp	$411,_buf
jeq	L21
cmp	$412,_buf
jeq	L21
mov	$L22,(sp)
jsr	pc,*$_printf
jsr	pc,_exit
L21:mov	$1,(sp)
mov	2+_buf,r1
clr	r0
mov	r1,-16(r5)
mov	r0,-20(r5)
mov	r1,-(sp)
mov	r0,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_lseek
add	$6,sp
mov	$1,(sp)
mov	4+_buf,r1
clr	r0
mov	r1,-16(r5)
mov	r0,-20(r5)
mov	r1,-(sp)
mov	r0,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_lseek
add	$6,sp
cmp	$1,16+_buf
jeq	L23
mov	$1,(sp)
mov	2+_buf,r1
clr	r0
mov	r1,-16(r5)
mov	r0,-20(r5)
mov	r1,-(sp)
mov	r0,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_lseek
add	$6,sp
mov	$1,(sp)
mov	4+_buf,r1
clr	r0
mov	r1,-16(r5)
mov	r0,-20(r5)
mov	r1,-(sp)
mov	r0,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_lseek
add	$6,sp
L23:mov	$14,(sp)
mov	10+_buf,-(sp)
clr	-(sp)
jsr	pc,*$_ldiv
cmp	(sp)+,(sp)+
mov	r0,-10(r5)
tst	-10(r5)
jne	L24
mov	$L25,(sp)
jsr	pc,*$_printf
jsr	pc,_exit
L24:mov	-10(r5),r1
mul	$14,r1
mov	r1,(sp)
jsr	pc,*$_sbrk
mov	r0,_nlp
mov	-10(r5),r1
mul	$14,r1
mov	r1,(sp)
mov	_nlp,-(sp)
mov	_fi,-(sp)
jsr	pc,*$_read
cmp	(sp)+,(sp)+
tst	_pflg
jne	L26
mov	$_compare,(sp)
mov	$14,-(sp)
mov	-10(r5),-(sp)
mov	_nlp,-(sp)
jsr	pc,*$_qsort
add	$6,sp
L26:mov	$1,(sp)
jsr	pc,*$_dup
mov	r0,_fout
mov	$1,(sp)
jsr	pc,*$_close
clr	-12(r5)
L27:cmp	-10(r5),-12(r5)
jle	L28
tst	_gflg
jeq	L10002
mov	_nlp,r0
bit	$40,10(r0)
jeq	L30
L10002:tst	_cflg
jeq	L31
cmpb	$137,*_nlp
jne	L30
clr	-14(r5)
L32:cmp	$7,-14(r5)
jle	L33
mov	_nlp,r0
add	-14(r5),r0
mov	_nlp,r1
add	-14(r5),r1
movb	1(r1),(r0)
L34:inc	-14(r5)
jbr	L32
L33:mov	_nlp,r0
clrb	7(r0)
L31:mov	_nlp,r0
mov	10(r0),r0
bic	$-40,r0
mov	r0,-14(r5)
tst	_uflg
jeq	L10003
tst	-14(r5)
jne	L30
L10003:tst	_uflg
jne	L35
tst	-14(r5)
jne	L36
mov	_nlp,r0
tst	12(r0)
jne	L36
mov	$L37,(sp)
jsr	pc,*$_printf
jbr	L38
L36:mov	_nlp,r0
mov	12(r0),(sp)
jsr	pc,*$_printo
L38:cmp	$4,-14(r5)
jge	L39
mov	-14(r5),r0
jbr	L41
L42:mov	$6,-14(r5)
jbr	L40
L43:mov	$7,-14(r5)
jbr	L40
L44:mov	$10,-14(r5)
jbr	L40
jbr	L40
L41:mov	$L10005,r1
mov	r0,L10006
L10007:cmp	r0,(r1)+
jne	L10007
jmp	*L10006-L10005(r1)
.data
L10005:24
37
L10006:..
L43
L42
L44
.text
L40:jbr	L45
L39:tst	-14(r5)
jne	L46
mov	_nlp,r0
tst	12(r0)
jeq	L46
mov	$5,-14(r5)
L46:L45:mov	_nlp,r0
bit	$40,10(r0)
jeq	L10008
mov	$L48,r0
jbr	L10009
L10008:mov	$L49,r0
L10009:add	-14(r5),r0
movb	(r0),r0
mov	r0,(sp)
mov	$L47,-(sp)
jsr	pc,*$_printf
tst	(sp)+
L35:mov	_nlp,(sp)
mov	$L50,-(sp)
jsr	pc,*$_printf
tst	(sp)+
L30:add	$14,_nlp
L29:inc	-12(r5)
jbr	L27
L28:jsr	pc,_flush
L3:jmp	cret
L1:sub	$12,sp
jbr	L2
.globl	_compare
.text
_compare:
~~compare:
jsr	r5,csv
~p1=4
~p2=6
jbr	L51
L52:~a=177770
~i=177766
clr	-10(r5)
tst	_nflg
jeq	L54
mov	4(r5),r0
mov	6(r5),r1
cmp	12(r1),12(r0)
jhis	L55
mov	$1,-10(r5)
jbr	L56
L55:mov	4(r5),r0
mov	6(r5),r1
cmp	12(r1),12(r0)
jlos	L57
mov	$-1,-10(r5)
jbr	L56
L57:L54:clr	-12(r5)
L58:cmp	$10,-12(r5)
jle	L59
mov	4(r5),r0
add	-12(r5),r0
mov	6(r5),r1
add	-12(r5),r1
cmpb	(r1),(r0)
jeq	L61
mov	4(r5),r0
add	-12(r5),r0
mov	6(r5),r1
add	-12(r5),r1
cmpb	(r1),(r0)
jge	L62
mov	$1,-10(r5)
jbr	L63
L62:mov	$-1,-10(r5)
L63:jbr	L56
L61:L60:inc	-12(r5)
jbr	L58
L59:L56:mov	-10(r5),r1
mul	_rflg,r1
mov	r1,r0
jbr	L53
L53:jmp	cret
L51:sub	$4,sp
jbr	L52
.globl	_printo
.text
_printo:
~~printo:
jsr	r5,csv
~v=4
jbr	L64
L65:~i=177770
tst	4(r5)
jge	L10010
mov	$61,(sp)
jbr	L10011
L10010:mov	$60,(sp)
L10011:mov	$L67,-(sp)
jsr	pc,*$_printf
tst	(sp)+
clr	-10(r5)
L68:cmp	$5,-10(r5)
jle	L69
mov	4(r5),r0
ash	$-14,r0
bic	$-10,r0
add	$60,r0
mov	r0,(sp)
mov	$L71,-(sp)
jsr	pc,*$_printf
tst	(sp)+
mov	4(r5),r0
ash	$3,r0
mov	r0,4(r5)
L70:inc	-10(r5)
jbr	L68
L69:L66:jmp	cret
L64:tst	-(sp)
jbr	L65
.globl
.data
L17:.byte 141,56,157,165,164,0
L20:.byte 143,141,156,156,157,164,40,157,160,145,156,40,151,156,160,165,164,12,0
L22:.byte 142,141,144,40,146,157,162,155,141,164,12,0
L25:.byte 156,157,40,156,141,155,145,40,154,151,163,164,12,0
L37:.byte 40,40,40,40,40,40,0
L47:.byte 45,143,40,0
L48:.byte 125,101,124,104,102,103,106,132,77,0
L49:.byte 165,141,164,144,142,143,146,172,77,0
L50:.byte 45,56,70,163,12,0
L67:.byte 45,143,0
L71:.byte 45,143,0
