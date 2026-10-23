	.text
	.globl	_add
_add:
	mov	2(sp),r0
	add	4(sp),2(r0)
	adc	(r0)
	rts	pc

	.globl	_delay,_puts
_delay:
	mov	r2,-(sp)
	mov	$4f,r2
	mov	4(sp),r1	/ r1 = total delay
	jeq	0f

	clr	r0
	div	$60.,r0
	jsr	pc,*$5f		/ r1 = delay in minutes
	mov	r0,r1
	jeq	0f

	clr	r0
	div	$24.,r0	
	jsr	pc,*$5f		/ r1 = delay in hours
	mov	r0,r1
	jeq	0f

	mov	$1f,-(sp)	/ r1 = delay in days
	jbr	5f
0:
	movb	$' ,-(r2)
	cmp	r2,$3f
	jgt	0b
1:
	mov	$2f,-(sp)
	jsr	pc,_puts
	tst	(sp)+
	mov	(sp)+,r2
	mov	2(sp),r0
	rts	pc

2:	< - >; 3: <dd hh mm>; 4: <\0>

5:				/ print delay in form b'dd
	mov	r0,-(sp)
	clr	r0
	div	$10.,r0
	add	$'0,r1
	movb	r1,-(r2)
	mov	r0,r1
	jeq	0f
	add	$'0,r1
	movb	r1,-(r2)
	jbr	1f
0:
	movb	$' ,-(r2)
1:
	movb	$' ,-(r2)
	mov	(sp)+,r0
	rts	pc
	.globl _ldiv
_ldiv:
	mov	2(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	div	4(sp),r0
	rts	pc
	.globl _position,_sumdes
_position:
	tst	2(sp)
	jne	0f
	mov	4(sp),r0
	mul	6(sp),r0
	mov	r1,-(sp)
	ashc	$-9,r0
	mov	r1,block
	mov	(sp)+,r0
	bic	$!777,r0
	movb	r0,into
0:
	mov	_sumdes,r0
	sys	seek; block: ..; 3
	sys	seek; into:  ..; 1
	rts	pc

	.globl	_scan, _action
_scan:
	mov	r2,-(sp)
	mov	*$_action,r2
	mov	r3,-(sp)
	mov	$0f,r3
	mov	r4,-(sp)
	mov	10(sp),-(sp)
	jsr	pc,(r3)
	tst	(sp)+
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
0:
	mov	2(sp),r0
	mov	r4,-(sp)
	mov	r0,r4
	tst	r4
	jeq	1f
	mov	2(r4),-(sp)
	jsr	pc,(r3)
	mov	(r4),(sp)
	jsr	pc,(r2)
	mov	4(r4),(sp)
	jsr	pc,(r3)
	tst	(sp)+
1:
	mov	(sp)+,r4
	rts	pc
timezone = 0.*60.*60.
secin8hr = 8.*60.*60.
.globl	_time,_minute,_hour,_days,_day,_year,_base,_ytime
_ytime:
	mov	pc,flag
	jbr	0f
_time:
	clr	flag
0:
	mov	2(sp),r0
	mul	$60.,r0		/ get seconds in r0.r1
	add	_base,r0
	add	_base+2,r1
	adc	r0
	mov	r2,-(sp)
	mov	r3,-(sp)

	sub	$timezone,r1
	sbc	r0		/ time zone correction.

	div	$secin8hr,r0	/ r0= no of 8 hr lots since start 1970
				/ r1= no of seconds into 8 hour lot
	mov	r0,r3
	clr	r2
	div	$3,r2		/ r2= no of days since start of 1970
				/ r3= no of 8 hours in day
	clr	r0
	div	$60.,r0		/ r0= no of minutes in 8 hr grp
				/ r1= no of seconds .......
	mov	r0,r1
	clr	r0
	div	$60.,r0		/ r0= no of hour into 8 hr grp
				/ r1= no of minutes ..........
	mov	r1,_minute
	jsr	r5,numbin
	minute			/ put in minutes

	mov	r3,r1
	mul	$8.,r1
	add	r0,r1		/ r1= no of hours.
	mov	r1,_hour
	jsr	r5,numbin
	hour			/ put in hours

/	r2= no of dayes since start of 1970 (a thursday).
	clr	r0
	mov	r2,r1
	div	$7.,r0		/ r1 = day of week 0->6 :: thur->wed
	mov	r1,_day
	tst	flag
	jeq	0f

	jsr	r5,stringin
	weekday;thu		/ put in day of week
0:

	mov	$69.,r1
0:	inc	r1
	mov	$365.,r0
	bit	$3,r1
	bne	1f
	inc	r0
1:	sub	r0,r2
	bge	0b
	add	r0,r2		/ r2= days in current year
	mov	r2,_days
	mov	r1,_year
	jsr	r5,numbin; year+2
	clr	r0		/ leap year ??
	div	$4,r0
	tst	r1
	bne	2f
	mov	$29.,mtab+2
2:	mov	$mtab,r0
0:	sub	(r0)+,r2
	bge	0b
	add	-(r0),r2
	sub	$mtab,r0
	asr	r0		/ r0= month in year 0->11
	inc	r2		/ r2= day in month 1->31

	mov	r2,r1
	tst	flag
	jne	2f
	cmp	r0,pmon
	jne	0f
	cmp	r2,pday
	jne	1f
	mov	$"  ,daymonth
	jbr	3f
0:	mov	r0,pmon
1:	mov	r2,pday
2:
	jsr	r5,numbin
	daymonth		/ put in day of month
3:

	mov	r0,r1
	jsr	r5,stringin
	month;jan		/ put in month name.
	tst	flag
	jeq	0f
	mov	$weekday,r0
	jbr	1f
0:	mov	$daymonth,r0
1:

	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
numbin:
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	(r5)+,r2
	clr	r0
	div	$10.,r0
	bis	$'0,r0
	bis	$'0,r1
	movb	r0,(r2)+
	movb	r1,(r2)
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	rts	r5
stringin:
	mov	r0,-(sp)
	mov	r2,-(sp)
	mov	(r5)+,r2
	mul	$3,r1
	add	(r5)+,r1
	movb	(r1)+,(r2)+
	movb	(r1)+,(r2)+
	movb	(r1),(r2)
	mov	(sp)+,r2
	mov	(sp)+,r0
	rts	r5

	.even
mtab:	31.;28.;31.;30.;31.;30.;31.;31.;30.;31.;30.;31.
pmon:	0
flag:	0
pday:	0
weekday:	<DAY >
month:		<MON >
daymonth:	<DD >
hour:		<HH:>
minute:		<MM >
year:		<19YY\0>
thu:	<thufrisatsunmontuewed>
jan:	<janfebmaraprmayjunjulaugsepoctnovdec>

