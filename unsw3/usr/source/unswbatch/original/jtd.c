#include "batch.h"
/*
 *    this program transmits jobs to the cyber one at a time.  the jobs
 *     are queued for it by 'crd' or 'submit'.
 *
 *    fd:  0: input file
 *	   0: commands up.
 *         1: cyber->
 *         2: terminal                       ian j.  march '76
 *         3: checkpoints
 */
char m1[] "\n\n\njtd: file open error\n";

main(argc,argv)
char **argv; int argc;
{     register struct quelt *q; int register n;
       int buff[512];
	if(fstat(3,cbuf) < 0) exit();
	if( local ) exit();	/* daveh */
       /*
        *    keep on sending ........
        */
       for(;;) {
               q=qget(forcyber);
               q->bgtime=dtime();       /* note time of selection */
		if( nopen(q->npfile,0)<0 ) {
			prints(m1); qput(q,free); continue;
		}
               do {
                   if((n=read(0,buff,1024)))
                         while(!write(1,&buff[1],buff[0]));  /* last 52 -> 64 bytes useless */
                  } while(n==1024);
	       write(1,0,0);	/* wait till all gone */
               close(0);
               nunlink(q->npfile);       /* delete the file */
               q->npfile=0;             /* note this */
               qput(q,atcyber);         /* add to cyber queue */
		while( open("/dev/cyb.cmdsu",1)<0 ) sleep(1);
		while( write(0,"q,n",3) != 3 ) sleep(1);
		do { sleep(1); } while( write(0,"r",1) != 1 ) ;
		close(0);
               }
}
