#

/*
 *	copyright  march 1976 ian johnstone.
 *
 *	cdc u200 emulator.
 */

#include "../param.h"
#include "../conf.h"
#include "../user.h"
#include "../buf.h"

extern char	partab[];

#define	PS	0177776->integ
#define	EISPL	spl6

	/*  dp11  device  registers  */

#define	DPADDR	0174770

struct {
	int	xxrcsr;	/* receive status register	*/
	char	xxrbuf;	/* receive buffer		*/
	char	xxsyn0;	/* sync register		*/
	int	xxtcsr;	/* transmit status register	*/
	char	xxtbuf;	/* transmit buffer		*/
	char	xxsyn1;
	};

#define	ODDPAR	 010000
#define	DONE       0200
#define	IENABLE	   0100
#define	HDUPLX       02
#define	STRPSYNC     01
#define	CTRANS	0100000
#define	RORUN	 040000
#define	RING	 020000
#define	DSRDY	 010000
#define	CARRIER   04000
#define	DONE	   0200
#define	IENABLE	   0100
#define	SIENABLE    040
#define	IDLESYNC     02

/*	buffers and buffer pointers	*/

		/* receive buffer pointers */
char	ycrbuffer[1050]	0;
char	*ycrbuf		ycrbuffer;
char	*ycrbufe	&ycrbuffer[sizeof ycrbuffer - 1];
char	*ycrbufp	ycrbuffer;

		/* transmit buffer pointers */
char	*yctbufp	0;

		/* card buffer pointers */
int	ycjpadding	0;
char	ycjbuffer[984-14]	0;
struct buf	ycjbaddr {
	0,0,0,0, 0,0,0,ycjbuffer
	};

		/* print buffer pointers */
char	yclbuffer[1050-14]	0;
struct buf	yclbaddr {
	0,0,0,0, 0,0,0,yclbuffer
	};
#define	YCLBUF	yclbaddr.b_addr
char	*yclbufp	yclbuffer;
char	*yclbufe	&yclbuffer[sizeof yclbuffer - 1 + 14];
int	yclbufl	0;

		/* answer buffer pointers */
#ifdef	CLOBBER
#define	ycabuf	main
extern char	main[];
#endif	CLOBBER
#ifndef	CLOBBER
char	ycabuf[740]	0;
#endif	CLOBBER
int	ycabufl	0;
char	*ycabufe	0;

		/* command buffer pointers */
char	yccbuf[22]	0;
int	yccbufl		{ (sizeof yccbuf)-(1+3) };

/*	flags to control all	*/
int	ycflag;
int	yceoj;
int	ycsus;		/* set when output suspended by op */

#define	CRDY	(1<<0)	/* =| if cmd queued to cyber */
#define	RRDY	(1<<1)	/* =| if message from cyber avail */
#define	RUSE	(1<<2)	/* =| if message being passed */

#define	JRDY	(1<<3)	/* another batch of cards ready to go */
#define	RDE2	(1<<4)	/* =| if a 'read e2'  sent for card reader  */

#define	WJOPN	(1<<5)	/* cyb.jobs open for write */
#define	RJOPN	(1<<6)	/* cyb.jobs open for read */

#define	WCOPN	(1<<7)	/* cyb.cmds open for write */
#define	RCOPN	(1<<8)	/* cyb.cmds open for read */

#define	JFAVL	(1<<9)	/* set if print buffer ready */
#define	PRE2	(1<<10)	/* set if a 'read e2' sent for printer */
#define	WPRT	(1<<11)	/* set if waiting for prt output */
#define	WCRD	(1<<12)	/* set if waiting for card buffer */
#define	WMSG	(1<<13)	/* set if waiting for message */
#define	WCOM	(1<<14)	/* set if waiting for command buffer */


#define	CYOPN	(WJOPN|RJOPN|WCOPN|RCOPN)

	/* yceoj flags */
#define	FINJ	01
#define	ENDJ	02

/*	symbolic equates	*/

#define	THIS_SITE	0160
#define	STOH		0001
#define	EOT		0003
#define	QEOT		0203
#define	ESCP		0076
#define	QESCP		0276
#define	REJECT		0030		/* u-200 */
#define	QREJECT		0230
#define	SYNC		0026		/* u-200 */
#define	ERROR		0025		/* u-200 */
#define	QERROR		0225
#define	READ		0023		/* u-200 */
#define	QREAD		0223
#define	ACK		0006		/* u-200 */
#define	QACK		0206
#define	WRITE		0021		/* u-200 */
#define	CWRITE		0022		/* u-200 */
#define	RWRITE		0014		/* u-200 */
#define	POLL		0005		/* u-200 */
#define	ALERT		0007		/* u-200 */
#define	VT		'\013'
#define	SPACE3		0040		/* cyber */
#define	SPACE2		0112		/* cyber */
#define	SPACE1				/* cyber */
#define	SPACE0		0060		/* cyber */
#define	EJECT		0101		/* cyber */
#define	SKIPC4		0105		/* cyber */
#define	EOJ		0126		/* cyber */
#define	CEOL		0120		/* cyber */
#define	CCR		0101		/* cyber */
#define	LOWBCD		0040
#define	HIGHBCD		0137
#define	E1		0102
#define	QE1		0302
#define	E2		0040
#define	QE2		0240
#define	E3		0041
#define	QE3		0241
#define	EOI		0126		/* cyber */
#define	QEOI		0326
#define	EOR		0127		/* cyber */
#define	QEOR		0327
#define	CTRLR		'\022'

/*	cyber talking control words	*/

char	ycerrf	0;	/* non-zero => receive error */
	/*
	 *	0 ==> o.k.
	 *	1 ==> format error in received msg.
	 *	2 ==> lpc error in received msg.
	 *	3 ==> hardware detected receive error.
	 *	4 ==> last msg. transmitted no good.
	 *	5 ==> last msg. received too long.
	 *	6 ==> talk syncronizing error.
	 */

	/*
	 * short error description
	 *			 1  2  3  4  5  6
	 */
	char	*ycerrfm	"fmtlpcrecxmtlenpro";
	int	ycerrfc[6];		/* error counts */
/****/	int	ycering;	/*******/
/****/	int	ycecarrier;	/*******/
/****/	int	yceoverun;	/*******/

char	ycsite	0;	/* site address from last msg. */
char	ycstat	0;	/* station address from last msg. */
char	yccode	0;	/* control code from last msg. */
char	ycetyp	0;	/* } code from last msg. */
char	ycwstat	0141;	/* station address from last write msg. */
char	yctstat	0;	/* station address to be transmitted */

char	*ycbmsg[10]	0;	/* a 10 msg stack */
int	ycb	0;		/* msg stack pointer */
int	ycnsync	1;
char	*ycmsg	-1;	/* ptr to last 'msg' sent to cyber */
char	*ycnowmsg	0;	/* ptr to an immediate msg */


int	ycrnext	1;
int	yctnext	1;

/*	std. messages for cyber */

char	ycackm[]	{	QACK,	QEOT				};
char	ycrejm[]	{	QREJECT,QEOT				};
char	ycerrm[]	{	QERROR,	QEOT				};
char	ycreed[]	{	QREAD,	'r',	QESCP,	QE1,	QEOT	};
char	ycgo[]		{	QREAD,	'g',	QESCP,	QE1,	QEOT	};
char	yccont[]	{	QREAD,	'c',	QESCP,	QE1,	QEOT	};
char	yce1m[]		{	QREAD,	QESCP,	QE1,	QEOT		};
char	yce2m[]		{	QREAD,	QESCP,	QE2,	QEOT		};
char	yce3m[]		{	QREAD,	QESCP,	QE3,	QEOT		};
char	ycbye[]		{	QREAD,	'B',	QESCP,	QE1,	QEOT	};
char	ycdiag1[] 	"\nerror: modem not ready\n";
char	ycdiag2[] 	"\nlinerr    ";


char ycbtoa[] {		/* bcd to ascii conversion */
	'-', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', '!', '$', '*', '^', '#', '>',
	'+', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', '<', '.', ')', '?', ':', ';',
	':', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0', '=','\'','\\', '%', '[',
	' ', '/', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', ']', ',', '(', '&', '"', '@',
};

char ycatob[] {		/* ascii to bcd conversion */
	0120,0052,0136,0056,0053,0116,0135,0114,	/*  !"#$%&' */
	0134,0074,0054,0060,0133,0040,0073,0121,	/* ()*+,-./ */
	0112,0101,0102,0103,0104,0105,0106,0107,	/* 01234567 */
	0110,0111,0100,0077,0072,0113,0057,0075,	/* 89:;<=>? */
	0137,0061,0062,0063,0064,0065,0066,0067,	/* @ABCDEFG */
	0070,0071,0041,0042,0043,0044,0045,0046,	/* HIJKLMNO */
	0047,0050,0051,0122,0123,0124,0125,0126,	/* PQRSTUVW */
	0127,0130,0131,0117,0115,0132,0055,0120,	/* XYZ[\]^_ */
	0120,0061,0062,0063,0064,0065,0066,0067,	/* `abcdefg */
	0070,0071,0041,0042,0043,0044,0045,0046,	/* hijklmno */
	0047,0050,0051,0122,0123,0124,0125,0126,	/* pqrstuvw */
	0127,0130,0131,0120,0120,0120,0120,0120,	/* xyz{|}~  */
};

/*	cyber commands		*/

ycopen(dev, flag)
{
	register int x;

	x = !flag ? RCOPN : WCOPN;
	if ((ycflag & x) || cystart())
		u.u_error = ENXIO;
	else	
		ycflag =| x;
}

ycclose(dev, flag)
{
	ycflag =& ~(!flag ? RCOPN : WCOPN);
}

ycmsgout(msg, len)	/* place msgs for ops for cyb.cdn */
char	msg[];
int	len;
{
	register char	*x, *y, *z;

	if (!(ycflag & RUSE)) {
		z = ycabufe - ycabuf;
		y = len;
		if (z > y)
			z = ycabufl = y;
		else
			ycabufl = z;
		x = ycabuf;
		y = msg;
		while (z-- != 0)
			*x++ = *y++;
		ycflag =| RRDY;
		if (ycflag & WMSG) {
			ycflag =& ~WMSG;
			wakeup(&ycabuf);
		}
		return(x);	/* indicate all okay */
	}
	return(0);
}

ycwrite(dev)		/* to send commands to cyber */
int dev;
{
	register char x, *y;

	EISPL();

	if (ycflag&CRDY || u.u_count>yccbufl)
		u.u_error = ENXIO;
	else {
		y = yccbuf;
		*y++ = QREAD;

		while ((x = cpass()) > 0)
			*y++ = x & 0177;
		*y++ = QESCP;
		*y++ = QE1;
		*y++ = QEOT;

		switch (yccbuf[1]) {

		case 'l':
		case 'c':
			ycsus = 0;
			break;

		case 's':
			ycsus = 1;
			break;
		}

		ycflag =| CRDY;
		ycqmsg(yccbuf);
	}
	spl0();
}

ycread(dev)	/* to read response from cyber */
int dev;
{
	register char *x;
	register int y;

	EISPL();
	while (!(ycflag & RRDY)) {
		ycflag =| WMSG;
		sleep(&ycabuf, 1);
	}
	ycflag =& ~RRDY;
	ycflag =| RUSE;
	spl0();
	x = ycabuf; 
	y = (ycabufl > u.u_count) ? u.u_count : ycabufl;
	while (y-- != 0)
		passc(*x++);
	ycflag =& ~RUSE;
}

/*	cyber jobs		*/

yjopen(dev, flag)
{
	register int x;

	x = !flag ? RJOPN : WJOPN;
	if ((ycflag & x) || cystart())
		u.u_error = ENXIO;
	else 
		ycflag =| (!flag ? (RJOPN|PRE2) : (WJOPN|RDE2));
}

yjclose(dev, flag)
{
	if (flag)
		ycflag =& ~(WJOPN|JRDY|RDE2);
	else
		ycflag =& ~(RJOPN|JFAVL);
}

yjwrite(dev)	/* to send jobs to the cyber */
int dev;
{
	register int n, m;

	EISPL();
	while (ycflag & JRDY) {
		ycflag =| WCRD;
		sleep(&ycjbuffer, 1);
	}
	spl0();
	ycjbaddr.b_addr = ycjbuffer;
	n = u.u_count;
	if ((m = n & 01776) != 0)
		iomove(&ycjbaddr, 0, m, B_WRITE);
	if (n & 1)
		ycjbuffer[m++] = cpass();

	if (m == 0)
		return;
	ycjbuffer[m++] = QESCP;
	ycjbuffer[m++] = QE3;
	ycjbuffer[m++] = QEOT; 
	EISPL();
	ycflag =| JRDY;
	if (ycflag & RDE2) {
		ycqmsg(ycreed);
		ycflag =& ~RDE2;
	}
	spl0();
}

yjread(dev)	/*
		 * to read jobs output from the cyber. as 
		 * much  as possible each time
		 */
int dev;
{
	register int n,  m;

	EISPL();
	if (yceoj & ENDJ) {
		/* terminate last job */
		yceoj =& ~ENDJ;
		u.u_error = ENXIO;
		spl0();
		return;
	}
	while (!(ycflag & JFAVL)) {
		ycflag =| WPRT;
		sleep(&YCLBUF, 1);
	}
	if (yceoj & FINJ)
		yceoj = ENDJ;
	spl0();
	n = (u.u_count < yclbufl) ? u.u_count : yclbufl;
	if ((m = n & 03776) != 0)
		iomove(&yclbaddr, 0, m, B_READ);
	if (n & 1)
		passc(YCLBUF[m]);
	EISPL();
	ycflag =& ~JFAVL;	/* buffer all used */
	spl0();
}

ycawful(dummy)	/* fuk fuk fuk fuk fuk fuk fuk fuk fuk */
{
	if (ycflag & CYOPN) {
		if (ycb < 2) {
			/* stir up export -- yes it is needed */
			if (!ycsus)
				ycqmsg(yccont);
			ycqmsg(ycgo);
		}
		timeout(ycawful, 0, 20*HZ);
	}
}

ycqmsg(mp)		/* place messages for cyber in 'fifo' queue */
char	*mp;
{
	register int x,	sps;
	
	if (ycb <= 8) {
		sps = PS;
		EISPL();
		for (x = ++ycb; x > 1; x--)
			ycbmsg[x - 1] = ycbmsg[x - 2];
			ycbmsg[0] = mp;
		PS = sps;
	}
}

ycerep(en)		/* called to log errors and optionally tell op */
int	en;
{
	register char *x,*y;

	ycerrfc[en - 1]++;
	if (!(SW->integ & 020000)) {

		x = &ycerrfm[en*3 - 3];
		y = ycdiag2 + 8;
		*y++ = *x++;
		*y++ = *x++;
		*y = *x;
		ycmsgout(ycdiag2, 11);
	}
}

ycwpro()	/*
		 * process cyber write commands -- called from 
		 * receive interrupt routine
		 */
{
	register char *x, *y;
	register int *z;

	if (ycmsg == &ycjbuffer[-1]) {
		ycflag =& ~JRDY;
		if (ycflag & WCRD) {
			ycflag =& ~WCRD;
			wakeup(&ycjbuffer);
		}
	} else if (ycmsg == yccbuf)
		ycflag =& ~CRDY;
	ycmsg = 0;	/* last 'msg' to cyber was accepted */
	
	switch (ycetyp) {	/* process write according to } code */

	case E1:
		/* if bit #12 in swreg is set throw away useless crap */
		if (SW->integ & 010000) {
			z = ycrbuf;
			if ((z[0] == ' R') ||	/* ' ready' */
			    (z[1] == 'TE') ||	/* '  terminal idle */
			    (z[0] == ' C') ||	/* ' card reader not ready' */
			    (z[1] == 'RI') || 	/* ' printer not ready' */
			    (z[6] == 'SU') ||	/* ' no file is suspended */
			    (z[2] == 'IN'))	/* ' no input active' */
				break;
		}
		/* can't just ignore last job read messages */
		if (!ycmsgout(ycrbuf, ycrbufp-ycrbuf) && z[0]==' L') {
			yctbufp = ycrejm;
			return;
		}
		break;

	case E2:  	/* print output */
		/* if can't accept pretend not ready */
		if ((ycflag & JFAVL) || !(ycflag & RJOPN)) {
			ycnowmsg = yce2m;
			ycflag =| PRE2;
			break;
		}
		x = ycrbufp;
		y = ycrbuf;
		z = ycrbufe;
		switch (*y) {		/* adjust first char */

		default:
			*y = '\n';
		case '\n':
		case '\r':
		case '\f':
		case VT:
			break;
		}
		ycrbufp = ycrbuf = YCLBUF;
		ycrbufe = yclbufe; 
		yclbufp = YCLBUF = y;
		yclbufe = z;
		yclbufl = x - y;
		ycflag =| JFAVL;
		if (ycflag & WPRT) {
			ycflag =& ~WPRT;
			wakeup(&YCLBUF);
		}
		/*  tell cyber want more output */
		if (!ycb)
			ycnowmsg = yce3m;
		break;

	case E3:				/* card input */
		if (ycflag & JRDY) {
			ycnowmsg = &ycjbuffer[-1];
			ycjbuffer[-1] = QREAD;
		} else {
			/* no cards to go so go not ready */
			ycnowmsg = yce2m;
			ycflag =| RDE2;
		}
		break;
	}
	yctbufp = ycackm;		/* acknowlege receipt of write */
}

cyrint(dev)	/* receive interrupt routine */
int dev;
{
	register int c;
	static int lpc;
#define	YCRSTOH	1
#define	YCRSITA	2
#define	YCRSTAA	3
#define	YCRCC	4
#define	YCRMSG	5
#define	YCRMSG1	6
#define	YCRMSG2	7
#define	YCREND	8
#define	YCREOT	9
#define	YCRLPC	10
#define	YCRERR	11

	while (DPADDR->xxrcsr & DONE) {
		c = DPADDR->xxrbuf & 0177;	/* obtain next char sans parity */
		lpc =^ c;		/* calc longtitudinal parity */

		if (!(DPADDR->xxrcsr & ODDPAR))
			ycrnext = YCRERR;

		switch (ycrnext) {	/* message sequence */

		case YCRSTOH: 		/* first char must be start of header */
			if (c != STOH)
				goto ycrerror;
		ycrstoh1:
			ycrnext = YCRSITA;
			lpc = STOH;
			continue;

		case YCRSITA: 		/* second char <=0177 &  >=0160  */
			if (c < 0160)
				goto ycrerror;
			ycrnext = YCRSTAA;
			ycsite = c;
			continue;

		case YCRSTAA: 		/* third char 0140,0141,0160,0161 */
			switch (c) {

			default:
				goto ycrerror;

			case 0140:
			case 0141:
			case 0160:
			case 0161:
				ycrnext = YCRCC;
				ycstat = c;
				break;
			}
			continue;

		case YCRCC:		/* fourth must be acceptable command */
			switch (c) {

			default:
				goto ycrerror;

			case WRITE:
			case CWRITE:
			case RWRITE:
				yccode = WRITE;
				ycrnext = YCRMSG;
				continue;

			case POLL:
			case ALERT:
				yccode = c;
				ycrnext =YCREOT;
			  	continue;
			}

		case YCRMSG:		/* process data portion of message */
			ycrnext = YCRMSG2;
			switch (c) {	/* carriage control */

			case SPACE3:
				*ycrbufp++ ='\n';
			case SPACE2:
				*ycrbufp++ ='\n';
			default:
				if (ycrbufp == ycrbuf)
					goto ycrtr;
			  	c = '\n';
				break;

			case SPACE0:
				c = '\r'; 
				break;

			case EJECT:
				c = '\f';
				break;

			case SKIPC4:
				c = VT;
				break;

			case ESCP:
				ycrnext = YCRMSG1;
				continue;
			}
			goto ycrmsg3;

		case YCRMSG1:
			switch (c) {	/* function code */

			case E1:
			case E2:
			case E3:	/* ending code */
				DPADDR->xxrcsr =& ~STRPSYNC;
				ycetyp = c;
				ycrnext = YCREOT;
				continue;

			case EOJ:
				yceoj = FINJ; 
			case CEOL:
			case CCR:
				ycrnext = YCRMSG;
				continue;

			default:		/* '0' or ' ' expansion */
				c =- 040;
				if (c>=3 && c<=037)
					*ycrbufp++ = -1;
				else {
					c =- 040;
					if (c>=3 && c<=017)
						*ycrbufp++ = -2;
					else
						c = ' ';
				}
				ycrnext = YCRMSG2;
				goto ycrmsg3;
			}

		case YCRMSG2:
					/* bcd data */
			if (c == ESCP) {
				ycrnext = YCRMSG1;
				continue;
			}
		ycrtr:
			if (c>HIGHBCD || c<LOWBCD)
				c = ' ';
			else
				c = ycbtoa[c - LOWBCD];
		ycrmsg3:
			if (ycrbufp > ycrbufe) {
				if (!ycerrf)
					ycerrf = 5;	/* len */
				goto ycrerros;
			}
			*ycrbufp++ = c;
			continue;

		case YCREND:		/* look for end of message */
			if (c == STOH) {
				ycrbufp = ycrbuf;
				goto ycrstoh1;
			}
			continue;

		case YCREOT:		/* this char must be "eot" */
			if (c != EOT)
				goto ycrerror;
			ycrnext = YCRLPC;
			continue;

		case YCRERR:
		ycrerror:
			if (!ycerrf)
				ycerrf = 1;	/* fmt */
		ycrerros:
			ycrnext = YCREND;
			DPADDR->xxrcsr =& ~STRPSYNC;
			continue;

		case YCRLPC:		/* this char is longtitudinal parity */
			if (lpc!=0177 && !ycerrf)
				ycerrf = 2;	/* lpc */
			DPADDR->xxtcsr = 0;	/* terminate input */
			DPADDR->xxrcsr = 0;	/* terminate input */
			ycrnext = YCRSTOH;	/* =| for next receive */

			if (ycsite != THIS_SITE) {
				DPADDR->xxsyn0 = SYNC;
				DPADDR->xxrcsr = HDUPLX|STRPSYNC|IENABLE;
				DPADDR->xxtcsr =| SIENABLE;
			} else {
				if (ycerrf) {
					ycerep(ycerrf);
					ycerrf = 0;
					yctbufp = ycerrm;
				} else {
					if (ycstat & 1)
						switch (yccode) {	/* odd station address */

						case WRITE:
							/* save write address */
							ycwstat = ycstat;
							ycwpro();
							break;

						case ALERT:
							yctbufp = ycackm;
							ycqmsg(yce1m);
							break;

						default:
							yctbufp = ycerrm;	/* ??????? */
							ycerep(6);	/* pro */
							break;
						}
					else
						switch (yccode) {	/* even station address */

						case POLL:
							if (ycmsg == -1) {
								ycmsg = 0;
								yctbufp = ycrejm;
							} else if ((yctbufp = ycmsg) != 0)
								ycerep(4);	/* xmt */
							else if (ycnowmsg) {
								yctbufp = ycmsg = ycnowmsg;
								ycnowmsg = 0;
							} else
								yctbufp = (ycb ? (ycmsg = ycbmsg[--ycb]) : ycrejm);
							break;

						default:
							yctbufp = ycerrm;
							ycerep(6);	/* pro */
							break;
						}
					/* reject to 'poll' is different */
					yctstat = ycwstat;
					if (yccode==POLL && *yctbufp==0177630)
						yctstat =& 0176;
				}
				ycrbufp = ycrbuf;
				DPADDR->xxtcsr = DSRDY|IENABLE|IDLESYNC;
				DPADDR->xxrcsr = HDUPLX;
				DPADDR->xxtbuf = SYNC;
			}
			break;
		}
	}
}

cyxint(dev)		/* transmitter status interrupt routine */
/*	yctbufp points to the message to be sent to the cyber	*/
/*	QEOT signals end of message.				*/
/*	any char with parity bit on is not translated		*/
int dev;
{
	register c;
	static int lpc;
#define	YCXSYNC	1
#define	YCXSTOH	2
#define	YCXSITA	3
#define	YCXSTAA	4
#define	YCXMSG 	5
#define	YCXLPC 	6
#define	YCXEND	7

	if (DPADDR->xxtcsr & SIENABLE) {
		/* receive error */
		if (!ycerrf) {
			ycerrf = 3;	/* rec */
			if (DPADDR->xxtcsr & RORUN)
				yceoverun++;
			if (DPADDR->xxtcsr & RING)
				ycering++;
			if (DPADDR->xxtcsr & CTRANS)
				ycecarrier++;
		}
		DPADDR->xxtcsr =& ~(CTRANS|RORUN|RING);
		DPADDR->xxsyn0 = SYNC;	/* reset just in case */
		return;
	}
	switch (yctnext) {		/* transmit message sequence */

	case YCXSYNC:			/* send 4 syncs */
		c = SYNC;
		if (++ycnsync == 4)
			yctnext = YCXSTOH;
		goto ycxmit;

	case YCXSTOH:			/* 1st char is start of header */
		c = STOH;
		yctnext = YCXSITA;
		lpc = 0;
		goto ycxmit;

	case YCXSITA:			/* 2nd char is site address */
		c = ycsite;
		yctnext = YCXSTAA;
		goto ycxmit;

	case YCXSTAA:			/* 3rd char is station address */
		c = yctstat;
		yctnext = YCXMSG;
		goto ycxmit;

	case YCXMSG:			/* output data till eot found */
		c = *yctbufp++;
		if (c & 0200) {
			c =& 0177;
			if (c == EOT)
				yctnext = YCXLPC;
		} else
			c = (c < ' ') ? 0120 : ycatob[c - 040];
	ycxmit:
		c =| ~partab[c] & 0200;	/* odd parity gen */
		lpc =^ c;		/* long. parity */
		DPADDR->xxtbuf = c;
		return;

	case YCXLPC:			/* output long. parity */
		c = ~lpc & 0177;
		yctnext = YCXEND;
		goto ycxmit;

	case YCXEND:			/* all msg sent - tidy up */
		if (ycflag & CYOPN) {
			DPADDR->xxsyn0 = SYNC;	/* =| just in case */
			DPADDR->xxrcsr = HDUPLX|STRPSYNC|IENABLE;	/* enable receive */
			DPADDR->xxtcsr = SIENABLE;	/* enable receive */
		} else {
			DPADDR->xxrcsr = 0;	/* no open devices so close down */
			DPADDR->xxtcsr = 0;	/* no open devices so close down */
		}
		ycrnext = YCRSTOH;
		yctnext = YCXSYNC;
		ycnsync = 1;
		break;
	}
}

cystart()	/* initiate receive on dp-11 if necessary */
{
	extern ycawful();

	if (!(DPADDR->xxtcsr & DSRDY)) {
		printf(ycdiag1);	/* modem not ready */
		return(1);
	}
	if (!(ycflag & CYOPN)) {
		DPADDR->xxsyn0 = SYNC;
		DPADDR->xxrcsr = HDUPLX|STRPSYNC|IENABLE;
		DPADDR->xxtcsr = SIENABLE;

		/* reinit necessary variables */
	
		ycsus = ycerrf = ycflag = ycb = 0;
		ycmsg = -1;
		ycrbuf = ycrbufp = ycrbuffer;
		ycrbufe = &ycrbuffer[sizeof ycrbuffer - 1];
		YCLBUF = yclbufp = yclbuffer;
		yclbufe = &yclbuffer[sizeof yclbuffer - 1 + 14];
		ycrnext = YCRSTOH;
		yctnext = YCXSYNC;
		ycnsync = 1;
		ycabufe = &ycabuf[730];
		ycqmsg(ycbye);
		timeout(ycawful, 0, 20*HZ);
	}
	return(0);
}
