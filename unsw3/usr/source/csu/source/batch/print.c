#include "batch.h"
/*
 *	this daemon will select jobs from the print q and
 *	pass them one by one to the line printer daemon.
 *
 *	the statistics file is updated from info in the q.
 *
 *					daveh.	march '78
 */

char	*sfile	"/usr/lpd/x/dfx0";
char	*sfile1	"/usr/lpd/tfx0";
char	*lockf	"/usr/lpd/lock";
char	*lpd	"/etc/lpd";

char	zeroes[40];

stopit()
{
	unlink(sfile1);
	exit(0);
}

main()
{
	register int	f;
	register struct quelt	*q;
	char	buf[36];

	if ( fstat(3, buf) < 0 || noprt ) exit(1);

	signal(SIGTERM, stopit);
	unlink(sfile1);	/* avoid possible hang */

	for (;;)
	{
		if ( stat(sfile, buf) == 0 && stat(lockf, buf) < 0 )	/* lpd not active but should be */
			if (!fork())
			{
				execl(lpd, lpd+5, 0);
				exit(1);
			}
			else
				waitx(buf);

		if ((q = qget(print)) == 0)
			exit(0);
		q->prtime = dtime();	/* note starting time */

		while (stat(sfile, buf) == 0 ) sleep(1);	/* wait for lpd to take file */

		f = creat(sfile1, 0600);

		if ( q->origin & tycyb )
			ncybjobs--;

		if (q->origin & (tyerr | tyloc))
			write(f, "B", 1);	/* use batchnote */
		else
			write(f, "*", 1);	/* cyber-generated banner page */

		write(f, q->jobn, 7);
		write(f, zeroes, 32);
		gfnm(q->nofile);
		write(f, "F", 1);
		write(f, ffnm, 39);
		write(f, "U", 1);
		write(f, ffnm, 39);

		close(f);
		link(sfile1, sfile);	/* rename to command file */
		unlink(sfile1);
		q->extime = dtime();
		write(1, q->jobn, statrln);
		qput(q, free);
	}
}
