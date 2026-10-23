#

/*
 *	Optimized disk driver for
 *	Pertec series D3000 drive (quad).
 *
 *	Resembles 8 rk05j's (but no overlapped seek)
 *	both in capacity and configuration.
 */

#include "../param.h"
#include "../systm.h"
#include "../buf.h"
#include "../conf.h"
#include "../user.h"

#define NPT	8	/* number of drives on system */
#define INTLV	2	/* magic interleaving number - see code */

#define PTADDR	0177460	/* base address of Pertec control registers */

#define NPTSEC	12	/* 12 sectors per track */
#define NTRACK	2	/* 2 tracks per cylinder */
#define	NCYLIND	(NPTSEC * NTRACK)	/* 24 blocks per cylinder */
#define NPTBLK	4872	/* 4872 blocks per platter */
#define NPTSKIP 10	/* number of times io can be preempted */

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

struct devtab	pttab;

struct {
	int	rkds;
	int	rker;
	int	rkcs;
	int	rkwc;
	int	rkba;
	int	rkda;
	};

struct buf	rptbuf;

char	ptfmt[NPT] { 0 };

/*
 *	structure of the PT disk queue
 */

struct {
	int	pt_lcyl;		/* last cylinder accessed */
	char	pt_dirf;		/* direction of head motion  */
	} pt11;

/*
 *	Use av_back in the buffer header to save cylinder address;
 *	b_sector to save sector within cylinder; b_scratch to save
 *	entire disc address.
 */

#define ptcyl	av_back.integ
#define	ptsec	b_sector
#define ptdad	b_scratch
#define	ptignore	b_error

/*
 * Check the validity of the minor number
 */
ptopen(dev, flag)
{
	register	i1, i2;

	i1 = dev.d_minor;
	if (((i1 & 07) + ((i1 >> 3) & 07)) >= NPT)
		u.u_error = EIO;
	else
		ptfmt[dev.d_minor & 07] = 0;
	return(0);
}

/*
 *	Ptstrategy() does all the block mapping for
 *	all varieties of PT formats supported by this driver.
 *	The differing formats are distinguished by the minor device
 *	number used in referencing the disk:
 *
 *	minor device 00-07,10-16,20-25,30-34,40-43,50-52,60-61,70:
 *			Contiguous 'disk' from drive 'n' to drive 'n'+'m'
 *			where minor device is (m*10 + n).
 *	minor device 0100+other:
 *			re-mapped to slightly optimize disk access by unix
 */
ptstrategy(bp)
register struct buf	*bp;
{
	register char *p1, *p2;
	int f;

	p1 = bp->b_dev.d_minor;
	p2 = (((p1 >> 3) & 07) + 1) * NPTBLK;
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
	p1 = (p1 & 07) + (p2 / NPTBLK);
	p2 =% NPTBLK;
	if (bp->b_dev.d_minor&0100 && p2!=0)
		/*
		 * We do the strange rotation of the
		 * virtual disk block numbers here.
		 */
		if (p2 > (NPTBLK/2))
			p2 =- (NPTBLK/2);
		else
			p2 =+ (NPTBLK/2) - 1;

	bp->av_forw = NULL;
	bp->ptignore = NPTSKIP;
			/*
			 * ptignore counts the number of times
			 * this io has been skipped in the queue.
			 * when it becomes 0 it cannot be ignored
			 * anymore. NPTSKIP should be set to the
			 * average size of contiguous command files
			 * e.g. in bin...
			 */

	/*
	 *	calculate cylinder, sector, drive and surface for
	 *	this request and save same.
	 */

	/*
	 * The Pertec disk appears as two disks per surface;
	 * with the cylinders interleaved.
	 */
	bp->ptcyl = ((p2 / NCYLIND) << 1) | (p1 & 01);
	bp->ptsec = p2 % NPTSEC;
	p2 =/ NPTSEC;
	bp->ptdad = (p1 << 13) | (p2 << 4) | bp->ptsec;
	spl5();
	if ((p1 = pttab.d_actf) == NULL) {
		/* queue was empty */
		pttab.d_actf = bp;
		if (!pttab.d_active && (PTADDR->rkcs & RDY))
			ptstart();
	} else {
		/*
		 * non-empty; determine where to place this request
		 * in the queue, taking into account current
		 * direction of head motion and minimization of
		 * head movement.
		 */
		f = pt11.pt_dirf;
		for (p2 = p1->av_forw; p2 != NULL; p2 = p2->av_forw)
			if (p2->ptignore == 0)
				p1 = p2;
		for (; (p2 = p1->av_forw) != NULL; p1 = p2) {
			if (p1->ptcyl<=bp->ptcyl && bp->ptcyl<=p2->ptcyl ||
			    p1->ptcyl>=bp->ptcyl && bp->ptcyl>=p2->ptcyl) {
				while (p1->ptcyl == p2->ptcyl) {
					/*
					 * if a cylinder match is found,
					 * do rotational optimization.
					 * INTLV is set to 2 because system
					 * timing constraints dictate
					 * a two-block gap between
					 * sequential blocks to be read
					 * or written for optimum efficiency.
					 */
					if (p2->ptsec > p1->ptsec) {
						if (bp->ptsec > p1->ptsec+INTLV &&
						    bp->ptsec < p2->ptsec-INTLV)
							break;
					} else {
						if (bp->ptsec > p1->ptsec+INTLV ||
						    bp->ptsec < p2->ptsec-INTLV)
							break;
					}
					p1 = p2;
					if ((p2 = p1->av_forw) == NULL)
						break;
				}
				break;
			}
			if (f) {
				if (p2->ptcyl < p1->ptcyl)
					if (bp->ptcyl > p1->ptcyl)
						break;
					else
						f = 0;
			} else {
				if (p2->ptcyl > p1->ptcyl)
					if (bp->ptcyl < p1->ptcyl)
						break;
					else
						f++;
			}
		}
		bp->av_forw = p2;
		p1->av_forw = bp;
		for (; p2 != NULL; p2 = p2->av_forw)
			if (p2->ptignore != 0)
				p2->ptignore--;
	}
	spl0();
}

/*
 *	ptstart() goes through all the queues and
 *	sets everybody seeking that should be.
 */
ptstart()
{
	register struct buf	*bp;

	if ((bp = pttab.d_actf) != NULL) {
		devstart(bp, &PTADDR->rkda, bp->ptdad, ptfmt[bp->b_dev.d_minor & 07]);
		if (bp->ptcyl > pt11.pt_lcyl)
			pt11.pt_dirf = 1;
		else if (bp->ptcyl < pt11.pt_lcyl)
			pt11.pt_dirf = 0;
		pt11.pt_lcyl = bp->ptcyl;
		bp->b_error = 0;
		pttab.d_active++;
	}
}

ptintr()
{
	register n;
	register int *bp;

	if (PTADDR->rkcs < 0) {	/* error bit */
		if (PTADDR->rker & WLO)
			n = 1;
		else if (!(PTADDR->rkds & DRY))
			n = 2;
		else {
			n = 0;
			bp = &PTADDR->rkda;
			printf("ptds=%o,er=%o,cs=%o,wc=%o,ba=%o,da=%o\n",
				*--bp, *--bp, *--bp, *--bp, *--bp, *bp);
		}
		if (n)
			printf("pt%d - %s\n", (PTADDR->rkda >> 13) & 07, n==1 ? "write-protected" : "off-line");
		PTADDR->rkcs = CRESET|GO;
		if (pttab.d_actf != NULL)
			pttab.d_active = 0;
		bp = pttab.d_actf;
		if (++pttab.d_errcnt==10 || n) {
			bp->b_flags =| B_ERROR;
			pttab.d_actf = bp->av_forw;
			pttab.d_errcnt = 0;
			iodone(bp);
		}
		while (!(PTADDR->rkcs & RDY));
		ptstart();
	} else if (pttab.d_active) {
		/* we get here if this is an I/O completion interrupt */
		bp = pttab.d_actf;
		pttab.d_actf = bp->av_forw;
		pttab.d_active = 0;
		pttab.d_errcnt = 0;
		ptstart();
		iodone(bp);
	}
}

/*
 *	Raw interfaces. Note that the raw interface does NOT wopt with
 *	any of the "funny" PT formats, because mapping must be done on
 *	a block-by-block basis, However, one may still use the raw device
 *	interface for such applications as backups even when the disk in
 *	question is a shuffled one, since all one wants to do in such a
 *	case is copy the disk directly without caring at all what's there
 *	and in what order. Raw device files should only be made with
 *	minor devices zero through NPT.
 */
ptread(dev)
{
	physio(ptstrategy, &rptbuf, dev, B_READ);
}

ptwrite(dev)
{
	physio(ptstrategy, &rptbuf, dev, B_WRITE);
}

/*
 * Set device control register
 */
ptsgtty(dev, v)
register int	*v;
{
	register char	*s;

	if (v != NULL)
		return(1);	/* gtty */
	s = &ptfmt[dev.d_minor & 07];
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
