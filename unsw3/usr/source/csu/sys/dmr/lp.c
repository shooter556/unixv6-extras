#

/*
 *	copyright 1976 ian johnstone
 *
 *	copyright 1976 dave horsfall
 *
 *	-->	this line printer driver will handle cdc printers
 *		in the best most efficient way.
 *	-->	no upper to lower case conversion.
 *	-->	special cdc printer skips used.
 *	-->	recognize zero,blank suppression.
 *	-->	support infinite size pages.
 *	-->	support 013 as skip to channel four.
 *******
 *	-->	will talk to printers of various characteristics.
 *	-->	will drive plotters as well.
 *	-->	will operate from compressed plot files.
 *	-->	kronos eor/eof taken as form feeds.
 *	-->	stty calls supported to change modes.
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../proc.h"
#include "../buf.h"

/* some parameters */
#define	LPPRI	10
#define	BMAX	3		/* maximum number of buffers used per device */

int	lpages;		/* pages printed since last boot (in toto) */

struct	lpdev {
	int	lpsr;
	int	lpbuf;
	};

struct lpst {
	struct devtab	dtab;	/* buffer pointer */
	struct lpdev	*lpaddr;/* device register address */
	int	flag;		/* control flags */
	int	ejline;		/* lines per page (settable) */
	int	ejline1;	/* initial lines per page */
	int	maxcol;		/* max cols per line (settable) */
	int	mcc;
	int	ccc;		/* character count */
	int	mlc;		/* linecount */
	int	skp;		/* used to accumlate linefeeds */
	int	bfree;		/* number of buffers that are left to go */
	int	bc;		/* byte count (-ve) */
	char	*ba;		/* pointer of where up to */
	char	fc;		/* if non-zero it is char to be duplicated */
	int	(*lpspl)();	/* spl priority (4 or 5) */
	};

int	spl4(), spl5();

/* some status and mode bits */
#define	OPEN	0000001		/* device open */
#define	CLOSIN	0000002		/* device closing */
#define	TRNS	0000004		/* if half ascii */
#define	SKIP	0000010		/* if CDC printer skips used */
#define	THROW	0000020		/* throw page on close */
#define	PACK	0000040		/* if packed mode */
#define	PLTTR	0000100		/* if a plotter */
#define	FEED	0000200		/* if always take ff's, lf's */
#define	NOEJECT	0000400		/* if disable auto skip */
#define	ESCAPE	0001000		/* null byte escape found */
#define	NULLS	0002000		/* if still expanding nulls */
#define	SPFUNC	0004000		/* special function found */

#define	NLP	3

#define	XTRANS
#define	XSKIP
#define	XPLOT
#define	XPACK

/* define characteristics of devices */
#define	LVF0	0		/* versatec in print mode */
struct lpst	lp0 { 0,0,0,0,0,0,0177514,LVF0,60,60,132,0,0,0,0,BMAX,0,0,0,spl4 };
#define	LVF1	(TRNS | SKIP)	/* cdc printer */
struct lpst	lp1 { 0,0,0,0,0,0,0160024,LVF1,60,60,136,0,0,0,0,BMAX,0,0,0,spl4 };
#define	LVF2	PLTTR		/* xy plotter */
struct lpst	lp2 { 0,0,0,0,0,0,0172554,LVF2, 0, 0,  0,0,0,0,0,BMAX,0,0,0,spl5 };

struct lpst	*lp11[NLP] {
	&lp0,		/* versatec */
	&lp1,		/* cdc printer */
	&lp2,		/* calcomp xy */
	};

/* status register bits */
#define	IENABLE	0100
#define	DONE	0200

#ifdef	XPLOT
/* plotter control bits */
#define	BCLEAR	020
#define	REOT	010
#define	RFF	004
#define	LTERM	002
#define	PLOT	001
#endif	XPLOT

/* mask for mode bits */
#ifdef	XPLOT
#ifdef	XPACK
#ifdef	XTRANS
#define	MODES	(NOEJECT|FEED|PLTTR|PACK|THROW|SKIP|TRNS)
#else	XTRANS
#define	MODES	(NOEJECT|FEED|PLTTR|PACK|THROW|SKIP)
#endif	XTRANS
#else	XPACK
#ifdef	XTRANS
#define	MODES	(NOEJECT|FEED|PLTTR|THROW|SKIP|TRNS)
#else	XTRANS
#define	MODES	(NOEJECT|FEED|PLTTR|THROW|SKIP)
#endif	XTRANS
#endif	XPACK
#else	XPLOT
#ifdef	XTRANS
#define	MODES	(NOEJECT|FEED|THROW|SKIP|TRNS)
#else	XTRANS
#define	MODES	(NOEJECT|FEED|THROW|SKIP)
#endif	XTRANS
#endif	XPLOT

#ifdef	XPLOT
/* some special function codes - plot mode */
#define	EOL	012		/* end of line = newline */
#define	EOT	004		/* end of xmission = eot */
#define	FORM	014		/* remote form feed */
#endif	XPLOT

lpopen(dev, flag)
{
	register struct lpst *plp;

	plp = lp11[dev.d_minor & 07];
	if ((dev.d_minor & 07)>=NLP || plp->flag&OPEN || !flag || (plp->lpaddr->lpsr < 0)) {
		u.u_error = EIO;
		return;
	}
#ifdef	XPLOT
	if (!(dev.d_minor & 010)) {
#endif	XPLOT
		/* print mode */
		plp->flag =| OPEN;
		plp->lpaddr->lpsr =| IENABLE;
		plp->ccc = plp->mlc = plp->mcc = 0;
		plp->lpaddr->lpbuf = (plp->flag & THROW) ? '\f' : '\r';
#ifdef	XPLOT
	} else {
		/* plot mode */
		plp->flag =| OPEN|PLTTR;
		plp->lpaddr->lpsr =| IENABLE|REOT|RFF|PLOT;
	}
#endif	XPLOT
}

lpclose(dev, flag)
{
	register struct lpst *plp;

	plp = lp11[dev.d_minor & 07];
	plp->flag =| CLOSIN;
	while (plp->bfree != BMAX)
		sleep(plp, LPPRI);
#ifdef	XPLOT
	if (!(dev.d_minor & 010)) {
#endif	XPLOT
		/* print mode */
		plp->lpaddr->lpsr =& ~IENABLE;
		if (plp->mlc!=0 || plp->mcc!=0)
			plp->lpaddr->lpbuf = (plp->flag & THROW) ? '\f' : '\r';
		plp->ejline = plp->ejline1;
#ifdef	XPLOT
	} else {
		/* plot mode */
		plp->lpaddr->lpsr =| REOT|BCLEAR;
		plp->lpaddr->lpsr =& ~(IENABLE | PLOT);
	}
#endif	XPLOT
	plp->flag =& ~(OPEN | CLOSIN | PLTTR);
}

lpwrite(dev)
{
	register struct buf *bp;
	register struct lpst *plp;
	register int n;

	plp = lp11[dev.d_minor & 07];
	(*plp->lpspl)();
	while (u.u_count!=0 && u.u_error==0) {
		while (plp->bfree == 0)
			sleep(plp, LPPRI);
		plp->bfree--;
		bp = getblk(NODEV);
		bp->av_forw = NULL;
		(*plp->lpspl)();
		if ((n = u.u_count) > 512)
			n = 512;
		bp->b_wcount = -n;
		iomove(bp, 0, n, B_WRITE);
		if (plp->dtab.d_actf == NULL) {
			plp->dtab.d_actf = plp->dtab.d_actl = bp;
			plp->ba = bp->b_addr;
			plp->bc = bp->b_wcount;
			lpint(dev);
		} else {
			plp->dtab.d_actl->av_forw = bp;
			plp->dtab.d_actl = bp;
		}
	}
	spl0();
}

lpint(dev)
{
	register struct lpst *plp;
	struct buf *bp;
	register int *lpa;
	register  c;

	plp = lp11[dev.d_minor & 07];

	if ( (bp = plp->dtab.d_actf) == NULL)
		goto fin;

	if (plp->lpaddr->lpsr < 0) {	/* error */
		timeout(&lpint, dev, 2*HZ);
		return;
	}
	lpa = &plp->lpaddr->lpbuf;
#ifdef	XPACK
	if (plp->flag & NULLS) {	/* still expanding nulls */
		c = plp->fc & 0377;	/* saved residual null count */
		plp->flag =& ~NULLS;	/* clear flag */
		goto here;		/* jump into a "while" !!! */
	}
#endif	XPACK
loop:
	while ((plp->lpaddr->lpsr & DONE) && (plp->bc++ < 0)) {
		c = *plp->ba++;
#ifdef	XPLOT
		if (plp->flag & PLTTR) {	/* a plotter */
#ifdef	XPACK
			if (!(plp->flag & PACK)) {	/* not compressed */
#endif	XPACK
				*lpa = c;
				continue;
#ifdef	XPACK
			}

			/* compressed mode */
			if (plp->flag & ESCAPE) {	/* zero byte found last time */
				plp->flag =& ~ESCAPE;
				if ((c =& 0377) != 0) {	/* null count */
here:
					while ((plp->lpaddr->lpsr & DONE) && (c--)!=0)
						*lpa = 0;

					if (c > 0) {	/* nulls remaining */
						plp->flag =| NULLS;
						plp->fc = c;
						return;
					}
					continue;
				}

				/* a special function coming up */
				plp->flag =| SPFUNC;
				continue;
			}

			if (plp->flag & SPFUNC) {	/* special func */
				plp->flag =& ~SPFUNC;
				switch (c) {

				case EOL:	/* end of plot line */
					plp->lpaddr->lpsr =| LTERM;
					continue;

				case EOT:	/* end of transmission */
					plp->lpaddr->lpsr =| REOT;
					continue;

				case FORM:	/* form feed */
					plp->lpaddr->lpsr =| RFF;
					continue;

				default:	/* ??? */
					continue;
				}
			}
			if (c == 0)
				plp->flag =| ESCAPE;
			else
				*lpa = c;
			continue;
		}
#endif	XPACK
#endif	XPLOT
		if (plp->fc != 0) {
			c =& 0377;
			plp->ccc =+ --c;
			if (plp->fc == '0') {
				if (plp->ccc > plp->maxcol)
					c =- plp->ccc - plp->maxcol;
				while (c-- >= 0)
					*lpa = '0';
			}
			plp->fc = 0;
			continue;
		}
#ifdef	XTRANS
		if (plp->flag & TRNS)
			switch (c) {

			case '{':
				c = '(';
				goto esc;

			case '}':
				c = ')';
				goto esc;

			case '`':
				c = '\'';
				goto esc;

			case '|':
				c = '!';
				goto esc;

			case '~':
				c = '^';
				goto esc;

			esc:
				while (plp->ccc > plp->mcc) {
					*lpa = ' ';
					plp->mcc++;
				}
				*lpa = c;
				*(--plp->ba) = '-';
				--plp->bc;
				*lpa = '\r';
				plp->mcc = 0;
				continue;
			}
#endif	XTRANS
		switch (c) {

		case '\t':
			plp->ccc = (plp->ccc + 8) & ~7;
			break;

		case -1:
			plp->fc = c = ' ';
			goto dflt;

		case -2:
			plp->fc = c = '0';
			goto dflt;

		case -3:
			plp->ejline = 32767;
			break;

		case '\013':	/* pass thru 'vert.tab' char */
			if (plp->flag & SKIP) {
				*lpa = 0100103;
				plp->mlc = plp->ccc = plp->mcc = 0;
			}
			break;

		case '\006':	/* kronos eof */
		case '\022':	/* kronos eor */
			c = '\f';
		case '\f':
		case '\n':
			if (plp->flag&FEED || plp->mlc!=0 || plp->mcc!=0) {
				plp->mcc = 0;
				plp->mlc++;
				if (plp->mlc>=plp->ejline && !(plp->flag & NOEJECT))
					c = '\f';
				*lpa = c;
				if (c == '\f') { 
				    	plp->mlc = 0;
					lpages++; 
				}
			}
		case '\r':
			plp->ccc = 0;
			break;

		case '\b':
			if (plp->ccc > 0)
				plp->ccc--;
			break;

		case ' ':
			plp->ccc++;
			break;

		default:
			if (c < 040)
				break;
		dflt:
			if (plp->ccc < plp->mcc) {
				*lpa = '\r';
				plp->mcc = plp->fc = 0;
				--plp->ba;
				--plp->bc;
				break;
			}
			if (plp->ccc < plp->maxcol) {
				while (plp->ccc > plp->mcc) {
					*lpa = ' ';
					plp->mcc++;
				}
				*lpa = c;
				plp->mcc++;
			}
			plp->ccc++;
			break;
		}
	}
	if (plp->bc >= 0) {
		plp->dtab.d_actf = (bp = plp->dtab.d_actf)->av_forw;
		brelse(bp);
		if (plp->bfree++ == 0)
			wakeup(plp);
		if ((bp = plp->dtab.d_actf) != NULL) {
			plp->ba = bp->b_addr;
			plp->bc = bp->b_wcount;
			goto loop;
		}
	fin:
		if ((plp->lpaddr->lpsr & DONE) && (plp->flag & CLOSIN))
			wakeup(plp);
	}
	plp->lpaddr->lpsr =| IENABLE;	/* hardware botch */
}

lpsgtty(dev, v)
register int *v;
{
	register struct lpst *plp;

	plp = lp11[dev.d_minor & 07];

	if (v != NULL) {
		/* get */
		*v++ = plp->flag & MODES;
		*v++ = plp->ejline1;
		*v++ = plp->maxcol;
		return(1);
	} else {
		/* set */
		v = u.u_arg;
		 /* don't trample status bits */
		plp->flag = (*v++ & MODES) | (plp->flag & ~MODES);
		plp->ejline = plp->ejline1 = *v++;
		plp->maxcol = *v++;
		return(0);
	}
}
