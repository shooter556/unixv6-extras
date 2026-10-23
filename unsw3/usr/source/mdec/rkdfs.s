	.globl	_restart,_main,savr5,_initial,_initiam
.text
	jmp	start
.=1000^.
start:	5		/ reset
	mov	$start,sp
	mov	$buss,*$4		/ pick up bus errors
	jsr	pc,_initial
buss:	mov	$start,*$4		/ normal bus error exit
	jsr	pc,_initiam
	5		/ reset
	mov	$start,sp
	jsr	pc,_main
_restart:
	jmp	start
.bss
savr5:	. = . +2
