#include "batch.h"
/*
 *	this program transmits jobs to the cyber one at a time.	the jobs
 *	are queued for it by 'crd' or 'submit'.
 *
 *	fd:	0: input file
 *		0: commands up.
 *	 1: cyber->
 *	 2: terminal			ian j.	march '76
 *	 3: checkpoints
 */
char m1[] "\n\n\njtd: file open error\n";

int	re_flag;	/* for E/I crashes */


main(argc,argv)
  char **argv;
  int argc;
{
	register struct quelt *q;
	int register n, i;
	int buff[512];
	extern restart();

	if(fstat(3,ccbuf) < 0) exit();
	if( local ) exit();	/* daveh */
	signal( 1, restart );

	/*
	 *	keep on sending ........
	 */
	for(;;) {
		q=qget(forcyber);
		q->bgtime=dtime();	/* note time of selection */
		if( nopen(q->npfile,0)<0 ) {
			prints(m1); qput(q,free); continue;
		}
		do {
			if((n=read(0,buff,1024)))
				while ( (i = write( 1, &buff[1], buff[0] )) != buff[0] )
					if ( i== -1 && re_flag )
					{
						re_flag = 0;
						seek( 0, 0, 0 );
						n = 1024;
						break;
					}
			} while(n==1024);
		write(1,0,0);	/* wait till all gone */
		close(0);
		nunlink(q->npfile);	/* delete the file */
		q->npfile=0;		/* note this */
		qput(q,atcyber);	 /* add to cyber queue */
		while( open("/dev/cyb.cmdsu",1)<0 ) sleep(1);
#ifdef	QEI
		while( write(0,"q,n",3) != 3 ) sleep(1);
		do { sleep(1); } while( write(0,"r",1) != 1 ) ;
#endif
#ifndef	QEI
		while ( write( 0 , "r" , 1 ) != 1 )	sleep( 1 );
#endif
		close(0);
		}
}



restart()
{
	signal( 1, restart );
	re_flag++;
}
