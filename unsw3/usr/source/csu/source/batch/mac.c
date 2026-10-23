#include	"batch.h"
/*
 *	this code invoked for the sole purpose of executing 
 *	'macro-11' batch jobs.
 *					ianj.	march '76
 *					daveh.	march '78
 */
char	null[1]		0;
char	m1[]		"\n\n*** assembly failed: job terminated\n";
char	m2[]		"\n\n*** link edit failed: job terminated\n";
char	m3[]		"\n\n\t-------------------------------------------------\n\n";
char	m4[]		"*** macro jobs not allowed\n";
char	m5[]		"*** can't find macro assembler ***\n";
char	m6[]		"*** can't find macro linker ***\n";
char	cmsg[]		"\n\n*** Job cancelled by operator\n";
char	listing[]	"raft.lst";
char	object[]	"raft.obj";
char	monitor[]	"m.obj";
char	execute[]	"m.out";
char	prdmp[]		".sdump";
char	smacro[]	"/bin/macro";
char	slinkr[]	"/bin/linkr";
char	tmsg[]		"\n\n      seconds in execution\n";
#define	optmacro	"-ls", "-na:raft", "-xs:3", 
#define	optlinkr 

struct {
	long	putime,
		pstime,
		cutime,
		cstime;
} timbuf;

long	oldtime;

main(argc,argv)
char	**argv;
{
	int	status[2];
	register struct quelt	*q;
	int	statbuf[18];
	extern	fout;
	register	n;
	register char	*p;

	if ( fstat(3, statbuf) < 0) exit(1);

	/*
	 *	process next job 
	 */
	for (;;)
	{
		close(2);
		if ((q = qget(macro)) == 0)
			exit(0);	/* obtain the next job */
		q->bgtime = dtime();  /* record start of execution time */
		if (nomac)
		{
			creat(listing, 0600);
			write(0, m4, sizeof m4);
			close(0);
			goto finish;
		}
		gfnm(q->npfile);
		/*
	 	 *	produce 'object' and 'listing' from assembly
		 *	of 'fnm'. that is the users source program.
		 */
		if ( !fork() )
		{
			execl(smacro, smacro, optmacro fnm, 0);
			error(smacro, " can't exec\n");
		}
		waitx(status);
		if (status[0].highbyte == -1)
		{
			creat(listing, 0600);
			write(0, m5, sizeof m5);
			close(0);
			goto finish;
		}
		/*
		 *	setup standard output.
		 */
		open(listing, 2);	/* should have fd=0 */
		seek(0, 0, 2);
		write(0, m3, sizeof m3);	/* separator */
		dup(0); 		/* should have fd=1 */
		dup(0); close(0);	/* should have fd=2 */
		seek(2, 0, 2); seek(1, 0, 2);
		if (status[0].highbyte)
		{
			write(1, m1, sizeof m1);
			goto finish;
		}
		/*
		 * 	create 'execute' from 'object'
		 */
		if ( !fork() )
		{
			execl(slinkr, slinkr, optlinkr object, monitor, 0);
			error(slinkr, " can't exec\n");
		}
		waitx(status);
		if (status[0].highbyte == -1)
		{
			write(1, m6, sizeof m6);
			goto finish;
		}
		write(1, m3, sizeof m3);	/* separator */
		if (status[0].highbyte)
		{
			write(1, m2, sizeof m2);
			goto finish;
		}
		/*
		 *	run the users program
	 	 */
		if ( !q->tlimit ) goto finish;
		times(&timbuf);
		oldtime = timbuf.cutime + timbuf.cstime;
		if ( !(n = fork()) )
		{
			close(3);	/* save these from prying */
			if (q->ndfile) nopen(q->ndfile, 0); /* fd=0 */
			tlimit(q->tlimit); setuid(255);
			execl(execute, "local-mac", 0);
			error("can't exec output of linkr\n", null);
		}
		lmacpid = n;
		waitx(status);
		times(&timbuf);
		n = (timbuf.cutime + timbuf.cstime - oldtime) / 50;
		lmacpid = 0;
		if (status[0] & 0200)
		{
			if (!fork())
			{
				execl(prdmp, prdmp, "core", execute, 0);
				error(prdmp, " can't exec print dump\n");
			}
			waitx(status);
		}
		/*
		 *	complete the handling of the current job
		 */
		seek(1, 0, 2);		/* to eof */
		if ((status[0] & 0177) == sigkil)
			write(1, cmsg, sizeof cmsg);
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
		link(listing, fnm);
		unlink(listing); unlink(execute); unlink(object);
		q->ndtime = dtime();
		qput(q, print);
	}
}
error(a, b)
{
	prints("mac: ");  prints(a); prints(b);
	exit(-1);
}
