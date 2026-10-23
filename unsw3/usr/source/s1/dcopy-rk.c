#
#include <local-system>
/*
 *	disk copier and compacter
 *	J. N. Rottman
 *
 *	NOTE:
 *	1) Boostrap block is copied.
 *	2) Size of destination file system (in particular no inodes)
 *	   is used i.e it is not duplicated. An initial pass is thus
 *	   made to check if copied filesystem fits !!
 *	3) Disk errors are also copied, so dcheck and icheck aforehand.
 *	4) Will work for HP and MS disks only (agsm and eecf respectively).
 *
 *	C. H. Maltby esq.
 */

#define	KFH

/*
 *	5) Will now work for any disk (e.g. RK).  The algorithm was
 *	   plagiarised from `icheck.c'.
 *
 *	K. F. Hill (failed)
 */

#include <defines.h>
#include <param.h>
#include <filsys.h>
#include <ino.h>

#ifdef	EECF
#define	IN	10	/* magic interleave number */
			/*
			 *	W A R N I N G
			 *	*************
			 *
			 * IN and NSEC  MUST be relatively prime!!!!
			 * (that's what he thinks - KFH)
			 */
#define NSEC	21	/* number of sectors per track */
#endif	EECF
#ifdef	AGSM
#define	IN	7	/* magic interleave number */
			/*
			 *	W A R N I N G
			 *	*************
			 *
			 * IN and NSEC  MUST be relatively prime!!!!
			 * (that's what he thinks - KFH)
			 */
#define NSEC	22	/* number of sectors per track */
#endif	AGSM

#ifdef	KFH
#ifdef	EECF1140
#define	IN	3	/* magic interleave number */
#define	NSEC	24	/* number of sectors per cylinder - RK */
#endif	EECF1140
#endif	KFH

#ifdef	EECF & AGSM | EECF & EECF1140 | AGSM & EECF1140
more than one is defined .... this should not bee ;;
#endif

#define NTRACK	19	/* number of tracks per cylinder */
#define	NPOOL	50	/* size of output buffer pool */


struct	filsys	super0;
struct	filsys	super1;
struct	inode	node0;
struct	inode	node1;

struct	direct	{
	int	inum;
	char	name[14];
};

struct	inode	in_node[16];
struct	inode	out_node[16];

int	inum	16;
int	iblk	1;
int	onum	0;
int	oblk	2;
int	iunit;
int	*iindex;
int	fi;
int	fo;
int	lastblock;
int	inext	1;
int	in[256];
int	ind0[256];
int	ind1[256];
char	*blkno;


struct	{
	int	bn;
	int	bb[256];
} bpl[NPOOL];

int inpool;


#ifdef	KFH
char	adr[NSEC], flag[NSEC];
#endif	KFH


main(argc, argv)
char **argv;
{
	register i, *indx, icount;
	extern int *sbrk();

	if( argc != 3 )
	{
		printf("Usage: dcopy infilesys outfilesys\n");
		return(1);
	}
	if((fi = open(argv[1], 0)) < 0) {
		printf("Cannot open input.\n");
		return(1);
	}
	if((fo = open(argv[2], 2)) < 0) {
		printf("Cannot open output.\n");
		return(1);
	}
	blkno = argv[0];
#ifdef	KFH
	sync();
#endif	KFH
	getblk(fi, &super0, 0);
	putblk(&super0, 0);
	getblk(fi, &super0, 1);

	getblk( fo, in, 1);
	super1.s_isize = in->s_isize;
	super1.s_fsize = in->s_fsize;
	super1.s_time = super0.s_time;

	if( counti() > super1.s_isize)
	{
		printf("Too few inodes\n");
		return(1);
	}
	if( (super0.s_fsize - (super0.s_isize + countfree()))
		> (super1.s_fsize - super1.s_isize) )
	{
		printf("Insufficient free space\n");
		return(1);
	}

#ifdef	KFH
	icount = 0;
	for (i = 0; i < NSEC; i++)
	{
		while (flag[icount]) icount = (icount + 1) % NSEC;
		adr[i] = icount;
		flag[icount]++;
		icount = (icount + IN) % NSEC;
	}
#endif	KFH

	indx = sbrk(super0.s_isize * 2 * 16);
	iindex = indx - 1;
	icount = 16*super0.s_isize;


	iunit = fi;

	i = icount;
	do {
		geti();
		if(node0.i_mode & IALLOC) {
			copyi();
			*indx++ = inext++;
		} else
			*indx++ = 0;
	} while(--i);

	ipurge();

	for(i=1; i < inext; i++) {
		geti();
		if((node0.i_mode & IFMT) == IFDIR)
			reldir();
	}

	for(i = inext; i <= icount; i++)
		ifree(i);

	freelist();

	putblk(&super1, 1);
	purgepool();
	sync();
	return(0);
}

getblk(f, b, n)
{
	seek(f, n, 3);
	if(read(f, b, 512) != 512)
		printf("Input error: block %u\n", n);
}

putblk(b, n)
register int *b;
{
	register int *w;
	register int q;

	if(inpool == NPOOL)
		purgepool();
	bpl[inpool].bn = n;
	w = bpl[inpool++].bb;
	q = 256;
	do
		*w++ = *b++;
	while(--q);
}

purgepool()
{
	register int i;

	for(i=0; i < inpool; i++) {
		seek(fo, bpl[i].bn, 3);
		if(write(fo, bpl[i].bb, 512) != 512)
			printf("Output error: block %u\n", bpl[i].bn);
	}
	inpool = 0;
}

geti()
{
	register *r, *w, q;

	if(inum == 16) {
		getblk(iunit, in_node, ++iblk);
		decnum(blkno, iblk);
		inum = 0;
	}
	w = &node0;
	r = &in_node[inum++];
	q = 16;
	do 
		*w++ = *r++;
	while(--q);
}

decnum(s, n)
register char *s;
register int n;
{
	if(n/10 != 0)
		s = decnum(s, n/10);
	*s++ = n%10 + '0';
	*s = 0;
	return(s);
}

puti()
{
	register *r, *w, q;

	w = &out_node[onum];
	r = &node1;
	q = 16;
	do 
		*w++ = *r++;
	while(--q);
	if(++onum == 16) {
		putblk(out_node, oblk++);
		onum = 0;
	}
}

ipurge()
{
	register *r, *w, q;

	if(onum) {
		r = &out_node[onum];
		q = 16*(16-onum);
		do
			*r++ = 0;
		while(--q);
		putblk(out_node, oblk++);
	}

	r = out_node;
	q = 256;
	do
		*r++ = 0;
	while(--q);

	while(oblk < 2 + super1.s_isize)
		putblk(out_node, oblk++);

	iblk = 1;
	inum = 16;
	iunit = fo;
	purgepool();
}

nxtblk()
{
	static int cyl, sector, track;
	register char *block;

	for(;;) {
#ifndef	KFH
		lastblock = block = (((cyl * NTRACK) + track) * NSEC) + sector;

		if((sector =+ IN) >= NSEC)
			sector =- NSEC;
		if( (sector == 0) && (++track >= NTRACK) )
		{
			cyl++;
			track = 0;
		}
#else
		lastblock = block = (cyl * NTRACK + track) * NSEC + adr[sector];

		if (++sector >= NSEC)
		{
			sector = 0;
			if (++track >= NTRACK)
			{
				cyl++;
				track = 0;
			}
		}
#endif	KFH
		if(block < 2 + super1.s_isize)
			continue;
		return(block);
	}
}

freelist()
{
	register cyl, sector, track;
	extern int ldivr;
	char *block;

	cyl = ldiv(0, super1.s_fsize, NTRACK*NSEC);
	track = ldivr / NSEC;
	sector = ldivr % NSEC;
	bfree(0);			/* end-of-list sentinel */

	while( sector )
		bfree( (((cyl * NTRACK) + track) * NSEC) + --sector );
	for(;;) {
		if(sector == 0 && track-- == 0)
		{
			track =+ NTRACK;
			cyl--;
		}
#ifndef	KFH
		if((sector =- IN) < 0)
			sector =+ NSEC;
		block = (((cyl * NTRACK) + track) * NSEC) + sector;
#else
		if (--sector < 0) sector = NSEC - 1;
		block = (cyl * NTRACK + track) * NSEC + adr[sector];
#endif	KFH
		if(block < 2 + super1.s_isize)
			continue;
		if(block == lastblock)
			break;
		bfree(block);
	}
}

ifree(i)
{
	if(super1.s_ninode != 100)
		super1.s_inode[super1.s_ninode++] = i;
}

bfree(b)
{
	register *r, *q, w;
	if(super1.s_nfree == 100) {
		r = in;
		*r++ = super1.s_nfree;
		q = super1.s_free;
		w = 100;
		do
			*r++ = *q++;
		while(--w);
		putblk(in, b);
		super1.s_nfree = 0;
	}
	super1.s_free[super1.s_nfree++] = b;
}

copyi()
{
	register int w, ww;
	int *r, *q;

	r = &node0;
	q = &node1;
	w = 16;
	do
		*q++ = *r++;
	while(--w);

	if(node1.i_mode & (IFCHR&IFBLK)) {
		puti();
		return;
	}

	if((node1.i_mode&ILARG) == 0) {
		for(w = 0; w < 8; w++) 
			if(node1.i_addr[w]) {
				getblk(fi, in, node1.i_addr[w]);
				node1.i_addr[w] = nxtblk();
				putblk(in, node1.i_addr[w]);
			}
		puti();
		return;
	}
	for(w = 0; w < 7; w++) 
		if(node1.i_addr[w]) {
			getblk(fi, ind0, node1.i_addr[w]);
			node1.i_addr[w] = nxtblk();
			for(ww = 0; ww < 256; ww++)
				if(ind0[ww]) {
					getblk(fi, in, ind0[ww]);
					ind0[ww] = nxtblk();
					putblk(in, ind0[ww]);
				}
			putblk(ind0, node1.i_addr[w]);
		}

	if(node1.i_addr[7]) {
		getblk(fi, ind0, node1.i_addr[7]);
		node1.i_addr[7] = nxtblk();
		for(w = 0; w < 256; w++)
			if(ind0[w]) {
				getblk(fi, ind1, ind0[w]);
				ind0[w] = nxtblk();
				for(ww = 0; ww < 256; ww++)
					if(ind1[ww]) {
						getblk(fi, in, ind1[ww]);
						ind1[ww] = nxtblk();
						putblk(in, ind1[ww]);
					}
				putblk(ind1, ind0[w]);
			}
		putblk(ind0, node1.i_addr[7]);
	}
	puti();
}

reldir()
{
	register int i, j;

	if((node0.i_mode & ILARG) == 0) {
		for(i=0; i < 8; i++)
			relx(node0.i_addr[i]);
		return;
	}
	for(i=0; i < 7; i++)
		if(node0.i_addr[i]) {
			getblk(fo, ind0, node0.i_addr[i]);
			for(j = 0; j < 256; j++)
				relx(ind0[j]);
		}
}

relx(b)
{
	register struct  direct *p;
	register int i;

	if(b != 0) {
		getblk(fo, in, b);
		p = in;
		i = 32;
		do {
			if(p->inum)
				p->inum = iindex[p->inum];
			p++;
		} while(--i);
		putblk(in, b);
	}
}

counti()
{
	register i,j,n;

	n = 0;
	for( i = 0; i < super0.s_isize; i++)
	{
		getblk(fi, &in_node, 2+i);
		for( j = 0; j < 16; j++)
		{
			if( in_node[j].i_mode & IALLOC )
				n++;
		}
	}
	return((n + 15) / 16);
}

countfree()
{
	register n,m;

	n = super0.s_nfree;
	if( n)
	{
		m = super0.s_free[0];
		while( m)
		{
			getblk( fi, in, m);
			n =+ in[0];
			m = in[1];
		}
	}
	return(n);
}
