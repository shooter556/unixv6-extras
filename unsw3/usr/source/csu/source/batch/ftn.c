#include	"batch.h"
/*
 *	this code invoked for the sole purpose of executing 
 *	'fortran' jobs.
 *
 *					daveh.	march '78
 */
char	null[1]		0;
char	m1[]		"\n\n*** compilation errors: job terminated\n";
char	m2[]		"\n\n*** link errors: job terminated\n";
char	m3[]		"\n\n\t-------------------------------------------------\n\n";
char	m4[]		"\n\n*** time limit exceeded; job terminated\n";
char	m5[]		"*** fortran jobs not allowed\n";
char	m6[]		"*** can't find fortran compiler ***\n";
char	m7[]		"*** can't find fortran linker ***\n";
char	listing[]	"-l=fort.lst";
char	object[]	"-o=fort.obj";
char	execute[]	"fort.out";
char	fortran[]	"/bin/fortran";
char	slink[]		"/bin/link";
char	tmsg[]		"\n\n      seconds in execution\n";
char	cmsg[]		"\n\n*** job cancelled by operator\n";
#define	optfort		"-ls:3", "-st", "-xs:3", "-wr", listing, object,
#define	optlink		"-p1", "-ns",

struct	{
	long	putime,
		pstime,
		cutime,
		cstime;
} timbuf;

long	oldtime;

main(argc, argv)
char	**argv;
{
	register struct quelt	*q;
	extern	fout;
	char	cbuf[36];
	register	n;
	register char	*p;

	if (fstat(3, cbuf) < 0) exit(1);

	/*
	 *	process next job 
	 */

	for (;;)
	{
		close(2);
		if ((q = qget(ftn)) == 0)
			exit(0);	/* obtain the next job */
		q->bgtime = dtime();  /* record start of execution time */

		if (noftn)		/* no fortran jobs */
		{
			creat(listing+3,0600);
			write(0, m5, sizeof m5);
			close(0);
			goto finish;
		}

		gfnm(q->npfile);
		/*
	 	 *	produce 'object' and 'listing' from compilation
		 *	of 'fnm'. that is the users source program.
		 */
		if ( !fork() )
		{
			execl(fortran, fortran+5, optfort fnm, 0);
			error(fortran, " can't exec\n");
		}
		waitx(cbuf);
		if (cbuf[1] == -1)	/* no ftn ? */
		{
			creat(listing+6, 0600);
			write(0, m6, sizeof m6);
			close(0);
			goto finish;
		}
		/*
		 *	setup standard output.
		 */
		open(listing+3, 2);	/* should have fd=0 */
		seek(0, 0, 2);
		write(0, m3, sizeof m3);	/* separator */
		dup(0); 		/* should have fd=1 */
		dup(0); close(0);	/* should have fd=2 */
		seek(2, 0, 2); seek(1, 0, 2);

		if (cbuf[1])
		{
			write(1, m1, sizeof m1);
			goto finish;
		}

		/*
		 * 	create 'execute' from 'object'
		 */
		if ( !fork() )
		{
			execl(slink, slink+5, optlink object+3, "-f4", 0);
			error(slink, " can't exec\n");
		}
		waitx(cbuf);
		if (cbuf[1] == -1)
		{
			write(1, m7, sizeof m7);
			goto finish;
		}
		write(1, m3, sizeof m3);	/* separator */
		if (cbuf[1])
		{
			write(1, m2, sizeof m2);
			goto finish;
		}

		/*
		 *	run the users program
	 	 */
		if (!q->tlimit ) goto finish;
		times(&timbuf);
		oldtime = timbuf.cutime + timbuf.cstime;
		if ( !(n = fork()) )
		{
			close(3);	/* save these from prying */
			if (q->ndfile) nopen(q->ndfile, 0); /* fd=0 */
			setuid(255);
			tlimit(q->tlimit);
			execl(execute, "local-ftn", 0);
			error("can't exec output of link\n",null);
		}
		lftnpid = n;
		waitx(cbuf);
		times(&timbuf);
		n = (timbuf.cutime + timbuf.cstime - oldtime) / 50;
		lftnpid = 0;
		/*
		 *	complete the handling of the current job
		 */
		seek(1, 0, 2);		/* eof */
		if (( cbuf[0] & 0177) == SIGTLMT )
			write(1, m4, sizeof m4);	/* time limit */
		else
			if ((cbuf[0] & 0177) == sigkil)
				write(1, cmsg, sizeof cmsg);	/* cancelled */
		p = &tmsg[7];
		do
			*--p = n%10 + '0';
		while (n =/ 10);
		write(1, tmsg, sizeof tmsg);
finish:		close(1);
		if (q->ndfile) nunlink(q->ndfile);
		nunlink(q->npfile);
		q->nofile = q->npfile;
		q->npfile = q->ndfile = 0;
		gfnm(q->nofile);
		link(listing+3, fnm);
		unlink(listing+3); unlink(execute); unlink(object+3);
		q->ndtime = dtime();
		qput(q, print);
	}
}
error(a, b)
{
	prints("ftn: ");  prints(a); prints(b);
	exit(-1);
}
