#
/*
 *	QLOOK - A program to scan the checkpoint file.
 */

struct quelt
	{
	struct quelt *qlink;
	char jobn[7];
	char cjobn[3];
	int times[5];
	char origin,qtype;
	char *pf,*df,*of;
	} quelt;

struct qhdr
	{
	struct quelt *qptr;
	int qnum;
	}
	qhdr;

/*
 * prints...
 * core addr; link; names; origin; type; files
 * to convert core addr to file addr subtract 026
 */
#define qnumb 200
#define queltlen 30
#define free 0
#define nq 8

char *qtyp[nq]
	{
	"free",
	"forcy",
	"print",
	"atcyb",
	"ass",
	"fortran",
	"list",
	"pascal"
	};

extern int fout;
main()
{
	extern char *conv();
	register int i,fd;

	if( (fd = open("/spool/batch/.work/.checkpoint",0)) < 0)
	{
		prints(1, "File open error\n");
		exit(1);
	}
	fout = dup(1);
	seek(fd, 14, 0);	/*skip crap*/
	printf("addr link  name    ext    org    qtp  pgm     data    list\n\n");
	for(i = 0; i < qnumb; i++)
	{
		read(fd,&quelt,queltlen);
		if( quelt.qtype != free)
		{
			printf("%4o %4o; %7.7s/%3.3s; o:%3o; q:%3o; %7.7s",
				i*queltlen + 36, quelt.qlink, quelt.jobn, quelt.cjobn,
				quelt.origin, quelt.qtype, conv(quelt.pf) );
			printf(" %7.7s", conv(quelt.df) );
			printf(" %7.7s\n", conv(quelt.of) );
		}
	}
	printf("\n       addr   num\n\n");
	for(i = 0; i<nq; i++)
	{
		if( read(fd, &qhdr, 4) != 4) exit(1);
		printf("%-5.5s %5o - %3d\n",
			qtyp[i],
			qhdr.qptr,
			qhdr.qnum);
	}
	close(fd);
	flush();
	exit(0);
}
