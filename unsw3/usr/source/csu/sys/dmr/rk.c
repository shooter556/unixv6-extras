#

/*
 *	Optimized RK-11/RK03/RK05/disk driver
 *
 *	Copyright (c) 1975, the Children's Museum.
 *	This program is proprietary. Permission is hereby
 *	granted to use this program and associated documentation
 *	by any UNIX licensee in accordance with the provisions of
 *	any UNIX licensing agreement. Other uses of
 *	of this program, or distribution of this program
 *	in violation of the provisions of the UNIX license,
 *	must be approved by the Children's Museum in writing.
 *
 *	Inquiries, bug notices, etc, should be addressed to
 *		Computer Centre
 *		The Children's Museum
 *		Jamaicaway
 *		Boston, MA 02130
 *		(617) 522-4800 x25
 *	Authors: Bill Mayhew and Brent Byer, September 1975
 *
 */

#include "../param.h"
#include "../systm.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define NRK	8	/* number of drives on system */
#define	NQUEUE	8	/* Number of seek queues */
#define INTLV	2	/* magic interleaving number - see code */

#define RKADDR	0177400	/* base address of RK11 control registers */

#define NRKSEC	12	/* 12 sectors per track */
#define NTRACK	2	/* 2 tracks per cylinder */
#define	NCYLIND	(NRKSEC * NTRACK)	/* 24 blocks per cylinder */
#define NRKBLK	4872	/* 4872 blocks per platter */
#define NRKSKIP 10	/* number of times io can be preempted */

/* control register bits */
#define CRESET	0	/* control reset */
#define GO	01
#define SEEK	010
#define DRESET  014	/* drive reset */
#define IDE	0100	/* interrupt enable */
#define RDY	0200	/* control ready */
#define SCP	020000	/* seek complete */

/* error register */
#define	WLO	020000	/* write lockout */
#define	OVR	040000	/* overrun */
#define	DRE	0100000	/* drive error */

/* drive status register */
#define	WPS	040	/* drive write protected */
#define	DRY	0200	/* drive ready */
#define SIN	01000	/* seek incomplete */

struct devtab	rktab;

struct {
	int	rkds;
	int	rker;
	int	rkcs;
	int	rkwc;
	int	rkba;
	int	rkda;
	};

/*
 *	structure of an RK disk queue; one queue per drive.
 */

struct	rkq {
	struct buf	*rk_bufp;	/* pointer to first buffer in queue */
	int	rk_lcyl;		/* last cylinder accessed on this drive */
	char	rk_dirf;		/* direction of head motion  */
	char	rk_seek;		/* seeking flag */
	int	rk_errcnt;		/* error count */
	} rk_q[NQUEUE];


char	rkmap[NRK] { 0, 1, 2, 3, 4, 5, 6, 6, };

struct rkq	*rkqmap[NRK] {
	&rk_q[0],
	&rk_q[1],
	&rk_q[2],
	&rk_q[2],	/* none */
	&rk_q[2],	/* none */
	&rk_q[2],	/* none */
	&rk_q[3],
	&rk_q[3],
	};
struct rkq	*rk_ap;			/*
					 * pointer to queue of disk
					 * currently doing I/O transfer
					 */

char	rkfmt[NRK] { 0 };

/*
 *	Use av_back in the buffer header to save cylinder address;
 *	b_sector to save sector within cylinder; b_scratch to save
 *	entire disc address.
 */

#define rkcyl	av_back.integ
#define	rksec	b_sector
#define rkdad	b_scratch
#define	rkignore	b_error

/*
 * Check the validity of the minor number
 */
rkopen(dev, flag)
{
	register	i1, i2;

	i1 = dev.d_minor;
	if (((i1 & 07) + ((i1 >> 3) & 07)) >= NRK)
		u.u_error = EIO;
	else
		rkfmt[dev.d_minor & 07] = 0;
	return(0);
}

/*
 *	Rkstrategy() does all the block mapping for
 *	all varieties of RK formats supported by this driver.
 *	The differing formats are distinguished by the minor device
 *	number used in referencing the disk:
 *
 *	minor device 00-07,10-16,20-25,30-34,40-43,50-52,60-61,70:
 *			Contiguous 'disk' from drive 'n' to drive 'n'+'m'
 *			where minor device is (m*10 + n).
 *	minor device 0100+other:
 *			re-mapped to optimize disk access by UNIX
 */
rkstrategy(bp)
register struct buf	*bp;
{
	register char *p1, *p2;
	int f;

	p1 = bp->b_dev.d_minor;
	p2 = (((p1 >> 3) & 07) + 1) * NRKBLK;
	/*
	 *	check for validity of this request
	 */
	if (bp->b_blkno >= p2) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	f = p2 >> 1;
	p2 = bp->b_blkno;
	if (p1&0100 && p2!=0)
		/*
		 * We do the strange rotation of the
		 * virtual disk block numbers here.
		 */
		if (p2 > f)
			p2 =- f;
		else
			p2 =+ f - 1;
	p1 = (p1 & 07) + (p2 / NRKBLK);
	p2 =% NRKBLK;
	/*
	 *	Here we do the mapping for the various formats:
	 */

	bp->av_forw = NULL;
	bp->rkignore = NRKSKIP;
			/*
			 * rkignore counts the number of times
			 * this io has been skipped in the queue.
			 * when it becomes 0 it cannot be ignored
			 * anymore. NRKSKIP should be set to the
			 * average size of contiguous command files
			 * e.g. in bin...
			 */

	/*
	 *	calculate cylinder, sector, drive and surface for
	 *	this request and save same.
	 */

	bp->rkcyl = p2 / NCYLIND;
	bp->rksec = p2 % NRKSEC;
	p2 =/ NRKSEC;
	bp->rkdad = (p1 << 13) | (p2 << 4) | bp->rksec;
	/*
	 * The RK05-F disk appears as two concentric disks.
	 * This particular piece of code can handle any
	 * number of concentric disks.
	 */
	bp->rkcyl =+ (p1 - rkmap[p1]) * NCYLIND;
	p2 = rkqmap[p1.integ];
	spl5();
	if ((p1 = p2->rk_bufp) == NULL) {
		/* queue was empty */
		p2->rk_bufp = bp;
		if ((rk_ap == NULL) && (RKADDR->rkcs & RDY))
			rkstart();
	} else {
		/*
		 * non-empty; determine where to place this request
		 * in the queue, taking into account current
		 * direction of head motion and minimization of
		 * head movement.
		 */
		f = p2->rk_dirf;
		for (p2 = p1->av_forw; p2 != NULL; p2 = p2->av_forw)
			if (p2->rkignore == 0)
				p1 = p2;
		for (; (p2 = p1->av_forw) != NULL; p1 = p2) {
			if (p1->rkcyl<=bp->rkcyl && bp->rkcyl<=p2->rkcyl ||
			    p1->rkcyl>=bp->rkcyl && bp->rkcyl>=p2->rkcyl) {
				while (p1->rkcyl == p2->rkcyl) {
					/*
					 * if a cylinder match is found,
					 * do rotational optimization.
					 * INTLV is set to 2 because system
					 * timing constraints dictate
					 * a two-block gap between
					 * sequential blocks to be read
					 * or written for optimum efficiency.
					 */
					if (p2->rksec > p1->rksec) {
						if (bp->rksec > p1->rksec+INTLV &&
						    bp->rksec < p2->rksec-INTLV)
							break;
					} else {
						if (bp->rksec > p1->rksec+INTLV ||
						    bp->rksec < p2->rksec-INTLV)
							break;
					}
					p1 = p2;
					if ((p2 = p1->av_forw) == NULL)
						break;
				}
				break;
			}
			if (f) {
				if(p2->rkcyl < p1->rkcyl)
					if(bp->rkcyl > p1->rkcyl)
						break;
					else
						f = 0;
			} else {
				if(p2->rkcyl > p1->rkcyl)
					if(bp->rkcyl < p1->rkcyl)
						break;
					else
						f++;
			}
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
		/* mark overtaken blocks */
		for (; p2 != NULL; p2 = p2->av_forw)
			if (p2->rkignore != 0)
				p2->rkignore--;
	}
	spl0();
}

/*
 *	rkstart() goes through all the queues and
 *	sets everybody seeking that should be.
 */
rkstart()
{
	register struct buf	*bp;
	register struct rkq	*qp;

	for (qp = &rk_q[0]; qp < &rk_q[NQUEUE]; qp++) {
		bp = qp->rk_bufp;
		if (bp!=NULL && !qp->rk_seek) {
			RKADDR->rkda = bp->rkdad;
			RKADDR->rkcs = IDE|SEEK|GO;
			if (bp->rkcyl > qp->rk_lcyl)
				qp->rk_dirf = 1;
			else if (bp->rkcyl < qp->rk_lcyl)
				qp->rk_dirf = 0;
			qp->rk_lcyl = bp->rkcyl;
			qp->rk_seek++;
			bp->b_error = 0;
			while (!(RKADDR->rkcs & RDY));
		}
	}
}

rkintr()
{
	register n;
	register struct rkq	*qp;
	register int *bp;
	int errflag;

	if (RKADDR->rkcs < 0) {	/* error bit */
		if (RKADDR->rker & WLO)
			errflag = 1;
		else if (!(RKADDR->rkds & DRY))
			errflag = 2;
		else {
			errflag = 0;
			bp = &RKADDR->rkda;
			printf("rkds=%o,er=%o,cs=%o,wc=%o,ba=%o,da=%o\n",
				*--bp, *--bp, *--bp, *--bp, *--bp, *bp);
		}
		if (RKADDR->rkds & SIN) {
			/* Seek Incomplete */
			n = (RKADDR->rkds >> 13) & 07;
			RKADDR->rkda = rkmap[n] << 13;
			RKADDR->rkcs = IDE|DRESET|GO;
			qp = rkqmap[n];
			qp->rk_lcyl = 0;
			qp->rk_seek = 0;
			n = 0;
		} else {
			/* other errors */
			n = (RKADDR->rkda >> 13) & 07;
			RKADDR->rkcs = IDE|CRESET|GO;
			if (errflag)
				printf("rk%d - %s\n", n, errflag==1 ? "write-protected" : "off-line");
			for (qp = &rk_q[0]; qp < &rk_q[NQUEUE]; qp++)
				if (qp->rk_bufp != NULL)
					qp->rk_seek = 0;
			qp = rkqmap[n];
			n = 1;
		}
		bp = qp->rk_bufp;
		rk_ap = NULL;
		if (++(qp->rk_errcnt)==10 || errflag) {
			bp->b_flags =| B_ERROR;
			qp->rk_bufp = bp->av_forw;
			qp->rk_errcnt = 0;
			iodone(bp);
		}
		if (n) {
			while (!(RKADDR->rkcs & RDY));
			rkstart();
		}
	} else if (RKADDR->rkcs & SCP) {
		/* overlapped seek operation complete */
		n = (RKADDR->rkds >> 13) & 07;
		if (n>=NRK || rk_ap!=NULL) {
			printf("rk softerr\n");
			return;
		}
		qp = rkqmap[n];
		if (qp->rk_seek) {
			qp->rk_seek = 0;
			rk_ap = qp;
			bp = qp->rk_bufp;
			bp->b_error = 0;
			devstart(bp, &RKADDR->rkda, bp->rkdad, rkfmt[bp->b_dev.d_minor & 07]);
		} else
			rkstart();	/* drive reset */
	} else if (rk_ap != NULL) {
		/* we get here if this is an I/O completion interrupt */
		n = (RKADDR->rkda >> 13) & 07;
		bp = rk_ap->rk_bufp;
		qp = rkqmap[n];
		qp->rk_bufp = bp->av_forw;
		rk_ap = NULL;
		qp->rk_errcnt = 0;
		rkstart();
		iodone(bp);
	}
}

/*
 *	Raw interfaces. Note that the raw interface does NOT work with
 *	any of the "funny" RK formats, because mapping must be done on
 *	a block-by-block basis, However, one may still use the raw device
 *	interface for such applications as backups even when the disk in
 *	question is a shuffled one, since all one wants to do in such a
 *	case is copy the disk directly without caring at all what's there
 *	and in what order. Raw device files should only be made with
 *	minor devices zero through NRK.
 */
rkread(dev)
{
	physio(rkstrategy, NULL, dev, B_READ);
}

rkwrite(dev)
{
	physio(rkstrategy, NULL, dev, B_WRITE);
}

/*
 * Set device control register
 */
rksgtty(dev, v)
register int	*v;
{
	register char	*s;

	if (v != NULL)
		return(1);	/* gtty */
	s = &rkfmt[dev.d_minor & 07];
	switch (u.u_arg[0]) {

	case 0:	/* reset */
		*s = 0;
		break;

	case 1:	/* format on i/o */
		*s = 04;
		break;

	case 2:	/* format on i/o with no increment */
		*s = 014;
		break;
	}
	return(0);
}
