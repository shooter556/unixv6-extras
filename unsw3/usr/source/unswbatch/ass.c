#include  "batch.h"
/*
 *	this code invoked for the sole purpose of executing 
 *	'macro-11' batch jobs.
 *
 */
#define IN_SESSION
#define	DELAY	2
       /*
	* Jobs will not be executed until 15 minutes have elapsed
	* from read time.
	* For building moral fortitude in students and
	* provoking better debugging and saving cyber jobs and paper
	*/
char null[1] 0;
char m1[] "\n\n*** assembly errors: job terminated\n";
char m2[] "\n\n*** load errors: job terminated\n";
char m3[] "\n\n\t-------------------------------------------------\n\n";
char listing[] "raft.lst";
char object[]  "raft.obj";
char monitor[] "m.obj";
char execute[] "m.out";
char core[]    "core";
char prdmp[]   ".sdump";
char smacro[]  "/bin/macro";
char slink[]  "/bin/link";
#define optmacro "-ls","-na:raft"
#define optlink  "-o=m.out"
#define TIMELIMIT 15

main(argc,argv)
char **argv; int argc;
{	int status[1]; register struct quelt *q; register n,m;
	register int pid;
	extern fout;

	if(fstat(3,cbuf) < 0) exit();
	/*
	 *	process next job 
	 */
	for(;;)
	{
		close(2);
		q=qget(assembl);	/* obtain the next job */
#		ifdef	IN_SESSION
			n = dtime() - q->rdtime;
			if( n < DELAY )
				sleep(n * 60);	/* minutes */
#		endif
		q->bgtime = dtime();  /* record start of execution time */
		gfnm(q->npfile);
		/*
	 	 *	produce 'object' and 'listing' from assembley
		 *	of 'fnm'. that is the users source program.
		 */
		if( !(pid = fork()) )
		{
			tlimit(60);
			open("/dev/null",2);		/*macro seems to need this*/
			dup(0);
			execl(smacro,smacro+5,optmacro,fnm,0);
			error(smacro," can't exec\n");
		}
		while(pid != waitx(status));
		/*
		 *	setup standard output.
		 */
		open(listing,2);	/* should have fd=0 */
		seek(0,0,2);
		dup(0); 		/* should have fd=1 */
		dup(0); close(0);	/* should have fd=2 */
		seek(2,0,2); seek(1,0,2);
	
		if(status[0].highbyte) {
		      write(1,m1,sizeof m1);
		      goto finish;
		}
		write(1,m3,sizeof m3);	/* separator */
	
		/*
		 * 	create 'execute' from 'object'
		 */
		if( !(pid = fork()) ) {
			   tlimit(60);
			   execl(slink,slink+5,optlink,object,monitor,0);
			   error(slink," can't exec\n");
			      }
		while(pid != waitx(status));
		if(status[0].highbyte) {
		      write(1,m2,sizeof m2);
		      goto finish;
		}
		write(1,m3,sizeof m3);	/* separator */
	
		/*
		 *	run the users program
	 	 */
		if( !(pid = fork()) ) {
			   close(3);	/* save these from prying */
			   if(q->ndfile) nopen(q->ndfile,0); /* fd=0 */
			   setgid(255); setuid(255);
			   tlimit(TIMELIMIT); /*give an execution limit of timelimit sec */
			   execl(execute,0);
			   error("can't exec output of link\n ",null);
			      }
		while(pid != waitx(status));
	
		if(status[0]&0200){
			if(!fork()){
				execl(prdmp,prdmp,0);
				error(prdmp," can't exec print dump\n");
			}
			waitx(status);
		}
		/*
		 *	complete the handling of the current job
		 */
	finish:	close(1);
		if(q->ndfile) nunlink(q->ndfile);
		nunlink(q->npfile);
		q->nofile[0]=q->npfile;
		q->npfile=q->ndfile=0;
		gfnm(q->nofile[0]);
		link(listing,fnm);
		unlink(listing);
		close(creat(execute,0705));
		close(creat(object,0604));
		close(creat(core,0606));
		q->ndtime = dtime();
		qput(q,print);
	}
}
error(a,b)
{
	prints("ass: ");  prints(a); prints(b);
       exit(-1);
}
