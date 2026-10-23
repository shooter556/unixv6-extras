/*
 *	qlook - a program to scan the ckp file.
 *
 *					daveh.	march '78
 */

struct quelt
{
	struct quelt	*qlink;
	char		jobn[7];
	char		cjobn[3];
	int		pad[5];
	char		origin;
	char		qtype;
	int		tlimit;
	int		npfile;
	int		ndfile;
	int		nofile;
} elt;

struct qhdr
{
	struct quelt	*qptr;
	int		qnum;
} hdr;

int	filen, njobs, ncybjobs;
long	basetime;

struct
{
	int	integ;
};

#define	nqt	8
#define	nqs	100
#define	qeltlen	32
#define	fullckp	"/spool/batch/.work/.checkpoint"
#define	free	0
#define skip	012	/* stuff at start of file */

#define offset	026		/* offset in ckp file */

char	*qtyp[nqt]
{
	"free",
	"forcy",
	"print",
	"atcyb",
	"macro",
	"ftn",
	"list",
	"shell",
};
extern	fout;

main()
{
	register int	i;
	int	fd;

	fout = dup(1);
	if ( (fd=open(fullckp, 2)) < 0 )
	{
		prints(2, "qlook: file open error\n");
		exit(1);
	}

	read(fd, &filen, 2);	read(fd, &njobs, 2);
	read(fd, &ncybjobs, 2);	read(fd, &basetime, 4);

	printf("File: %s; Njobs: %l; Ncybjobs: %l; Basetime: %s\n",
		conv(filen), njobs, ncybjobs, ctime(basetime));

	printf(" addr   link jobname cyb flg  q   pgm     data    list   time\n\n");
	for ( i = 0; i < nqs; i++)
	{
		if ( read(fd, &elt, qeltlen) < 0 ) exit(1);
		if ( elt.qtype != free || elt.qlink.integ == 1 )
		{
			printf("%5o - %4o %7.7s/%3.3s %3o %2d\
 %7.7s %7.7s %7.7s %5l\n",
				i*qeltlen+skip+offset,
				elt.qlink, elt.jobn, elt.cjobn,
				elt.origin & 0377, elt.qtype & 0377,
				conv(elt.npfile), conv(elt.ndfile),
				conv(elt.nofile), elt.tlimit);
		}
	}

	printf("\n\n       addr   num\n\n");
	for (i = 0; i < nqt; i++ )
	{
		if ( read(fd, &hdr, 4) != 4 ) exit(1);
		printf("%-5.5s %5o - %3d\n", qtyp[i], hdr.qptr, hdr.qnum);
	}
	close(fd);
	flush();
	exit(0);
}
