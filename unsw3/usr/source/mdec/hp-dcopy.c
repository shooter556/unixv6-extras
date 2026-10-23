#
/*
 *	HP disk copier and compacter
 *	J. N. Rottman
 */

#include "/user1/rottman/sys/filsys.h"
#include "/user1/rottman/sys/ino.h"

#define	IN	7	/* magic interleave number */
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


main(argc, argv)
char **argv;
{
	register i, *indx, icount;
	extern int *sbrk();

	if((fi = open(argv[1], 0)) < 0) {
		printf("Cannot open input.\n");
		return(1);
	}
	if((fo = open(argv[2], 2)) < 0) {
		printf("Cannot open output.\n");
		return(1);
	}
	blkno = argv[0];
	getblk(fi, &super0, 0);
	putblk(&super0, 0);
	getblk(fi, &super0, 1);

	super1.s_isize = super0.s_isize;
	super1.s_fsize = super0.s_fsize;
	super1.s_time = super0.s_time;

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

	while(oblk < 2 + super0.s_isize)
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
		lastblock = block = (((cyl * 19) + track) * 22) + sector;

		if((sector =+ IN) >= 22)
			sector =- 22;
		if(++track >= 19)
			track =- 19;
		if(sector == 0 && track == 0)
			cyl++;
		if(block < 2 + super0.s_isize)
			continue;
		return(block);
	}
}

freelist()
{
	register cyl, sector, track;
	extern int ldivr;
	char *block;

	cyl = ldiv(0, super0.s_fsize, 19*22);
	block = ldivr;
	sector = (block * 3) % 22;
	track = block % 19;
	bfree(0);

	for(;;) {
		if(sector == 0 && track == 0)
			cyl--;
		if((sector =- IN) < 0)
			sector =+ 22;
		if(--track < 0)
			track =+ 19;
		block = (((cyl * 19) + track) * 22) + sector;
		if(block < 2 + super0.s_isize)
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
