#

/*
 *	Optimized disk driver for
 *	Datum Inc controller with ampex disks.
 *
 *	Resembles 4 RK05j's in terms of capacity
 *	and configuration except no overlapped seek
 *	and logical drives are a bit funny ...
 *
 *	Looks like:	-1-3-+-3-1-	(remov)
 *			     |
 *			-0-2-+-2-0-	(fixed)
 *
 *
 *	I actually interchange drives 1 & 2 to allow
 *	contiguous disks to be implemented in a sensible manner !
 *	(i.e. drives 0,1 fixed; drives 2,3 removable)
 */

#include "../param.h"
#include "../systm.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define NAX	8	/* number of drives on system */
#define INTLV	2	/* magic interleaving number - see code */

#define AXADDR	0177460	/* base address of ampex control registers */

#define NAXSEC	12	/* 12 sectors per track */
#define NTRACK	2	/* 2 tracks per cylinder */
#define	NCYLIND	(NAXSEC * NTRACK)	/* 24 blocks per cylinder */
#define NAXBLK	4872	/* 4872 blocks per platter */
#define NAXSKIP 10	/* number of times io can be preempted */

/* control register bits */
#define CRESET	0	/* control reset */
#define GO	01
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

struct devtab	axtab;

struct {
	int	axds;
	int	axer;
	int	axcs;
	int	axwc;
	int	axba;
	int	axda;
	};

struct buf	raxbuf;

char	axfmt[NAX] { 0 };

char	axmap[];	/* remap physical drives 1 & 2 */

/*
 *	structure of the AX disk queue
 */

struct {
	int	ax_lcyl;		/* last cylinder accessed */
	char	ax_dirf;		/* direction of head motion  */
	} ax11;

/*
 *	Use av_back in the buffer header to save cylinder address;
 *	b_sector to save sector within cylinder; b_scratch to save
 *	entire disc address.
 */

#define axcyl	av_back.integ
#define	axsec	b_sector
#define axdad	b_scratch
#define	axignore	b_error

/*
 * Check the validity of the minor number
 */
axopen(dev, flag)
{
	register	i1, i2;

	i1 = dev.d_minor;
	if (((i1 & 07) + ((i1 >> 3) & 07)) >= NAX)
		u.u_error = EIO;
	else
		axfmt[dev.d_minor & 07] = 0;
	return(0);
}

/*
 *	axstrategy() does all the block mapping for
 *	all varieties of AX formats supported by this driver.
 *	The differing formats are distinguished by the minor device
 *	number used in referencing the disk:
 *
 *	minor device 00-07,10-16,20-25,30-34,40-43,50-52,60-61,70:
 *			Contiguous 'disk' from drive 'n' to drive 'n'+'m'
 *			where minor device is (m*10 + n).
 *	minor device 0100+other:
 *			re-mapped to slightly optimize disk access by unix
 */
axstrategy(bp)
register struct buf	*bp;
{
	register char *p1, *p2;
	int f;

	p1 = bp->b_dev.d_minor;
	p2 = (((p1 >> 3) & 07) + 1) * NAXBLK;
	/*
	 *	check for validity of this request
	 */
	if (bp->b_blkno >= p2) {
		bp->b_flags =| B_ERROR;
		iodone(bp);
		return;
	}
	p2 = bp->b_blkno;
	/*
	 *	Here we do the mapping for the various formats:
	 */
	p1 = (p1 & 07) + (p2 / NAXBLK);
	p2 =% NAXBLK;
	if (bp->b_dev.d_minor&0100 && p2!=0)
		/*
		 * We do the strange rotation of the
		 * virtual disk block numbers here.
		 */
		if (p2 > (NAXBLK/2))
			p2 =- (NAXBLK/2);
		else
			p2 =+ (NAXBLK/2) - 1;

	bp->av_forw = NULL;
	bp->axignore = NAXSKIP;
			/*
			 * axignore counts the number of times
			 * this io has been skipped in the queue.
			 * when it becomes 0 it cannot be ignored
			 * anymore. NAXSKIP should be set to the
			 * average size of contiguous command files
			 * e.g. in bin...
			 */

	/*
	 *	calculate cylinder, sector, drive and surface for
	 *	this request and save same.
	 */

	/*
	 * The Ampex disk appears as two disks per surface.
	 */
	bp->axcyl = p2 / NCYLIND;
	bp->axsec = p2 % NAXSEC;
	p2 =/ NAXSEC;
	bp->axdad = (axmap[p1] << 13) | (p2 << 4) | bp->axsec;
	spl5();
	if ((p1 = axtab.d_actf) == NULL) {
		/* queue was empty */
		axtab.d_actf = bp;
		if (!axtab.d_active && (AXADDR->axcs & RDY))
			axstart();
	} else {
		/*
		 * non-empty; determine where to place this request
		 * in the queue, taking into account current
		 * direction of head motion and minimization of
		 * head movement.
		 */
		f = ax11.ax_dirf;
		for (p2 = p1->av_forw; p2 != NULL; p2 = p2->av_forw)
			if (p2->axignore == 0)
				p1 = p2;
		for (; (p2 = p1->av_forw) != NULL; p1 = p2) {
			if (p1->axcyl<=bp->axcyl && bp->axcyl<=p2->axcyl ||
			    p1->axcyl>=bp->axcyl && bp->axcyl>=p2->axcyl) {
				while (p1->axcyl == p2->axcyl) {
					/*
					 * if a cylinder match is found,
					 * do rotational optimization.
					 * INTLV is set to 2 because system
					 * timing constraints dictate
					 * a two-block gap between
					 * sequential blocks to be read
					 * or written for optimum efficiency.
					 */
					if (p2->axsec > p1->axsec) {
						if (bp->axsec > p1->axsec+INTLV &&
						    bp->axsec < p2->axsec-INTLV)
							break;
					} else {
						if (bp->axsec > p1->axsec+INTLV ||
						    bp->axsec < p2->axsec-INTLV)
							break;
					}
					p1 = p2;
					if ((p2 = p1->av_forw) == NULL)
						break;
				}
				break;
			}
			if (f) {
				if (p2->axcyl < p1->axcyl)
					if (bp->axcyl > p1->axcyl)
						break;
					else
						f = 0;
			} else {
				if (p2->axcyl > p1->axcyl)
					if (bp->axcyl < p1->axcyl)
						break;
					else
						f++;
			}
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
		for (; p2 != NULL; p2 = p2->av_forw)
			if (p2->axignore != 0)
				p2->axignore--;
	}
	spl0();
}

/*
 *	axstart() goes through all the queues and
 *	sets everybody seeking that should be.
 */
axstart()
{
	register struct buf	*bp;

	if ((bp = axtab.d_actf) != NULL) {
		devstart(bp, &AXADDR->axda, bp->axdad, axfmt[bp->b_dev.d_minor & 07]);
		if (bp->axcyl > ax11.ax_lcyl)
			ax11.ax_dirf = 1;
		else if (bp->axcyl < ax11.ax_lcyl)
			ax11.ax_dirf = 0;
		ax11.ax_lcyl = bp->axcyl;
		bp->b_error = 0;
		axtab.d_active++;
	}
}

axintr()
{
	register n;
	register int *bp;

	if (AXADDR->axcs < 0) {	/* error bit */
		if (AXADDR->axer & WLO)
			n = 1;
		else if (!(AXADDR->axds & DRY))
			n = 2;
		else {
			n = 0;
			bp = &AXADDR->axda;
			printf("axds=%o,er=%o,cs=%o,wc=%o,ba=%o,da=%o\n",
				*--bp, *--bp, *--bp, *--bp, *--bp, *bp);
		}
		if (n)
			printf("ax%d - %s\n", axmap[(AXADDR->axda >> 13) & 07], n==1 ? "write-protected" : "off-line");
		AXADDR->axcs = CRESET|GO;
		if (axtab.d_actf != NULL)
			axtab.d_active = 0;
		bp = axtab.d_actf;
		if (++axtab.d_errcnt==10 || n) {
			bp->b_flags =| B_ERROR;
			axtab.d_actf = bp->av_forw;
			axtab.d_errcnt = 0;
			iodone(bp);
		}
		while (!(AXADDR->axcs & RDY));
		axstart();
	} else if (axtab.d_active) {
		/* we get here if this is an I/O completion interrupt */
		bp = axtab.d_actf;
		axtab.d_actf = bp->av_forw;
		axtab.d_active = 0;
		axtab.d_errcnt = 0;
		axstart();
		iodone(bp);
	}
}

/*
 *	Raw interfaces. Note that the raw interface does NOT work with
 *	any of the "funny" AX formats, because mapping must be done on
 *	a block-by-block basis, However, one may still use the raw device
 *	interface for such applications as backups even when the disk in
 *	question is a shuffled one, since all one wants to do in such a
 *	case is copy the disk directly without caring at all what's there
 *	and in what order. Raw device files should only be made with
 *	minor devices zero through NAX.
 */
axread(dev)
{
	physio(axstrategy, &raxbuf, dev, B_READ);
}

axwrite(dev)
{
	physio(axstrategy, &raxbuf, dev, B_WRITE);
}

/*
 * Set device control register
 */
axsgtty(dev, v)
register int	*v;
{
	register char	*s;

	if (v != NULL)
		return(1);	/* gtty */
	s = &axfmt[dev.d_minor & 07];
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
