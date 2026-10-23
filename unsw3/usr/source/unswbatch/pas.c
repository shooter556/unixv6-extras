#include  "batch.h"
/*
 *	this code invoked for the sole purpose of executing 
 *	'pascal' batch jobs.
 *
 */
/*
#define IN_SESSION
       /*
	* make daemon collect jobs for 20 mins before executing
	* them. for building moral fortitiude in students and
	* provoking better debugging and saving cyber jobs and paper
	*/
char null[1] 0;
char m1[] "\n\n*** compilation errors: job terminated\n";
char m2[] "\n\n*** time limit: job terminated\n";
#define	SIGCPUTL	16
char ssource[]	"botch.p";
char execute[] "obj";
char spi[]  "/bin/pi";
char sexec[]  "/bin/px";
#define optpc "-l"
#define TIMELIMIT 30
char nullfile[] "/dev/null";

#ifdef IN_SESSION
offagen()
{
	signal(1,offagen);
}
#endif IN_SESSION


main(argc,argv)
char **argv; int argc;
{	int status[1]; register struct quelt *q; register n,m;
	register int pid;
	extern fout;

#ifdef IN_SESSION
	signal(1,offagen);
#endif IN_SESSION
	if(fstat(3,cbuf) < 0) exit();
	/*
	 *	process next job 
	 */
	nice(4);
	for(;;)
	{
		close(2);
		q=qget(pc);	/* obtain the next job */
		q->bgtime = dtime();  /* record start of execution time */
		gfnm(q->npfile);
		/*
	 	 *	produce 'object' and 'listing' from compilation
		 *	of 'fnm'. that is the users source program.
		 */
		if( !(pid = fork()) )
		{
			tlimit(60);
			link(fnm,ssource);
			open(nullfile, 0);	/* std input */
			q->nofile[0] = crfnm();	/* std output */
			dup(1);
			execl(spi,spi+9,optpc,ssource,0);
			error(spi," can't exec\n");
		}
		while(pid != waitx(status));
		unlink(ssource);
		/*
		 *	setup standard output.
		 */
		gfnm(q->nofile[0]);
		open(fnm, 1);		/* should have fd=0 */
		seek(0, 0, 2);
		dup(0); 		/* should have fd=1 */
		dup(0); close(0);	/* should have fd=2 */

		if(status[0].highbyte) {
		      write(1,m1,sizeof m1);
		      goto finish;
		}

		/*
		 *	execute users program
		 */
		write(1, "\014", 1);
		if( !(pid = fork()) ) {
			close(3);
			if(q->ndfile)	nopen(q->ndfile, 0);	/*std input*/
			    else	open(nullfile,0);
			setuid(-1);
			tlimit(TIMELIMIT);
			execl(sexec, sexec+9, execute, 0);
			error(sexec," can't exec\n");
		}
		while(pid != waitx(status));
		if( (status & 0177) == SIGCPUTL )
		{
			write(1, m1, sizeof m1);
			close(creat("core", 0606));
		}

		/*
		 *	complete the handling of the current job
		 */
	finish:	close(1);
		close(creat(execute, 0606));
		if(q->ndfile) nunlink(q->ndfile);
		nunlink(q->npfile);
		q->npfile=q->ndfile=0;
		q->ndtime = dtime();
		qput(q,print);
#ifdef IN_SESSION
	if(qhdr[pc].qnum == 0) sleep(1063);
			/* sleep for a while to allow jobs to collect */
#endif
	}
}
error(a,b)
{
	prints("pc: ");  prints(a); prints(b);
       exit(-1);
}
