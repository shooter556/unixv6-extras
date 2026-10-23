#
/*
 *	tm11 driver
 *
 *	minor devices 0-7  get rewound on open
 *	minor devices 8-15 don't
 *
 *	indutry standard 2 tape marks for EOF
 *
 *	stty functions for skip forward/reverse file/block, rewind, write tape mark
 */


#include "../param.h"
#include "../conf.h"
#include "../user.h"
/*
#include "../userx.h"
 */
#include "../buf.h"
#include "../reg.h"

/*
 * New tm-11 driver
 * J. N. Rottman
 */

/*
 * Structure of device registers
 */

struct	{
	int	tmer;
	int	tmcs;
	int	tmbc;
	int	tmba;
	int	tmdb;
	int	tmrd;
};

struct	devtab	tmtab;
#ifndef	RAW_BUFFER_POOL
struct	buf	rtmbuf;
#endif

#define	TMADDR	0172520		/* register base	*/
#define	NTM11	1		/* unit select 0 and 1	*/

/* tmcs */
#define	GO	01		/* hardware go bit 	*/
#define	RCOM	03		/* read command 	*/
#define	WCOM	05		/* write command	*/
#define	WIRG	015		/* write extended gap	*/
#define	DENS	060000		/* 9-track 800 bpi	*/
#define	IENABLE	0100		/* interrupt enable	*/
#define	CRDY	0200		/* controller ready	*/

/* tmrd */
#define	GAPSD	010000		/* gap slow down	*/

/* tmer */
#define	TUR	01		/* tape driver ready	*/
#define	HARD	0102200		/* hard error		*/
#define	EOF	0040000		/* end-of-file		*/
#define	RWS	02		/* rewind started	*/
#define	WRL	04		/* write locked		*/
#define	SELR	0100		/* tape unit on line	*/

#define	SSEEK	1		/* positioning		*/
#define	SIO	2		/* read/write		*/
#define	SBSP	3		/* error recovery BS	*/
#define	SPHYS	4		/* stty functions	*/

#define	SSFB	011		/* skip forward block	*/
#define	SSRB	013		/* skip reverse block	*/
#define	SSFF	001		/* skip forward file	*/
#define	SSRF	003		/* skip reverse file	*/
#define	SSREW	017		/* rewind on-line	*/
#define	SSWEOF	007		/* write eof		*/
#define	SSWRL	040000		/* check tape write lock */
#define	SSONLN	0100000		/* check tape on-line	*/

#define	OPEN	01		/* drive open		*/
#define	HARDE	02		/* drive hard error	*/
#define	LASTW	04		/* driver last command was write */


struct	{
	char	t_status[NTM11];
	unsigned t_blkno[NTM11];
	unsigned t_nxrec[NTM11];
} tm11;


/*
 * Open: check for legal drive number, and
 * already open drive. If unit is in second
 * bank, don't rewind on open, otherwise, do so.
 */

tmopen(dev, flag)
{
	register dminor;

	dminor = dev.d_minor & 07;
	if(dminor >= NTM11 || tm11.t_status[dminor]&OPEN)
		u.u_error = ENXIO;
	else {
		tm11.t_status[dminor] = OPEN;
		tmcommand(dminor, flag ? SSWRL|SSONLN : SSONLN);
		if((dev.d_minor & 010) == 0)
			tmcommand(dminor, SSREW);
		tm11.t_blkno[dminor] = 0;
		tm11.t_nxrec[dminor] = 0177777;	/* last block written set at 64k */
		if(u.u_error)
			tm11.t_status[dminor] = 0;
	}
}

/*
 * Close: if the last command on the drive was
 * a write, then close off with a WEOF
 */

tmclose(dev)
{
	register dminor;

	dminor = dev.d_minor&07;
	if(tm11.t_status[dminor]&LASTW) {
		tmcommand(dminor, SSWEOF);
		tmcommand(dminor, SSWEOF);
	}
	tm11.t_status[dminor] = 0;
}

/*
 * Strategy: the tm supports three basically distinct types
 * of I/O. As a UNIX style block device, it behaves just like
 * a random access device, with the exception that writes
 * must be sequential, and invalidate what lies beyond them.
 * As a raw device, it can read and write records of any
 * blocksize. It also supports tape positioning functions:
 * skip [forward/reverse] [blocks/files], rewind, and weof.
 */

tmstrategy(bp)
register struct buf *bp;
{
	register unsigned *p;

	if((bp->b_flags&(B_NOTIO|B_PHYS))==0) {	/* i.e. block device */
		p = &tm11.t_nxrec[bp->b_dev.d_minor&07];
		if(*p <= bp->b_blkno) {
			if(*p < bp->b_blkno) {	/* attempt to read/write non-existent block */
				bp->b_flags =| B_ERROR;
				iodone(bp);
				return;
			}
			if(bp->b_flags&B_READ) {	/* attempt to read block immediately beyond last block written */
				clrbuf(bp);
				iodone(bp);
				return;
			}
		}
		if((bp->b_flags&B_READ)==0)
			*p = bp->b_blkno + 1;
	}
#ifdef	_1170
	if(bp->b_flags&B_PHYS)
		mapalloc(bp);
#endif
	bp->av_forw = NULL;
	spl5();
	if(tmtab.d_actf == NULL) {
		tmtab.d_actf = bp;
		tmtab.d_actl = bp;
		tmstart();
	} else {
		tmtab.d_actl->av_forw = bp;
		tmtab.d_actl = bp;
	}
	spl0();
}


/*
 * Start: If the requested function is on a drive with
 * a hard error, shoot it down.  turn skip-files into
 * the skip-blocks that the hardware supports. Do positioning
 * for block-type i/o.  Write with extended IRG for
 * crudy tapes.
 */

tmstart()
{
	register struct buf *bp;
	register com;
	register char *blkno;
	int unit;

loop:
	if((bp = tmtab.d_actf) == NULL)
		return;
	unit = bp->b_dev.d_minor & 07;
	if(tm11.t_status[unit]&HARDE) {
		bp->b_flags =| B_ERROR;
deq:
		tmtab.d_actf = bp->av_forw;
		iodone(bp);
		goto loop;
	}
	tm11.t_status[unit] =& ~LASTW;
	com = (unit << 8) | ((bp->b_xmem&03) << 4) | IENABLE | DENS;
	if(bp->b_flags&B_NOTIO) {
		if(bp->b_blkno & (SSWRL|SSONLN)) {
			TMADDR->tmcs = (unit << 8);
			com = 100;
			do; while(--com);
			if (((com = TMADDR->tmer)&(TUR|SELR))!=(TUR|SELR) || ((com&WRL)&&(bp->b_blkno&SSWRL)))
				bp->b_flags =| B_ERROR;
			goto deq;
		}
		if(bp->b_blkno == SSFF || bp->b_blkno ==SSRF) {
			com =| 010;
			TMADDR->tmbc = 0;
		} else
			TMADDR->tmbc = bp->b_wcount;
		tmtab.d_active = SPHYS;
		TMADDR->tmcs = com | bp->b_blkno;
	} else
		if((bp->b_flags&B_PHYS)==0 && (blkno = tm11.t_blkno[unit]) != bp->b_blkno) {
			tmtab.d_active = SSEEK;
			if(blkno < bp->b_blkno) {
				TMADDR->tmbc = blkno - bp->b_blkno;
				com =| SSFB;
			} else {
				if(bp->b_blkno == 0)
					com =| SSREW;
				else {
					TMADDR->tmbc = bp->b_blkno - blkno;
					com =| SSRB;
				}
			}
			TMADDR->tmcs = com;
		} else {
			tmtab.d_active = SIO;
			TMADDR->tmbc = (bp->b_wcount << 1);
			TMADDR->tmba = bp->b_addr;
			TMADDR->tmcs = com | ((bp->b_flags&B_READ)? RCOM:
				((tmtab.d_errcnt)? WIRG: WCOM));
		}
}


/* Interrupt: If there is nothing to do, just return.
 * Forget about the interrupt on initiating a rewind.
 * Fix up soft errors on read and write, continue
 * processing of Skip files.
 * Note that physical limitations mean that a mag-tape
 * could not have more than about 50000 records .
 */

tmintr()
{
	register struct buf *bp;
	register unit;
	register error;

	error = TMADDR->tmer;
	if(error&RWS || ((bp = tmtab.d_actf) == NULL))
		return;
	unit = bp->b_dev.d_minor & 07;

	if(TMADDR->tmcs < 0) {
		while(TMADDR->tmrd&GAPSD);
		if(bp->b_flags&B_NOTIO && error&EOF) {
			if(bp->b_blkno == SSFF || bp->b_blkno == SSRF)
				if(--bp->b_wcount <= 0)
					goto done;
				else {
					TMADDR->tmbc = 0;
					TMADDR->tmcs =| GO;
					return;
				}
			if(bp->b_blkno == SSFB || bp->b_blkno == SSRB || bp->b_blkno == SSWEOF)
				goto done;
		}
		if(tmtab.d_active == SIO && (error&(HARD|EOF))==0 &&  ++tmtab.d_errcnt < 10) {
			tmtab.d_active = SBSP;
			TMADDR->tmbc = -1;
			TMADDR->tmcs = (unit << 8) | IENABLE | SSRB;
			return;
		}
		if(error&HARD)
			tm11.t_status[unit] =| HARDE;
		tmtab.d_errcnt = 0;
		bp->b_flags =| B_ERROR;
		tmtab.d_active = SIO;
	}else
		if(tmtab.d_active == SIO && (bp->b_flags&B_READ)==0)
			tm11.t_status[unit] =| LASTW;

	if(tmtab.d_active == SSEEK)
		tm11.t_blkno[unit] = bp->b_blkno;
	else if(tmtab.d_active != SBSP) {
		tm11.t_blkno[unit]++;
done:
		tmtab.d_actf = bp->av_forw;
		bp->b_resid = TMADDR->tmbc;
		if(tmtab.d_active == SIO)
			bp->b_resid =>> 1;
		iodone(bp);
	}

	tmstart();
}


/*
 * TM stty: fetch code and count from user, and pass then
 * into raw buffer header. Obey buffer protocols.
 * Codes:
 *	0	Skip forward file
 *	1	Skip reverse file
 *	2	Unused
 *	3	Write end-of-file
 *	4	Skip forward block
 *	5	Skip reverse block
 *	6	Unused
 *	7	Rewind
 */

tmsgtty(dev)
{
	register com;

	com = ((u.u_arg[0] & 07) << 1) | GO;
	if(com == 05 || com == 015) {
		u.u_error = ENXIO;
		return;
	}
	u.u_ar0[R0] = tmcommand(dev, com, u.u_arg[1]);
}

tmcommand(dev, com, cnt)
{
	register struct buf *bp;

#ifndef	RAW_BUFFER_POOL
	bp = &rtmbuf;
	while(bp->b_flags&B_BUSY) {
		bp->b_flags =| B_WANTED;
		sleep(bp, PRIBIO);
	}
#endif
#ifdef	RAW_BUFFER_POOL
	bp = getrb(dev , B_NOTIO);
#endif
	bp->b_flags = B_BUSY | B_NOTIO;
	bp->b_dev = dev;
	bp->b_blkno = com;
	bp->b_wcount = cnt;
	tmstrategy(bp);
#ifndef	RAW_BUFFER_POOL
	spl5();
#endif
#ifdef	RAW_BUFFER_POOL
	spl6();
#endif
	while((bp->b_flags&B_DONE)==0)
		sleep(bp, PRIBIO);
#ifndef	RAW_BUFFER_POOL
	if(bp->b_flags&B_WANTED)
		wakeup(bp);
	spl0();
	bp->b_flags =& ~(B_BUSY|B_WANTED);
#endif
#ifdef	RAW_BUFFER_POOL
	freerb(bp);
#endif
	geterror(bp);
	if(com == SSFB || com == SSRB)
		return(bp->b_resid);
	return(bp->b_wcount);
}

#ifndef	RAW_BUFFER_POOL
tmread(dev)
{
	physio(tmstrategy, &rtmbuf, dev, B_READ);
}

tmwrite(dev)
{
	physio(tmstrategy, &rtmbuf, dev, B_WRITE);
}
#endif

#ifdef	RAW_BUFFER_POOL
tmread(dev)
{
	physio(tmstrategy, 0, dev, B_READ);
}

tmwrite(dev)
{
	physio(tmstrategy, 0, dev, B_WRITE);
}
#endif
