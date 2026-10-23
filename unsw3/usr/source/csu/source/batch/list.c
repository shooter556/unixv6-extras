#include	"batch.h"
/*
 *	this piece of code will produce listings of decks.
 *	it is invoked by "%list", and each kronos eor/eof
 *	implies a page skip (handled by printer driver).
 *
 *				daveh.	march '78
 */

char	null[1]	0;
main( argc, argv )
char	**argv;
{
	register struct quelt	*q;
	register int	n;
	int	buf[256];

	if ( fstat(3, buf) < 0 ) exit();

	for (;;)
	{
		if ((q = qget(list)) == 0)
			exit(0);
		q->bgtime = dtime();

		if ( q->ndfile )	/* append data file to pgm file */
		{
			if ( nopen(q->npfile, 2) < 0 )
			{
				prints("List: no program file\n");
				goto out;
			}
			if ( nopen(q->ndfile, 0) < 0 )
			{
				prints("List: no data file\n");
				close(0);
				goto out;
			}
			seek(0, 0, 2);	/* to eof */
			write(0, " \014", 2);	/* form feed */
			while ( n = read(1, buf, 512) ) write(0, buf, n);
			close(0); close(1);
			nunlink(q->ndfile);
			q->ndfile = 0;
		}
	out:	q->nofile = q->npfile;
		q->npfile = 0;
		q->ndtime = dtime();
		qput(q, print);
	}
}
