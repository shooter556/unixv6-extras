	sys	signal;2;0
	sys	signal;3;0
0:	mov	$2,r0
	sys	write;d;dl
	br	0b
d:	.byte	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
dl = .-d
