#
/*
 * a memory contents display program.
 * it should use a vt05 terminal's cursor control facilities.
 *
 * from the beautiful land of rsx-11d.
 */

#include	<defines.h>
#include	<param.h>
#include	<proc.h>
#include	<text.h>
char	*lks;		/* declared in	*/
int	cputype,maxmem;
long	time;		/*/usr/sys/systm.h */

/*#define	K64	0*/
/*#define	TDATA	0*/

/*
 * vt05 control characters
 *
 *  016	cursor addressing (040...)
 *  035	home beam
 *  037	clear screen
 *  036	clear line
 *  032	cursor up
 *  013	cursor down
 *  010	cursor left
 *  030	cursor right
 */
/*
 * transdata control characters
 *
 *  023	cursor addressing (000...)
 *  031	swap pages
 *  035	home beam
 *  034	clear screen
 *  037 clear line
 *  017	cursor up
 *  013	cursor down
 *  010 cursor left
 *  011	cursor right
 */

#ifndef	TDATA
char	vt05_1[] {
"\035\0\0\0\0\037\0\0\0\0********************* UNIX MEMORY CONTENTS DISPLAY *********************"
	} ;
char	vt05_2[] {
#ifndef	K64
"\016\042\0\0\0\0\040*******8******16******24******32******40******48******56******64 swapped"
#endif
#ifdef	K64
"\016\042\0\0\0\0\040*******4*******8******12******16******20******24******28******32 swapped"
#endif
	} ;
char	vt05_3[] {
#ifndef	K64
"\016\053\0\0\0\0\040******72******80******88******96*****104*****112*****120*****128"
#endif
#ifdef	K64
"\016\053\0\0\0\0\040******36******40******44******48******52******56******60******64"
#endif
	} ;
char	vt05_a[] { "\016\041\0\0\0\0\042Machine is PDP 11/" } ;
char	vt05_b[] { "; Clock is KW11-" } ;
char	vt05cadx[] { "\016x\0\0\0\0x" } ;
char	vt05time[] { "\016\041\0\0\0\0\116" } ;
#endif
#ifdef	TDATA
char	tdata_1[] {
"\035\0\0\0\0\0\0\0\0\034\0\0\0\0\0\0\0\0********************* unix memory contents display *********************"
	} ;
char	tdata_2[] {
#ifndef	K64
"\023\000\0\0\0\0\002\0\0\0\0*******8******16******24******32******40******48******56******64 swapped"
#endif
#ifdef	K64
"\023\000\002\0\0\0\0\0\0\0\0*******4*******8******12******16******20******24******28******32 swapped"
#endif
	} ;
char	tdata_3[] {
#ifndef	K64
"\023\000\013\0\0\0\0\0\0\0\0******72******80******88******96*****104*****112*****120*****128"
#endif
#ifdef	K64
"\023\000\013\0\0\0\0\0\0\0\0******36******40******44******48******52******56******60******64"
#endif
	} ;
char	tdata_a[] { "\023\002\001\0\0\0\0\0\0\0\0machine is pdp 11/" } ;
char	tdata_b[] { "; clock is kw11-" } ;
char	tdatacadx[] { "\023xx\0\0\0\0\0\0\0\0" } ;
char	tdatatime[] { "\023\056\001\0\0\0\0\0\0\0\0" } ;
#endif

int	*_time ;
struct proc	*_proc ;
struct text	*_text ;
int	swapped, wereswapped ;

char	col[128] ;
/*
 * 0 blank column
 * 1 name in column
 * -1 out-of-date name in column (should be cleared)
 * 2 *text* in column
 * -2 possibly out-of-date *text* in column
 */
char	nameflag[128] ;

struct xproc {
	char	cent ;
	int	pid ;
	char	pname[8] ;
	} xproc[NPROC] ;

char	bigbuf[1000] ;

int	oldtty[3], newtty[3] ;

int	memfid ;
int	stbuf[256] ;
int	junks;

main()
{
	register int	i ;
	extern	trap() ;
	register char	*cp ;

	if (getuid() & ~0377) {
		write(2, "not super-user.\n", 16) ;
		exit(0) ;
	}
	junks = '#';
	setpsw(0140000) ;
	memfid = open("/dev/mem", 0) ;

	if ((i = findsyms()) < 0) exit(0) ;

	gtty(1, oldtty) ;
	signal(2, trap) ;
	signal(3, trap) ;
	newtty[0] = oldtty[0] ;
	newtty[1] = oldtty[1] ;
	newtty[2] = oldtty[2] & ~(020 | 077400 | 04 | 02 | 0100001) ;
	stty(1, newtty) ;

#ifndef	TDATA
	write(1, vt05_1, sizeof vt05_1 - 1) ;
	write(1, vt05_2, sizeof vt05_2 - 1) ;
	write(1, vt05_3, sizeof vt05_3 - 1) ;
#endif
#ifdef	TDATA
	write(1, tdata_1, sizeof tdata_1 - 1) ;
	write(1, tdata_2, sizeof tdata_2 - 1) ;
	write(1, tdata_3, sizeof tdata_3 - 1) ;
#endif
	write(1, bigbuf, i) ;

	for (;;) {
		zmemget(_time, 2, &time) ;
		cp = bigbuf ;
#ifndef	TDATA
		cp = mvchars(vt05time, sizeof vt05time - 1, cp) ;
#endif
#ifdef	TDATA
		cp = mvchars(tdatatime, sizeof tdatatime - 1, cp) ;
#endif
		cp = mvchars(ctime(time), 24, cp) ;
		write(1, bigbuf, cp - bigbuf) ;

		for (i = maxmem ; i < 128 ; i++) {
			nameflag[i] = -nameflag[i] ;
			col[i] = ' ' ;
		}

		i = setpsw(0140040) ; /* goto br1 */
		zmemget(_proc, sizeof proc / 2, proc) ;
		zmemget(_text, sizeof text / 2, text) ;
		setpsw(i) ;

		doprocs() ;
		dotexts() ;
		dumpdispl() ;

		sleep(2) ;
	}
}

doprocs()
{
	register struct proc	*pp ;
	register struct xproc	*xpp ;
	register char	*cp ;
	int	i ;
	int	cnt ;
	char	buf[8] ;
	int	i1, i2 ;

	wereswapped = swapped ;
	swapped = 0 ;

	cp = bigbuf ;

	pp = proc ; xpp = xproc ; cnt = NPROC ; do {
		if (pp->p_stat != NULL && pp->p_pid) {
			if (pp->p_pid != xpp->pid) {
				xpp->pid = pp->p_pid ;
				xpp->cent = -1 ;
				xpp->pname[0] = 0 ;
			}
			if (pp->p_flag & SLOAD) {
#ifndef	K64
				i1 = (pp->p_addr + 16) >> 5 ;
				i2 = (pp->p_addr + pp->p_size) >> 5 ;
#endif
#ifdef	K64
				i1 = (pp->p_addr + 8) >> 4 ;
				i2 = (pp->p_addr + pp->p_size) >> 4 ;
#endif
				if ((i2 - i1) < 2) {
					i = i1 ;
					col[i] = '*' ;
				} else {
					col[i1] = '<' ;
					col[i2] = '>' ;
					i = (i2 + i1) / 2 ;
					while (--i2 != i1) col[i2] = '-' ;
				}
				if (xpp->cent != i) {
					xpp->cent = i ;
/*
					if (xpp->pname[0]) {
						cp = namecol(i, xpp->pname, cp) ;
					} else {
						mvnum(xpp->pid, buf) ;
						cp = namecol(i, buf, cp) ;
					}
*/
					if (!xpp->pname[0]) prcom(pp, xpp) ;
					cp = namecol(i, xpp->pname, cp) ;
				}
				nameflag[i] = 1 ;
			} else {
				xpp->cent = -1 ;
#ifndef	TDATA
				*cp++ = '\016' ;
				*cp++ = '\043' + swapped++ ;
				cp = mvchars("\0\0\0\0\141\036", 6, cp) ;
#endif
#ifdef	TDATA
				*cp++ = '\023' ;
				*cp++ = '\101' ;
				*cp++ = '\003' + swapped++ ;
				cp = mvchars("\0\0\0\0\037", 5, cp) ;
#endif
				if (xpp->pname[0]) {
					cp = mvstring(xpp->pname, cp) ;
				} else {
					cp = mvnum(xpp->pid, cp) ;
				}
			}
		}
		pp++ ; xpp++ ;
	} while (--cnt) ;

	write(1, bigbuf, cp - bigbuf) ;
}

dotexts()
{
	register struct text	*tp ;
	register char	*cp ;
	int	cnt ;
	register int	i ;
	int	i1, i2 ;

	cp = bigbuf ;

	tp = text ; cnt = NTEXT ; do {
		if (tp->x_ccount) {
#ifndef	K64
			i1 = (tp->x_caddr + 16) >> 5 ;
			i2 = (tp->x_caddr + tp->x_size) >> 5 ;
#endif
#ifdef	K64
			i1 = (tp->x_caddr + 8) >> 4 ;
			i2 = (tp->x_caddr + tp->x_size) >> 4 ;
#endif
			if ((i2 - i1) < 2) {
				i = i1 ;
				col[i] = '*' ;
			} else {
				col[i1] = '<' ;
				col[i2] = '>' ;
				i = (i1 + i2) / 2 ;
				while (--i2 != i1) col[i2] = '-' ;
			}
			if (nameflag[i] <= 0) {
				if (nameflag[i] == -2) {
					nameflag[i] = -nameflag[i] ;
				} else {
#ifndef	TDATA
					cp = namecol(i, "#TEXT#", cp) ;
#endif
#ifdef	TDATA
					cp = namecol(i, "#text#", cp) ;
#endif
					nameflag[i] = 2 ;
				}
			}
		}
	} while (--cnt) ;

	write(1, bigbuf, cp - bigbuf) ;
}

dumpdispl()
{
	register int	i ;
	register char	*cp, *xp ;

	cp = bigbuf ;

	xp = col ;
*cp++ = ' ' ; *cp++ = ' ' ;

#ifndef	TDATA
	cp = mvchars("\016\043\0\0\0\0\040", 7, cp) ;
#endif
#ifdef	TDATA
	cp = mvchars("\023\000\003\0\0\0\0", 7, cp) ;
#endif
	i = 64 ; do {
		*cp++ = *xp++ ;
	} while (--i) ;

#ifndef	TDATA
	cp = mvchars("\016\054\0\0\0\0\040", 7, cp) ;
#endif
#ifdef	TDATA
	cp = mvchars("\023\000\014\0\0\0\0", 7, cp) ;
#endif
	i = 64 ; do {
		*cp++ = *xp++ ;
	} while (--i) ;

	if (swapped < wereswapped) {
#ifndef	TDATA
		*cp++ = '\016' ;
		*cp++ = '\043' + swapped ;
		cp = mvchars("\0\0\0\0\141\036", 6, cp) ;
#endif
#ifdef	TDATA
		*cp++ = '\023' ;
		*cp++ = '\101' ;
		*cp++ = '\003' + swapped ;
		cp = mvchars("\0\0\0\0\037", 5, cp) ;
#endif
		for (i = wereswapped - swapped ; --i ;) {
#ifndef	TDATA
			cp = mvchars("\013\0\0\0\0\036", 6, cp) ;
#endif
#ifdef	TDATA
			cp = mvchars("\013\0\0\0\0\037", 6, cp) ;
#endif
		}
	}

	for (i = maxmem ; i < 128 ; i++) {
		if (nameflag[i] < 0) cp = clrcol(i, cp) ;
	}

	write(1, bigbuf, cp - bigbuf) ;
}

trap()
{
	stty(1, oldtty) ;
	exit(0) ;
}

mvnum(an, str)
int	an ;
char	str[] ;
{
	register char	*p ;
	register int	n ;
	char	outvec[8] ;

	n = an ;
	p = &outvec[8] ;
	*--p = '\0' ;
	do {
		*--p = n % 10 + '0' ;
	} while (n =/ 10) ;
	return(mvstring(p, str)) ;
}

mvstring(str1, str2)
char	str1[], str2[] ;
{
	register char	*p1, *p2 ;

	p2 = str2 ;
	for (p1 = str1 ; *p2++ = *p1++ ;) ;
	return(--p2) ;
}

mvchars(str1, cnt, str2)
int	cnt ;
char	*str1, *str2 ;
{
	register int	i ;
	register char	*cp, *cp1 ;

	cp = str2 ;
	cp1 = str1 ;
	if (i = cnt) do {
		*cp++ = *cp1++ ;
	} while (--i) ;

	return(cp) ;
}

clrcol(ai, acp)
int	ai ;
char	*acp ;
{
	register int	i ;
	register char	*cp ;

	cp = acp ;
	i = ai ;
	nameflag[i] = 0 ;
#ifndef	TDATA
	vt05cadx[1] = i < 64 ? '\044' : '\055' ;
	vt05cadx[6] = '\040' + (i & 077) ;
	cp = mvchars(vt05cadx, 7, cp) ;
#endif
#ifdef	TDATA
	tdatacadx[1] = '\000' + (i & 077) ;
	tdatacadx[2] = i < 64 ? '\004' : '\015' ;
	cp = mvchars(tdatacadx, 11, cp) ;
#endif

	i = 6 ; do {
		cp = mvchars(" \b\013\0\0\0\0", 7, cp) ;
	} while (--i) ;
	*cp++ = ' ' ;
	return(cp) ;
}

namecol(ai, aname, acp)
int	ai ;
char	*acp, *aname ;
{
	register char	*cp, *namep ;
	register int	i ;

	cp = acp ;
	i = ai ;
	if (nameflag[i]) cp = clrcol(i, cp) ;
#ifndef	TDATA
	vt05cadx[1] = i < 64 ? '\044' : '\055' ;
	vt05cadx[6] = '\040' + (i & 077) ;
	cp = mvchars(vt05cadx, 7, cp) ;
#endif
#ifdef	TDATA
	tdatacadx[1] = '\000' + (i & 077) ;
	tdatacadx[2] = i < 64 ? '\004' : '\015' ;
	cp = mvchars(tdatacadx, 11, cp) ;
#endif
	for (namep = aname ; *cp++ = *namep++ ;) {
		cp = mvchars("\b\013\0\0\0\0", 6, cp) ;
	}
	cp =- 7 ; /* account for "\b\013\0\0\0\0\0" */
	return(cp) ;
}

prcom(app, axp)
struct proc	*app ;
struct xproc	*axp ;
{
	register struct proc	*pp ;
	register char	*cp, *cp1 ;
	int	baddr, laddr ;

	pp = app ;
	laddr = pp->p_addr + pp->p_size - 8 ;
	baddr = laddr >> 3 ;
	laddr = (laddr & 07) << 6 ;
	seek(memfid, baddr, 3) ;	/* displacement with memory */
	seek(memfid, laddr, 1) ;	/* displacement with process */
	if (read(memfid, stbuf, 512) != 512) return ;

	for (cp = &stbuf[256] ; cp > stbuf ;) {
		if ((cp =- 2)->integ == -1) {
			cp =+ 2 ;
			if (*cp == 0) cp++ ;
			for (cp1 = axp->pname ; *cp1++ = *cp++ ;) {
				if (cp == &stbuf[256] || cp1 == &axp->pname[7]) {
					*cp1 = '\0' ;
					break ;
				}
			}
			return ;
		}
	}
}

struct nl {
	int	name[4] ;
	int	type ;
	int	value ;
	} nl[] {
	'_p', 'ro', 'c\0', '\0\0',	0,	0,	/* 0 */
	'_c', 'pu', 'ty', 'pe',		0,	0,	/* 1 */
	'_l', 'ks', '\0\0', '\0\0',	0,	0,	/* 2 */
	'_t', 'im', 'e\0', '\0\0',	0,	0,	/* 3 */
	'_e', 'nd', '\0\0', '\0\0',	0,	0,	/* 4 */
	'_t', 'ex', 't\0', '\0\0',	0,	0,	/* 5 */
	'\0\0', '\0\0', '\0\0', '\0\0',	0,	0,
	'\0\0', '\0\0', '\0\0', '\0\0',	0,	0,
	0
	} ;

findsyms()
{
	register char	*cp ;
	register struct nl	*np ;
	register int	i ;

	nlist("/unix", nl) ;
	for (np = nl ; np->name[0] ; np++) {
		if (np->type == -1) {
			write(2, "cannot access /unix file\n", 25) ;
			return(-1) ;
		}
	}

	/* get cputype */
	cputype = memget(nl[1].value) ;
	cp = bigbuf ;
#ifndef	TDATA
	cp = mvchars(vt05_a, sizeof vt05_a - 1, cp) ;
#endif
#ifdef	TDATA
	cp = mvchars(tdata_a, sizeof tdata_a - 1, cp) ;
#endif
	cp = mvnum(cputype, cp) ;

	/* get clock info */
	_time = nl[3].value ;
	lks = memget(nl[2].value) ;
#ifndef	TDATA
	cp = mvchars(vt05_b, sizeof vt05_b - 1, cp) ;
#endif
#ifdef	TDATA
	cp = mvchars(tdata_b, sizeof tdata_b - 1, cp) ;
#endif
#ifndef	TDATA
	*cp++ = lks == (0177546 >> 1) ? 'L' : 'P' ;
#endif
#ifdef	TDATA
	*cp++ = lks == (0177546 >> 1) ? 'l' : 'p' ;
#endif

	/* get proc table */
	_proc = nl[0].value ;

	/* and text table */
	_text = nl[5].value ;

	/* size of unix */

	/* _end == nl[4].value */
#ifndef	K64
	i = ldiv(0, nl[4].value, 2048) ;
#endif
#ifdef	K64
	i = ldiv(0, nl[4].value, 1024) ;
#endif
	maxmem = i + 1 ;
	col[0] = '<' ; col[i] = '>' ;
#ifndef	TDATA
	cp = namecol(i / 2, "*UNIX*", cp) ;
#endif
#ifdef	TDATA
	cp = namecol(i / 2, "*unix*", cp) ;
#endif
	while (--i) col[i] = '-' ;

	return(cp - bigbuf) ;
}

