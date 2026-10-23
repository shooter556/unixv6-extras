#
/*
 *	copyright 1975  ian inc.
 */
/*
 *	cr11 card reader driver
 *	ascii and binary version
 */

#include "../param.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

/*
 *	to add another card reader see notes
 * 	below with struct cr11
 */
#define	CRIPRI	20	/* sleeping priority */
#define	NCOLS	80	/* no. of columns    */
#ifdef	CRBUF
#define	SPLCR	spl4	/* hardware priority level */
#else	CRBUF
#define	SPLCR	spl7	/* hardware priority level */
#endif	CRBUF

/*
 *	card reader status bits
 *	see the dec peripherals handbook
 *	for more information
 */
#define	ERROR		(1<<15)
#define	CARDONE		(1<<14)
#define	HOPPER		(1<<13)		/* some machines only */
#define	MOTION		(1<<12)
#ifdef	CRBUF
#define	CBUFFER		(1<<11)
#else	CRBUF
#define	TIMING		(1<<11)
#endif	CRBUF
#define	ONLINE		(1<<10)
#define	BUSY		(1<<9)
#define	NOTREADY	(1<<8)
#define	COLDONE		(1<<7)
#define	IENABLE		(1<<6)
#ifdef	CRBUF
#define	CLEAR		(1<<1)
#else	CRBUF
#define	EJECT		(1<<1)
#endif	CRBUF
#define	READ		(1<<0)

/*
 *	card reader device registers
 *	the order of these fields is important
 */
struct {
	int	crstatus;	/* status register */
	int	crbbin;		/* 12 bit binary buffer */
	int	crbcode;	/* 8 bit encoded buffer */
};

/*
 *	card reader handler status
 */
#define	CLOSED		0
#define	READING		1
#define	CARDREAD	2
#define	ENDFILE		3
#define	VALERR		4
#ifndef	CRBUF
#define	TIMERR		5
#endif	CRBUF

/*
 *	card reader mode
 */
#define	O29	0
#define	O26	1
#define	BINARY	2

/*
 *	status info for card reader(s)
 */
struct cr11 {
	long	crdcnt;		/* double word card count */
	char	crstate;	/*
				 * current state of reader
				 *	0 - closed
				 *	1 - reading
				 *	2 - cardread
				 *	3 - endfile
				 */
	char	crmode;		/*
				 * 0 - 029
				 * 1 - 026
				 * 2 - binary
				 */
	int	crvalerr;	/* count of validity errors */
#ifndef	CRBUF
	int	crerror;	/* copy of status register on error */
#else	CRBUF
	int	crrdchk;	/* count of read checks */
#endif	CRBUF
	int	*craddr;	/* device register address */
	int	crchars;	/* columns not yet passed to user */
/************************************************************************
 *		this next word must be the 8th in the structure		*
 *		for the purposes of iomove				*
 *		(corresponds with b_addr)				*
 ************************************************************************/
	char	*b_addr;	/* buffer position pointer */
#ifndef	CRBUF
	int	*nextcol;	/* pointer to the last column read */
	int	crcols;		/* space remaining in buffer */
#endif	CRBUF
	int	buffer[NCOLS];	/* one card buffer */
};

#define	NCR11	2	/* no. of cr11 */
struct cr11	*cr11[];

#include <conf.cr>

/*
 *	To add another card reader -
 *   1. Increment NCR11 by 1.
 *   2.	Add another element to the array "cr11"
 *	and initialise the element to the address of
 *	a cr11 structure (similar to "cr0").
 *   3.	Add the cr11 struct complete with its
 *	initialisation.
 *
 *	If you can't follow the structure then
 *	perhaps you shouldn't be modifying it.
 */

/*
 *	special character codes
 */
#ifndef	CRBUF
#define	EOI1	01437	/*		  6 7 8 9 punching */
#define	EOI2	041777	/* 12 11 0 1	  6 7 8 9 punching */
#define	EOF	01427	/*		  6 7   9 punching */
#define	EOR	00437	/*		    7 8 9 punching */
#define	CNV	02427	/*		5   7   9 punching */
#else	CRBUF
#define	EOI1	017	/*		  6 7 8 9 punching */
#define	EOI2	07417	/* 12 11 0 1	  6 7 8 9 punching */
#define	EOF	015	/*		  6 7   9 punching */
#define	EOR	007	/*		    7 8 9 punching */
#define	CNV	025	/*		5   7   9 punching */
#endif	CRBUF

#define	CNTRLF	'\06'	/* eof returns cntrlf */
#define	CNTRLR	'\022'	/* eor returns cntrlr */

/*
 *	validity checking table
 */
#ifndef	CRBUF
char crvalid[8] {
	0, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1<<0,
};
#else	CRBUF
int crvalid[8] {
	0, 1<<8, 1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2,
};
#endif	CRBUF

char crcd26[] { "\0+=()'^#>?;<[]`\"&:!?^@%" };

char crcd29[] { "\0&#%<@);^(!{\"\\_>?]}[+'=" };

/*
 *	coded to ascii translation
 */
char crtab[256] {
	' ', '1', '2', '3', '4', '5', '6', '7',		/*			*/
	'8', '`', ':', -2 , -5 , -21, -22, -12,		/*		8	*/
	'9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	      9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,CNTRLR,	/*	      9 8	*/
	'0', '/', 's', 't', 'u', 'v', 'w', 'x',		/*	    0		*/
	'y', '^', -13, ',', -3 , -14, -15, -16,		/*	    0   8	*/
	'z',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	    0 9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	    0 9 8	*/
	'-', 'j', 'k', 'l', 'm', 'n', 'o', 'p',		/*	11		*/
	'q', '|', -17, '$', '*', -6 , -7 , -8 ,		/*	11      8	*/
	'r',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	11    9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	11    9 8	*/
	-18, '~', 'S', 'T', 'U', 'V', 'W', 'X',		/*	11  0		*/
	'Y',  0 ,  0 ,  0 ,  0 ,  0 ,'\t',  0 ,		/*	11  0   8	*/
	'Z',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	11  0 9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*	11  0 9 8	*/
	-1 , 'a', 'b', 'c', 'd', 'e', 'f', 'g',		/*   12			*/
	'h',  0 , -19, '.', -4 , -9 , -20, -10,		/*   12		8	*/
	'i',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12       9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12       9 8	*/
	-11, 'A', 'B', 'C', 'D', 'E', 'F', 'G',		/*   12     0		*/
	'H',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12     0   8	*/
	'I',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12     0 9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12     0 9 8	*/
	 0 , 'J', 'K', 'L', 'M', 'N', 'O', 'P',		/*   12 11		*/
	'Q',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11      8	*/
	'R',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11    9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11    9 8	*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11  0		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11  0   8	*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11  0 9		*/
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,		/*   12 11  0 9 8	*/
};

/*
 *	and now for something completely different
 *	the routines to handle all
 */

cropen(dev, flag)
int dev, flag;
{
	register struct cr11 *cr;

	if ((dev.d_minor & 07)>=NCR11 || flag) {
		u.u_error = ENXIO;
		return;
	}
	cr = cr11[dev.d_minor & 07];
	if (cr->crstate != CLOSED) {
		u.u_error = EEXIST;
		return;
	}
	cr->crmode = dev.d_minor&010 ? BINARY : O29;
#ifdef	CRBUF
	cr->crchars = 0;
#endif	CRBUF
/*	crstart(cr); */
	cr->crstate = ENDFILE;
}

crclose(dev, flag)
int dev, flag;
{
	register struct cr11 *cr;

	cr = cr11[dev.d_minor & 07];
	cr->craddr->crstatus = 0;
	cr->crstate = CLOSED;
}

crread(dev)
int dev;
{
	register struct cr11 *cr;
#ifdef	CRBUF
	register int	*devreg;
	register char	*pch;
	char	qch;
	struct {
		int	*integp;
		};
#endif	CRBUF

	cr = cr11[dev.d_minor & 07];
	if (cr->crchars == 0) {
		if (cr->crstate == ENDFILE)
			crstart(cr);
	loop:
		SPLCR();
		while (cr->crstate != CARDREAD)
			sleep(cr, CRIPRI);
		spl0();
		cr->crdcnt++;
#ifndef	CRBUF
		if (cr->crerror & TIMING) {
			crerror(dev, cr, TIMERR);
			goto loop;	/* next time put in a bigger card */
		}
		cr->crcols = NCOLS - cr->crcols;
		if (cr->crmode == BINARY)
			cr->crchars = cr->crcols*2;
		else {
			if (crascii(dev, cr))
				goto loop;	/* must have been an invalid card or conv */
		}
#else	CRBUF
		pch = cr->buffer;
		devreg = cr->craddr;
		do {
			if (cr->crmode == BINARY)
				*pch.integp++ = devreg->crbbin;
			else {
				/* ascii conversion - 1st validity check */
				if (crvalid[devreg->crbcode & 07] != (devreg->crbbin & 0774))
					switch (devreg->crbbin) {

					case EOI1:
					case EOI2:
						cr->crstate = ENDFILE;
						cr->crmode = O29;
						return;

					case EOF:
						*pch++ = CNTRLF;
						break;

					case CNV:
						if (pch == cr->buffer) {
							crrdcol(devreg);
							cr->crmode = devreg->crbbin==0 ? O26 : O29;
							crstart(cr);
							goto loop;
						}
						/* if not conversion, fall thru to default */
					default:
						crerror(dev, cr);
						goto loop;
					}
				else {
					/* valid - translate */
					qch = crtab[devreg->crbcode];

					/* check o26 or o29 specials */
					if (qch <= 0)
						if (qch == 0) {
							crerror(dev, cr);
							goto loop;
						} else
							*pch++ = (cr->crmode==O29 ? crcd29 : crcd26)[-qch];
					else
						*pch++ = qch;
				}
			}
		} while (pch<&cr->buffer[NCOLS] && crrdcol(devreg));
		if (cr->crmode != BINARY)
			*pch++ = '\n';
		cr->crchars = pch - cr->buffer;
#endif	CRBUF
		cr->b_addr = cr->buffer;
	}
	crpass(cr);
	if (cr->crstate!=ENDFILE && cr->crchars==0)
		crstart(cr);
}

crstart(cr)
register struct cr11 *cr;
{
	register int	*devreg;

#ifndef	CRBUF
	cr->nextcol = cr->buffer;
#endif	CRBUF
	cr->crstate = READING;
#ifndef	CRBUF
	cr->crerror = 0;
	cr->crcols = NCOLS;
	cr->crchars = 0;
#endif	CRBUF
	devreg = cr->craddr;
#ifdef	CRBUF
	devreg->crstatus = CLEAR;
#endif	CRBUF
	devreg->crstatus = IENABLE|READ;
}

#ifndef	CRBUF
crerror(dev, cr, err)
int dev;
register struct cr11 *cr;
int err;
{
	cr->crstate = err;
	if (err == VALERR)
		cr->crvalerr++;
	useron();
	printf("cr%d %s - reread last card\n", dev.d_minor&07,
		(err == TIMERR ? "length" : "validity"));
	useroff();
	cr->craddr->crstatus = IENABLE;
}
#else	CRBUF
crerror(dev, cr)
int dev;
register struct cr11 *cr;
{
	register int	*devreg;

	devreg = cr->craddr;
	cr->crstate = VALERR;
	cr->crvalerr++;
	useron();
	printf("cr%d validity - reread last card\n", dev.d_minor&07);
	useroff();
	devreg->crstatus = IENABLE|CLEAR;
}
#endif	CRBUF

#ifndef	CRBUF
crascii(dev, cr)
register struct cr11 *cr;
{
	register int *binp;
	register char *cp;

	for (binp = cp = cr->buffer; binp < cr->nextcol; binp++, cp++)
		/* at most one punching in columns 1-7 */
		if (crvalid[*binp & 07] != (*binp >> 8))
			switch (*binp) {

			case EOI1:
			case EOI2:
				cr->crstate = ENDFILE;
				cr->crmode = O29;
				return(0);

			case EOF:
				*binp = CNTRLF;
				break;

			case CNV:
				if (binp == cr->buffer) {
					cr->crmode = cr->buffer[1]==0 ? O26 : O29;
					crstart(cr);
					return(1);
				}

			default:
				crerror(dev, cr, VALERR);
				return(1);
			}
		else {
			/* translate character */
			*cp = crtab[*binp & 0377];

			/* check 026 029 and validity */
			if (*cp <= 0)
				if (*cp == 0) {
					crerror(dev, cr, VALERR);
					return(1);
				} else
					*cp = (cr->crmode ? crcd26 : crcd29)[-*cp];
		}
	*cp++ = '\n';
	cr->crchars = cr->crcols + 1;
	return(0);
}
#else	CRBUF
crrdcol(devreg)
register int	*devreg;
{
	int	x, y;

	devreg->crstatus = READ;	/* shift */
	/* wait a microsecond or 5 */
	x = y;	/* waste the required time */
	if (devreg->crstatus & CBUFFER)
		return(1);
	else
		return(0);
/*	return((devreg->crstatus & CBUFFER) != 0); */
}
#endif	CRBUF

crpass(cr)
register struct cr11 *cr;
{
	register int count1, count2;

	/* send back the min of cols on card and use count */
	if ((count1 = cr->crchars) > u.u_count)
		count1 = u.u_count;

	/* iomove an even number of characters */
	if ((count2 = count1 & ~1) > 0) {
		iomove(cr, 0, count2, B_READ);
		cr->crchars =- count2;
		cr->b_addr =+ count2;
	}
	/* passc the last character if odd */
	if (count1 & 1) {
		passc(*cr->b_addr++);
		cr->crchars--;
	}
}

crint(dev)
int dev;
{
	register struct cr11 *cr;
	register int *devreg;
	register int status;

	cr = cr11[dev.d_minor & 07];
	devreg = cr->craddr;
	status = devreg->crstatus;
#ifndef	CRBUF
	if (status & COLDONE)
		if (cr->crcols != 0) {
			cr->crcols--;
			*cr->nextcol++ = (cr->crmode == BINARY ?
					devreg->crbbin :
					devreg->crbcode | ((devreg->crbbin & 0774) << 6));
		}
#endif	CRBUF
	if (status & (ERROR|CARDONE|ONLINE)) {
#ifndef	CRBUF
		if (status & ERROR) {
			devreg->crstatus = EJECT|IENABLE;
			if (status & TIMING)
				cr->crerror = status;
		}
		if (status&CARDONE && !(status & MOTION) && cr->crcols!=NCOLS) {
#else	CRBUF
		devreg->crstatus = IENABLE;	/* clear interrupt */
		if (status & MOTION)
			cr->crrdchk++;
		else if ((status & (CARDONE|CBUFFER)) == (CARDONE|CBUFFER)) {
#endif	CRBUF
			devreg->crstatus = 0;
			cr->crstate = CARDREAD;
			wakeup(cr);
		} else if (status & ONLINE)
			crstart(cr);
	}
}

crsgtty(dev, v)
int dev;
register int *v;
{
	register struct cr11 *cr;

	cr = cr11[dev.d_minor & 07];
	if (v != NULL) {		/* gtty */
		*v++ = cr->crstate & 0377;
		*v++ = cr->crmode & 0377;
		*v++ = cr->craddr->crstatus;
		return(1);
	} else {			/* stty */
		/* only allowed to set mode ... */
		cr->crmode = u.u_arg[1];
		return(0);
	}
}
