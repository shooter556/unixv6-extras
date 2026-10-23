.globl	_aa
.data
_aa:
53
.globl	_bb
.data
_bb:
54
.globl	_main
.text
_main:
~~main:
jsr	r5,csv
jbr	L1
L2:~a=r4
mov	$26,-10(r5)
~x=177770
mov	$26,-10(r5)
clr	r0
mov	r0,-10(r5)
mov	r0,r4
~x=r4
~y=r3
~z=r2
clr	r2
mov	r2,r0
mov	r0,r3
mov	r3,r4
L3:jmp	cret
L1:tst	-(sp)
jbr	L2
.globl
.data
