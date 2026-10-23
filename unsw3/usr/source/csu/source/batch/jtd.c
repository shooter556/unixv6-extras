#include "batch.h"

#define	sigon	signal(6, init); signal(7, cancel)
#define	sigoff	signal(6, 1); signal(7, 1)

/*
 *	this program transmits jobs to the cyber one at a time.  the jobs
 *	 are queued for it by 'crd' or 'submit'.
 *
 *	fd:	0: input file
 *		0: commands up.
 *		1: cyber->
 *		2: terminal			ianj.	march '76
 *		3: checkpoint			daveh.	march '78
 */

char m1[] "\n\n\njtd: file open error\n";

int	sig;	/* set to reinitialize in various ways */

init()	/* drop job & requeue it */
{
	signal(6, init);
	sig = 6;
}
cancel()	/* e,cr */
{
	signal(7, cancel);
	sig = 7;
}

main(argc, argv)
char	**argv;
{
	register struct quelt	*q;
	int register	n;
	int	buff[512];
	int	cupfd;

	if ( fstat(3, buff)<0 || local ) exit();

	sigoff;

	/*
	 *    keep on sending ........
	 */
	for (;;)
	{
		sig = 0;
		if ((q = qget(forcyber)) == 0)
			exit(0);
		q->bgtime = dtime();       /* note time of selection */
		if ( nopen(q->npfile, 0) < 0 )
		{
			prints(m1);
			qput(q, free);
			continue;
		}

		sigon;

		/* tell export we want to transmit a job */
		while ( (cupfd=open("/dev/cyb.cmdsu", 1)) < 0)
			sleep(1);
		while ( write(cupfd, "r", 1) != 1 ) sleep(1);
		close(cupfd);

		/* send job, discarding remnants of buffer */
		do
		{
			if ((n = read(0, buff, 1024)))
				while (!write(1, &buff[1], buff[0]));  /* last 52 -> 64 bytes useless */
		} while ( n == 1024 && !sig);
		sigoff;
		if ( sig )
		{
			close(0);
			if (sig == 7 )
			{
				nunlink(q->npfile);
				qput(q, free);
			}
			else
				qput(q, forcyber);

			continue;	/* back to outer loop */
		}
		write(1, 0, 0);		/* wait till all gone */
		close(0);
		nunlink(q->npfile);	/* delete the file */
		q->npfile = 0;		/* note this */
		qput(q, atcyber);	/* add to cyber queue */
		sigon;
		while ( (cupfd=open("/dev/cyb.cmdsu", 1)) < 0 )
			sleep(1);
		while ( write(cupfd, "q,n", 3) != 3 )
			sleep(1);	/* make it identify job just read */
		close(cupfd);

		sigoff;
	}
}
