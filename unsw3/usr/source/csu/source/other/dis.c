#

/*
 * A memory contents display program.
 * It should use a vt05 terminal's cursor control facilities.
 *
 * From the beautiful land of RSX-11D.
 *
 *	Craig McGregor
 *	Computing Services Unit
 *	UNSW
 */

#include "/usr/sys/param.h"
#include "/usr/sys/tty.h"

char	partab[1];	/* dummy for tty.h */

#ifndef	XVT05
#ifndef	XTDATA
#ifndef	GTVT05
#define	XVT05	/* default */
#endif	GTVT05
#endif	XTDATA
#endif	XVT05

#ifndef	K64
#ifndef	K96
#ifndef	K128
#define	K128	/* default */
#endif	K128
#endif	K96
#endif	K64

#ifdef	K64
#define	KSIZE	64
#endif	K64
#ifdef	K96
#define	KSIZE	96
#endif	K96
#ifdef	K128
#define	KSIZE	128
#endif	K128


#ifdef	XVT05
/*
 * vt05 (gt40 version) control characters
 *
 *  016	cursor addressing (040...) (y, x)
 *  035	home beam
 *  037	clear screen
 *  036	clear line
 *  032	cursor up
 *  013	cursor down
 *  010	cursor left
 *  030	cursor right
 *
 * screen size: 72 20
 */
#endif	XVT05
#ifdef	XTDATA
/*
 * transdata control characters
 *
 *  023	cursor addressing (000...) (x, y)
 *  031	swap pages
 *  035	home beam
 *  034	clear screen
 *  037 clear line
 *  017	cursor up
 *  013	cursor down
 *  010 cursor left
 *  011	cursor right
 *
 * screen size: 80 24
 */
#endif	XTDATA
#ifdef	GTVT05
/*
 * vt05 control characters
 *
 *  016	cursor addressing (040...) (y, x)
 *  035	home beam
 *  037	clear screen
 *  036	clear line
 *  032	cursor up
 *  013	cursor down
 *  010	cursor left
 *  030	cursor right
 *
 * screen size: 72 32
 */
#endif	GTVT05

/*
 * home beam and clear screen message
 */
char	home[] {
#ifdef	XVT05
	"\035\0\0\0\0\037\0\0\0\0"
#endif	XVT05
#ifdef	XTDATA
	"\035\0\0\0\034\0\0\0\0\0"
#endif	XTDATA
#ifdef	GTVT05
	"\035\037"
#endif	GTVT05
	};

#ifndef	XLCASE
char	heading[] { "********************* UNIX MEMORY CONTENTS DISPLAY *********************" };
#endif	XLCASE
#ifdef	XLCASE
char	heading[] { "********************* unix memory contents display *********************" };
#endif	XLCASE

char	msg1[] {
#ifdef	K128
	"*******8******16******24******32******40******48******56******64 swapped"
#endif	XLCASE
#ifdef	K96
	"*******6******12******18******24******30******36******42******48 swapped"
#endif	K96
#ifdef	K64
	"*******4*******8******12******16******20******24******28******32 swapped"
#endif	K64
	};

char	msg2[] {
#ifdef	K128
	"******72******80******88******96*****104*****112*****120*****128"
#endif	K128
#ifdef	K96
	"******54******60******66******72******78******84******90******96"
#endif	K96
#ifdef	K64
	"******36******40******44******48******52******56******60******64"
#endif	K64
	};

#ifndef	XLCASE
char	mchmsg[] { "Machine is PDP-11/" };
#endif	XLCASE
#ifdef	XLCASE
char	mchmsg[] { "machine is pdp-11/" };
#endif	XLCASE

#ifndef	XLCASE
char	clkmsg[] { "; Clock is KW11-" };
#endif	XLCASE
#ifdef	XLCASE
char	clkmsg[] { "; clock is kw11-" };
#endif	XLCASE

/*
 * standard cursor addressing string
 */
char	cadmsg[] {
#ifdef	XVT05
	"\016y\0\0\0\0x"
#define	CADX	6
#define	CADY	1
#define	CADX0	040
#define	CADY0	040
#endif	XVT05
#ifdef	XTDATA
	"\023\0\0x\0\0y\0\0\0"
#define	CADX	3
#define	CADY	6
/*
 * there is a bug in the transdata in that a 0 coordinate
 * value is taken as a fill character(!). as the screen
 * is so large, the oportunity is taken to centre the picture.
 */
#define	CADX0	004	/**** 000!! ****/
#define	CADY0	002	/**** 000!! ****/
#endif	XTDATA
#ifdef	GTVT05
	"\016yx"
#define	CADX	2
#define	CADY	1
#define	CADX0	040
#define	CADY0	040
#endif	GTVT05
	};

/*
 * move 1 column left and down 1 row
 *
 * used to print characters down a column
 */
char	lftdwn[] {
#ifdef	XVT05
	"\b\013\0\0\0\0"
#endif	XVT05
#ifdef	XTDATA
	"\b\0\0\013\0\0\0"
#endif	XTDATA
#ifdef	GTVT05
	"\b\013"
#endif	GTVT05
	};

/*
 * move down 1 column
 */
char	down[] {
#ifdef	XVT05
	"\013\0\0\0\0"
#endif	XVT05
#ifdef	XTDATA
	"\013\0\0\0\0"
#endif	XTDATA
#ifdef	GTVT05
	"\013"
#endif	GTVT05
	};

/*
 * clear from current position to end-of-line
 */
char	ceolmsg[] {
#ifdef	XVT05
	"\036"
#endif	XVT05
#ifdef	XTDATA
	"\037\0\0\0\0"
#endif	XTDATA
#ifdef	GTVT05
	"\036"
#endif	GTVT05
	};

long	now;
long	time();
int	maxmem;
int	swapped, wereswapped;

char	col[128];	/* line containing - ' ', '-', '<', '>' */

/*
 *  0 blank column
 *  1 name in column
 * -1 out-of-date name in column (should be cleared)
 *  2 #text# in column
 * -2 possibly out-of-date #text# in column
 */
char	nameflag[128];

struct xproc {
	char	cent;		/* column containing name */
	int	pid;		/* process id */
	char	pname[8];	/* name of process */
	} xproc[NPROC];

char	bigbuf[2000];	/**** main output buffer ****/
char	*bufp;

int	oldtty[4], newtty[4];

int	memfid;
int	stbuf[256];

char	*unixfile;

#define	ROUND	/* +(KSIZE/8) */

main(argc,argv)
int	argc;
char	*argv[];
{
	register int	i;
	register char	*p;
	extern	trap();

	unixfile = argc>1 ? argv[1] : "/unix";
	if ((memfid = open("/dev/mem", 0)) < 0) {
		write(2, "Can't open /dev/mem\n", 20);
		return(1);
	}

	bufp = bigbuf;
	mvchars(home, sizeof home - 1);
#ifdef	XTDATA	/* !!! */
	cad(0, 0);	/* !!! */
#endif	XTDATA	/* !!! */
	mvchars(heading, sizeof heading - 1);

	if (findsyms() < 0)
		return(1);

	gtty(1, oldtty);
	signal(SIGINT, &trap);
	signal(SIGQIT, &trap);
	signal(SIGTERM, &trap);
	newtty[0] = oldtty[0];
	newtty[1] = oldtty[1];
	newtty[2] = oldtty[2] & ~(CRMOD|ECHO|XTABS|LCASE|DELAYS);
	newtty[3] = oldtty[3];
	stty(1, newtty);

	cad(0, 2);
	mvchars(msg1, sizeof msg1 - 1);
	cad(0, 11);
	mvchars(msg2, sizeof msg2 - 1);
	write(1, bigbuf, bufp - bigbuf);

	nice(-50);

	for (;;) {
		bufp = bigbuf;
		now = time();
		cad(46, 1);
#ifndef	XLCASE
		mvchars(ctime(now), 24);
#endif	XLCASE
#ifdef	XLCASE
		p = ctime(now);
		p[0] =+ 'a'-'A';
		p[4] =+ 'a'-'A';
		mvchars(p, 24);
#endif	XLCASE
		write(1, bigbuf, bufp - bigbuf);

		for (i = maxmem; i < 128; i++) {
			nameflag[i] = -nameflag[i];
			col[i] = ' ';
		}

		dotexts();
		doprocs();
		dumpdispl();

		sleep(2);
	}
}

/*
 * abort dis nicely
 */
trap()
{
	stty(1, oldtty);
	exit(0);
}

/*
 * display locations of program segments
 */
doprocs()
{
	register struct proc	*pp;
	register struct xproc	*xpp;
	register int	i;
	int	cnt;
	int	i1, i2;
#include "/usr/sys/proc.h"

	wereswapped = swapped;
	swapped = 0;	/* swapped out list size */

	bufp = bigbuf;

	gettable(0, proc, sizeof proc);

	pp = proc;
	xpp = xproc;
	cnt = NPROC;
	/* start at proc[1]? */
	do {
		if (pp->p_stat!=0 && pp->p_stat!=SZOMB && pp->p_pid) {
			/* can't catch scheduler */
			if (pp->p_pid != xpp->pid) {
				xpp->pid = pp->p_pid;
				xpp->cent = -1;
				xpp->pname[0] = '\0';
			}
			if (pp->p_flag & SLOAD) {
				i1 = (pp->p_addr ROUND) * 4 / KSIZE;
				i2 = (pp->p_addr + pp->p_size) * 4 / KSIZE;
				if (i1 == i2) {
					i = i1;
#ifndef	XLCASE
					col[i] = 'X';
#endif	XLCASE
#ifdef	XLCASE
					col[i] = 'x';
#endif	XLCASE
				} else {
					col[i1] = '<';
					col[i2] = '>';
					i = (i2 + i1) / 2;
					while (--i2 != i1)
						col[i2] = '-';
				}
				if (prcmd(pp->p_addr+pp->p_size, xpp) || xpp->cent!=i) {
					xpp->cent = i;
					clrcol(i);
					setcol(i);
					if (xpp->pname[0] != '\0')
						mvname(xpp->pname, 1);
					else
						mvnum(xpp->pid, 1);
				}
				nameflag[i] = 1;
			} else {
				xpp->cent = -1;
				if (++swapped < 17) {
					cad(65, 2 + swapped);
					if (xpp->pname[0] != '\0')
						mvname(xpp->pname, 0);
					else
						mvnum(xpp->pid, 0);
					mvchars(ceolmsg, sizeof ceolmsg - 1);
				}
			}
		}
		pp++;
		xpp++;
	} while (--cnt != 0);

	write(1, bigbuf, bufp - bigbuf);
}

/*
 * display locations of shared texts in memory
 */
dotexts()
{
	register struct text	*xp;
	int	cnt;
	register int	i, i2;
	int	i1;
#include "/usr/sys/text.h"

	bufp = bigbuf;

	gettable(1, text, sizeof text);

	xp = text;
	cnt = NTEXT;
	do {
		if (xp->x_ccount != 0) {
			i1 = (xp->x_caddr ROUND) * 4 / KSIZE;
			i2 = (xp->x_caddr + xp->x_size) * 4 / KSIZE;
			if (i1 == i2) {
				i = i1;
				col[i] = 'X';
			} else {
				col[i1] = '<';
				col[i2] = '>';
				i = (i1 + i2) / 2;
				while (--i2 != i1)
					col[i2] = '-';
			}
			if (nameflag[i] <= 0)
				if (nameflag[i] == -2)
					nameflag[i] = -nameflag[i];
				else {
					clrcol(i);
					setcol(i);
#ifndef	XLCASE
					mvname("#TEXT#", 1);
#endif	XLCASE
#ifdef	XLCASE
					mvname("#text#", 1);
#endif	XLCASE
					nameflag[i] = 2;
				}
		}
		xp++;
	} while (--cnt != 0);

	write(1, bigbuf, bufp - bigbuf);
}

/*
 * dump page to display
 */
dumpdispl()
{
	register int	i;
	register char	*cp;

	bufp = bigbuf;

	cad(0, 3);
	for (i = 0; i < 64; i++)
		*bufp++ = col[i];
	cad(0, 12);
	for (; i < 128; i++)
		*bufp++ = col[i];

	if (swapped < wereswapped) {
		cad(65, 3 + swapped);
		mvchars(ceolmsg, sizeof ceolmsg - 1);
		for (i = wereswapped - swapped; --i != 0;) {
			mvchars(down, sizeof down - 1);
			mvchars(ceolmsg, sizeof ceolmsg - 1);
		}
	}

	for (i = maxmem; i < 128; i++)
		if (nameflag[i] < 0)
			clrcol(i);

	write(1, bigbuf, bufp - bigbuf);
}

/*
 * print out a number - either vertical or horizontal
 */
mvnum(n, vflag)
register int	n;
int	vflag;
{
	register char	*p;
	static char	outvec[6];	/* only need 5 digits */

	p = &outvec[6];
	*--p = '\0';
	do {
		*--p = n % 10 + '0';
	} while ((n =/ 10) != 0);

	if (!vflag)
		while ((*bufp = *p++) != '\0')
			bufp++;
	else
		for (;;) {
			*bufp++ = *p++;
			if (*p == '\0')
				break;
			mvchars(lftdwn, sizeof lftdwn - 1);
		}
}

/*
 * print out a name field - either vertical or horizontal
 */
mvname(p, vflag)
register char	*p;
int	vflag;
{
	register int	i;
#ifdef	XLCASE
	register int	c;
#endif	XLCASE

	i = 7;
	if (!vflag)
#ifndef	XLCASE
		while ((*bufp = *p++) != '\0') {
			bufp++;
#endif	XLCASE
#ifdef	XLCASE
		while ((c = *p++) != '\0') {
			if (c>='A' && c<='Z')
				c =+ 'a'-'A';
			*bufp++ = c;
#endif	XLCASE
			if (--i == 0)
				break;
		}
	else {
#ifndef	XLCASE
		if ((*bufp = *p++) != '\0')
#endif	XLCASE
#ifdef	XLCASE
		if ((c = *p++) != '\0')
#endif	XLCASE
			do {
#ifndef	XLCASE
				bufp++;
#endif	XLCASE
#ifdef	XLCASE
				if (c>='A' && c<='Z')
					c =+ 'a'-'A';
				*bufp++ = c;
#endif	XLCASE
				if (--i == 0)
					break;
				mvchars(lftdwn, sizeof lftdwn - 1);
#ifndef	XLCASE
			} while ((*bufp = *p++) != '\0');
#endif	XLCASE
#ifdef	XLCASE
			} while ((c = *p++) != '\0');
#endif	XLCASE
	}
}

/*
 * copy "cnt" characters into the string buffer
 */
mvchars(cp, cnt)
register char	*cp;
register int	cnt;
{
	do {
		*bufp++ = *cp++;
	} while (--cnt != 0);
}

/*
 * set cursor to top of specified column
 */
setcol(col)
register int	col;
{
	cad(col & 077, col < 64 ? 4 : 13);
}

/*
 * clear specified column if not already cleared
 */
clrcol(col)
register int	col;
{
	register int	i;

	if (nameflag[col] != 0) {
		nameflag[col] = 0;
		setcol(col);

		for (i = 7;;) {
			*bufp++ = ' ';
			if (--i == 0)
				break;
			mvchars(lftdwn, sizeof lftdwn - 1);
		}
	}
}

/*
 * get in-core process name
 */
prcmd(adr, axp)
unsigned adr;
struct xproc	*axp;
{
	register char	*cp, *cp1, *cpe;
	long	l;
	int	flag;
	struct {
		int	*integp;
		};

	l = adr - 8;
	l =<< 6;	/* 64 bytes/block */
	lseek(memfid, l, 0);
	if (read(memfid, stbuf, 512) == 512)
		for (cp = &stbuf[256]; cp > stbuf;)
			if (*--cp.integp == -1) {
				cp.integp++;
				if (*cp == '\0')
					cp++;
				cp1 = axp->pname;
				flag = *cp1=='\0';
				cpe = cp1 + 8;
				for (;;) {
					if (*cp1++ != *cp++) {
						flag++;
						if ((cp1[-1] = cp[-1]) == '\0')
							break;
					}
					if (cp1 == cpe)
						break;
					if (cp == &stbuf[256]) {
						*cp1++ = '\0';
						break;
					}
				}
				return(flag);
			}
	return(0);
}

struct nl {
	char	name[8];
	char	type;
	char	seg;
	unsigned value;
	} nl[] {
	{ "_cputype",	0,	0,	0 },	/* 0 */
	{ "_lks",	0,	0,	0 },	/* 1 */
	{ "_eu",	0,	0,	0 },	/* 2 */
	0
	};

/*
 * find interesting symbols
 */
findsyms()
{
	register char	*cp;
	register struct nl	*np;
	register int	i;

	nlist(unixfile, nl);
	for (np = nl; np->name[0] != '\0'; np++)
		if (np->type == -1) {
			write(2, "Cannot access unix file\n", 24);
			return(-1);
		}

	/* get cputype */
	cad(2, 1);
	mvchars(mchmsg, sizeof mchmsg - 1);
	mvnum(memget(nl[0].value), 0);

	/* get clock info */
	mvchars(clkmsg, sizeof clkmsg - 1);
#ifndef	XLCASE
	*bufp++ = (memget(nl[1].value) == 0177546) ? 'L' : 'P';
#endif	XLCASE
#ifdef	XLCASE
	*bufp++ = (memget(nl[1].value) == 0177546) ? 'l' : 'p';
#endif	XLCASE

	/* size of unix */

	/* _end == nl[2].value */
	/* _eu == (nl[2] + USIZE) * 0100 */
	i = (nl[2].value + USIZE) * 4 / KSIZE;
	maxmem = i + 1;
	col[0] = '<';
	col[i] = '>';
	setcol(i / 2);
	mvname("*UNIX*", 1);
	while (--i != 0)
		col[i] = '-';

	return(0);
}

/*
 * read a word from memory
 */
memget(adr)
int	adr;
{
	int buf;

	buf = -1;
	seek(memfid, adr, 0);
	read(memfid, &buf, 2);
	return(buf);
}

/*
 * handle cursor addressing
 */
cad(x, y)
int	x, y;
{
	cadmsg[CADX] = CADX0 + x;
	cadmsg[CADY] = CADY0 + y;
	mvchars(cadmsg, sizeof cadmsg - 1);
}
