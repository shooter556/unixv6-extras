#include	"batch.h"
/*
 *	this piece of code will produce listings of decks.
 *	it is invoked by "%list", and each kronos eor/eof
 *	implies a page skip (handled by printer driver).
 */

char	null[1]	0;
main( argc, argv ) char **argv;
{
	register struct quelt *q;
	register int n;
	int buf[256];

	if(fstat(3,cbuf) < 0) exit();

	for( ;; ){
		q = qget(list);
		q->bgtime = dtime();

		if( q->ndfile ){	/* append data file to pgm file */
			if( nopen(q->npfile,2)<0 ) error("no prog file\n",null);
			if( nopen(q->ndfile,0)<0 ) error("no data file\n",null);
			seek(0, 0, 2);	/* to eof */
			write(0, "\014", 1);	/* form feed */
			while( n=read(1, buf, 512) ) write(0, buf, n);
			close(0); close(1);
			nunlink(q->ndfile);
		}
		q->nofile[0] = q->npfile;
		q->npfile = 0;
		q->ndtime = dtime();
		qput(q, print);
	}
}
error(a, b) char *a, *b;
{
	prints("list: ");
	prints(a); prints(b);
	exit(-1);
}
