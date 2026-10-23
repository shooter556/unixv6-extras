#
#include	<defines.h>
#include <param.h>
#include <local-system>

char	*dargv[]
{
#ifdef	EECF
	"rms00",
	"rms01",
	"rms02",
	"rms03",
	"rms10",
	"rms11",
	"rms12",
	"rms13",
#endif	EECF
	0
};

#define NINODE	16*16
#define	NI	20

#include <ino.h>
#include <filsys.h>

struct	inode	inode[NINODE];
struct	filsys	sblock;

int	tflg ;
int	headpr;

int ilist[NI] {-1};
int	fi;
char	*ecount;
char	*lasts;
int	ino;
int	nerror;
int	nfiles;
struct dir {
	int	ino;
	char	name[14];
};

main(argc, argv)
char **argv;
{
	register char **p;
	register int n, *lp;

	ecount = sbrk(0);
	if (argc == 1) {
		chdir("/dev");
		for (p = dargv; *p;)
			check(*p++);
		return(nerror);
	}
	while (--argc) {
		argv++;
		if (**argv=='-') switch ((*argv)[1]) {
		case 't':
			tflg++ ;
			continue ;

		case 'i':
			lp = ilist;
			while (lp < &ilist[NI-1] && (n = number(argv[1]))) {
				*lp++ = n;
				argv++;
				argc--;
			}
			*lp++ = -1;
			continue;

		default:
			printf("Bad flag\n");
		}
		check(*argv);
		ilist[0] = -1;		/* fix */
	}
	return(nerror);
}

check(file)
char *file;
{
	register i, j;
	fi = open(file, 0);
	if(fi < 0) {
		printf("cannot open %s\n", file);
		return;
	}
	headpr = 0;
	ino = 0;		/* fix */
	printf("%s:\n", file);
	sync();
	bread(1, &sblock, 512);
	nfiles = sblock.s_isize*16;
	if (lasts < nfiles) {
		if ((sbrk(nfiles - lasts + 1)) == -1) {
			printf("Not enough core\n");
			exit(04);
		}
		lasts = nfiles;
	}
	for (i=0; i<=nfiles; i++)
		ecount[i] = 0;
	for(i=0; ino<nfiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for(j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass1(&inode[j]);
		}
	}
	ino = 0;
	for (i=0; ino<nfiles; i =+ NINODE/16) {
		bread(i+2, inode, sizeof inode);
		for (j=0; j<NINODE && ino<nfiles; j++) {
			ino++;
			pass2(&inode[j]);
		}
	}
	if (tflg) saplook() ;
	close(fi) ;
}

pass1(aip)
struct inode *aip;
{
	register doff;
	register struct inode *ip;
	register struct dir *dp;
	int i;

	ip = aip;
	if((ip->i_mode&IALLOC) == 0)
		return;
	if((ip->i_mode&IFMT) != IFDIR)
		return;
	doff = 0;
	while (dp = dread(ip, doff)) {
		doff =+ 16;
		if (dp->ino==0)
			continue;
		for (i=0; ilist[i] != -1; i++)
			if (ilist[i]==dp->ino)
				printf("%5l arg; %l/%.14s\n", dp->ino, ino, dp->name);
		if( dp->ino > (sblock.s_isize*16) ) {
				printf("BAD inode %5l arg; %l/%.14s\n", dp->ino, ino, dp->name);
		} else ecount[dp->ino]++;
	}
}

pass2(aip)
{
	register struct inode *ip;
	register i;

	ip = aip;
	i = ino;
	if ((ip->i_mode&IALLOC)==0 && ecount[i]==0)
		return;
	if (ip->i_nlink==ecount[i] && ip->i_nlink!=0)
		return;
	if( ecount[i] > ip->i_nlink )
		nerror++;
	if (headpr==0) {
		printf("I-num  Dir-ent  Link-cnt\n");
		headpr++;
	}
	printf("%-5l      %3d       %3d\n", ino,
	    ecount[i]&0377, ip->i_nlink&0377);
}

#define	DTYPE	01	/* directory type i-node */
#define	DFOUND	02	/* found ref in another directory */
#define	DDONE	04	/* scanned this directory */

/*
 * look for disconnected branches
 */
saplook()
{
	register int	i, j, k ;
	int	done ;

	ino = 0 ;
	for (i = 0 ; ino < nfiles ; i =+ NINODE/16) {
		bread(i + 2, inode, sizeof inode) ;
		for (j = 0 ; j<NINODE && ino<nfiles ; j++) {
			ino++ ;
			setdflag(&inode[j]) ;
		}
	}

	if (!(ecount[1] & DTYPE)) {
		printf("Root i-node not a directory!\n") ;
		return ;
	}
	ecount[1] =| DFOUND ;	/* to start it off */

	do {
		for (i = ino = done = 0 ; i < nfiles ; i =+ NINODE/16) {
			bread(i + 2, inode, sizeof inode) ;
			for (j = 0 ; j<NINODE && ino<nfiles ; j++) {
				ino++ ;
				k = ecount[ino] ;
				if ((k & DTYPE) && (k & DFOUND) && !(k & DDONE)) {
					done++ ;	/* made a change this pass */
					passd(&inode[j]) ;
					ecount[ino] =| DDONE ;
				}
			}
		}
	} while (done != 0) ;

	for (i = 0 ; i < nfiles ; i++) {
		if (ecount[i]&DTYPE && !(ecount[i] & DFOUND)) {
			printf("orphan sapling %l\n", i) ;
		}
	}
}

/*
 * set flag if directory
 */
setdflag(ip)
register struct inode	*ip ;
{
	ecount[ino] = 0 ;
	if ((ip->i_mode & IALLOC) && (ip->i_mode & IFMT)==IFDIR) {
		ecount[ino] = DTYPE ;
	}
}

/*
 * scan a directory collecting i-numbers
 */
passd(ip)
register struct inode	*ip ;
{
	register struct dir	*dp ;
	register int	doff ;

	for (doff = 0 ; dp = dread(ip, doff) ; doff =+ 16) {
		if (dp->ino != 0) ecount[dp->ino] =| DFOUND ;
	}
}

dread(aip, aoff)
{
	register b, off;
	register struct inode *ip;
	static ibuf[256];
	static char buf[512];

	off = aoff;
	ip = aip;
	if ((off&0777)==0) {
		if (off==0177000) {
			printf("Monstrous directory %l\n", ino);
			return(0);
		}
		if ((ip->i_mode&ILARG)==0) {
			if (off>=010000 || (b = ip->i_addr[off>>9])==0)
				return(0);
			bread(b, buf, 512);
		} else {
			if (off==0) {
				if (ip->i_addr[0]==0)
					return(0);
				bread(ip->i_addr[0], ibuf, 512);
			}
			if ((b = ibuf[(off>>9)&0177])==0)
				return(0);
			bread(b, buf, 512);
		}
	}
	return(&buf[off&0777]);
}

bread(bno, buf, cnt)
{

	seek(fi, bno, 3);
	if(read(fi, buf, cnt) != cnt) {
		printf("read error %d\n", bno);
		exit();
	}
}

number(as)
char *as;
{
	register n, c;
	register char *s;

	s = as;
	n = 0;
	while ((c = *s++) >= '0' && c <= '9') {
		n = n*10+c-'0';
	}
	return(n);
}
