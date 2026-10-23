#

/*
 * I/O routines for slup
 */

#include "slup.h"

/*
 * Load an empty buffer when reading
 */
fload(iop)
register struct ubuf	*iop;
{
	iop->io_ptr = iop->io_buf;
	iop->io_cnt = read(iop->io_fid, iop->io_ptr, sizeof iop->io_buf);
	if (--iop->io_cnt < 0) {
		iop->io_cnt = 0;
		return(-1);
	} else
		return(*iop->io_ptr++ & 0377);
}


/*
 * Empty a buffer when writing
 */
fflush(iop)
register struct ubuf	*iop;
{
	register int	lwant, ldid;

	iop->io_ptr = iop->io_buf;
	lwant = (sizeof iop->io_buf) - iop->io_cnt;
	iop->io_cnt = sizeof iop->io_buf;
	ldid = write(iop->io_fid, iop->io_ptr, lwant);
	return((lwant == ldid) ? NOERR : ERRIO);
}

/*
 * OPen an input file
 */
fopen(nm, iop)
char	*nm;
register struct ubuf	*iop;
{
	iop->io_cnt = 0;
	return(((iop->io_fid = open(nm, 0)) < 0) ? ERRIO : NOERR);
}

/*
 * Open an output file
 */
fcreat(nm, iop)
char	*nm;
register struct ubuf	*iop;
{
	iop->io_ptr = iop->io_buf;
	iop->io_cnt = sizeof iop->io_buf;
	return(((iop->io_fid = creat(nm, 0600)) < 0) ? ERRIO : NOERR);
}

/*
 * Word I/o routines
 */
getw(iop)
register struct ubuf	*iop;
{
	register int	i, j;

	if ((i = getc(iop)) < 0 ||
	    (j = getc(iop)) < 0)
		return(-1);
	return(i | (j << 8));
}

putw(w, iop)
int	w;
register struct ubuf	*iop;
{
	struct {
		char	lobyte;
		char	hibyte;
		};

	if (putc(w.lobyte, iop) < 0 ||
	    putc(w.hibyte, iop) < 0)
		return(ERRIO);
	return(NOERR);
}

/*
 * Open a new file in update (read/write) mode
 */
openu(nm)
char	*nm;
{
	register int	f;

	if ((f = creat(nm, 0600)) >= 0) {
		close(f);
		if ((f = open(nm, 2)) >= 0)
			return(f);
	}
	return(ERRIO);
}

/*
 * Copy large slab of file#1 to file#2
 */
copyf(filin, filout, leng)
int	filin, filout;
register long	leng;
{
	register unsigned int	l;
	int	bigbuf[256];	/* fast i/o buffer */

	while (leng != 0) {
		l = leng > (sizeof bigbuf) ? (sizeof bigbuf) : leng;
		if (read(filin, bigbuf, l) != l ||
		    write(filout, bigbuf, l) != l)
			return(ERRIO);
		leng =- l;
	}
	return(NOERR);
}

/**********************************************************************/

struct	bmap {
	unsigned int	b_base;
	unsigned int	b_size;
	} *bmap;

int	bsegs;
int	mxbsegs;

/*
 * Initialise block map
 */
binit()
{
	register char	*p;

	p = alloc(50 * (sizeof *bmap));
	if (p == -1)
		return(ERRMEM);
	bmap = p;
	mxbsegs = 50;

	bsegs = 1;
	p->b_base = 0;
	p->b_size = 2000;

	return(NOERR);
}

/*
 * Allocate some blocks on the scratch file
 */
balloc(size)
long	size;
{
	register int	i;
	register struct bmap	*bp;
	register unsigned int	l;
	long	ll;

	l = (size + 511) / 512;
	for (i = 0, bp = bmap; i < bsegs; i++, bp++)
		if (bp->b_size >= l) {
/***/			ll = bp->b_base;
/***/			blkbase = ll * 512;
			bp->b_base =+ l;
			if ((bp->b_size =- l) == 0) {
				for (i++; i < bsegs; i++, bp++) {
					bp->b_base = bp[1].b_base;
					bp->b_size = bp[1].b_size;
				}
				bsegs--;
			}
			lseek(scrfid, blkbase, 0);
			return(NOERR);
		}
	abort();
}

/*
 * Release some blocks on the scratch file
 */
bfree(base, size)
long	base, size;
{
	register int	i;
	register struct bmap	*bp, *bpx;
	unsigned int	bbase, bsize, btop;
	char	*p;

	bbase = base / 512;
	bsize = (size + 511) / 512;
	btop = bbase + bsize;
	if (bsize == 0)
		return(NOERR);

	if (bsegs == mxbsegs) {
		i = (mxbsegs + 20) * (sizeof *bmap);
		if ((p = alloc(i)) == -1)
			return(ERRMEM);
		for (i = 0, bp = p, bpx = bmap; i < bsegs; i++, bp++, bpx++) {
			bp->b_base = bpx->b_base;
			bp->b_size = bpx->b_size;
		}
		if (mxbsegs == 0)
			free(bmap);
		bmap = p;
		mxbsegs =+ 20;
	}
	for (i = 0, bp = bmap; i < bsegs; i++, bp++)
		if (bp->b_base > bbase)
			break;
	if (i!=0 && (bp[-1].b_base + bp[-1].b_size)==bbase) {
		/* fits over the last one */
		bp[-1].b_size =+ bsize;
		if (btop == bp->b_base) {
			/* joins the two */
			bp[-1].b_size =+ bp->b_size;
			for (bsegs--; i < bsegs; i++, bp++) {
				bp->b_base = bp[1].b_base;
				bp->b_size = bp[1].b_size;
			}
		}
	} else if (btop == bp->b_base) {
		/* fits under this one */
		bp->b_base = bbase;
		bp->b_size =+ bsize;
	} else {
		/* must insert new entry */
		for (bp = &bmap[bsegs]; i < bsegs; i++, bp--) {
			bp->b_base = bp[-1].b_base;
			bp->b_size = bp[-1].b_size;
		}
		bsegs++;
		bp->b_base = bbase;
		bp->b_size = bsize;
	}
	return(NOERR);
}
