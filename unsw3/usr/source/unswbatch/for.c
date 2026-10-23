#include  "batch.h"
/*
 *	this code invoked for the sole purpose of executing 
 *	'fortran' jobs.
 */
char null[1] 0;
char m1[] "\n\n*** compilation errors: job terminated\n";
char m2[] "\n\n*** link errors: job terminated\n";
char m3[] "\n\n\t-------------------------------------------------\n\n";
char listing[] "-l=fort.lst";
char object[]  "-o=fort.obj";
char execute[] "fort.out";
char sfortran[]	"/bin/fortran";
char slink[]  "/bin/link";
#define optfort "-ls:3", "-st", "-nv", "-i4", listing, object,
#define	optlink "-p1", "-ns",

main(argc,argv)
char **argv; int argc;
{
	register struct quelt *q;
	extern fout;
	if(fstat(3,cbuf) < 0) exit();

	/*
	 *	process next job 
	 */
 for(;;){
	close(2);
	q=qget(fortran);	/* obtain the next job */
	q->bgtime = dtime();  /* record start of execution time */
	gfnm(q->npfile);
	/*
 	 *	produce 'object' and 'listing' from compilation
	 *	of 'fnm'. that is the users source program.
	 */
	if( !fork() ) {
		tlimit(300);
		execl(sfortran, sfortran+5, optfort fnm, 0);
		error(sfortran, " can't exec\n");
		}
	wait(cbuf);
	/*
	 *	setup standard output.
	 */
	open(listing+3,2);	/* should have fd=0 */
	seek(0,0,2);
	write(0,m3,sizeof m3);	/* separator */
	dup(0); 		/* should have fd=1 */
	dup(0); close(0);	/* should have fd=2 */
	seek(2,0,2); seek(1,0,2);

	if(cbuf[1]) {
	      write(1,m1,sizeof m1);
	      goto finish;
	}

	/*
	 * 	create 'execute' from 'object'
	 */
	if( !fork() ) {
		tlimit(300);
		execl(slink, slink+5, optlink object+3, "-f4", 0);
		error(slink, " can't exec\n");
		}
	wait(cbuf);
	write(1,m3,sizeof m3);	/* separator */
	if(cbuf[1]) {
	      write(1,m2,sizeof m2);
	      goto finish;
	}

	/*
	 *	run the users program
 	 */
	if( !fork() ) {
		   close(3);	/* save these from prying */
		   if(q->ndfile) nopen(q->ndfile,0); /* fd=0 */
		   setgid(255); setuid(255);
		   tlimit(30);
		   execl(execute,0);
		   error("can't exec output of link\n ",null);
		      }
	/*
	 *	allow only 30 seconds execution time in execution
	 */
	wait(cbuf);
	/*
	 *	complete the handling of the current job
	 */
finish:	close(1);
	if(q->ndfile) nunlink(q->ndfile);
	nunlink(q->npfile);
	q->nofile[0]=q->npfile;
	q->npfile=q->ndfile=0;
	gfnm(q->nofile[0]);
	link(listing+3,fnm);
	unlink(listing+3); unlink(execute); unlink(object+3);
	q->ndtime = dtime();
	qput(q,print);
	}
}
error(a,b)
{
	prints("for: ");  prints(a); prints(b);
       exit(-1);
}
