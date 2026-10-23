#include "batch.h"

#ifdef	QEI
#define	COM1C	'L'
#define	COM1I	1
#define	COM2C	'J'
#define	COM2I	6
#define	COM3C	'R'
#define	COM3I	10
#define	ID1	20
#define	ID2	21
#define	ID3	22
#endif

#ifdef	NOS
int	ei_logfd;
int	start_up;
char	ei_log[]	"/spool/batch/ei.log";
char	dq[]		"/bin/dq";
int	finish();
#endif


/*
 *	files:-
 *		0 = commands down
 *		1 = log file
 *		2 = monitor
 *		3 = chkpt
 */


main()
{
	char bufar[1000];
	int num;
	register n;
	register char *cbuff;
	register struct quelt *pq;

	if( local ) exit();	/* daveh */
	if(fstat(3,ccbuf) < 0) exit();
	cbuff = bufar;

#ifdef	NOS
	if ( (ei_logfd = open( ei_log, 1 )) >= 0 )
	{
		signal( BATCHTERMINATE, finish );
		signal( SIGTERMINATE, finish );
		seek( ei_logfd, 0, 2 );
		write( ei_logfd, "\n", 1 );
		ptime();
		write( ei_logfd, "  Batch started on ", 19 );
		pday();
		write( ei_logfd, "\n\t  Unix logged in: ", 20 );
		start_up++;
	}
#endif

	for(;;)	
	{
  readit:
		n = read(0,cbuff,sizeof bufar);
#ifdef	QEI
		if((cbuff[COM1I]==COM1C)&&(cbuff[COM2I]==COM2C)&&(cbuff[COM3I]==COM3C))
		{
			pp(qlock);
			if( (pq = qhdr[atcyber].qptr) ) 
			do 
			{
				if( (!pq->cjobn[0])
				  && (pq->jobn[0]==cbuff[16])
				  && (pq->jobn[1]==cbuff[17])
				)
				{
					pq->cjobn[0]=cbuff[ID1];
					pq->cjobn[1]=cbuff[ID2];
					pq->cjobn[2]=cbuff[ID3];
					vv(qlock);
					goto readit;
				}
			}
			while( pq = pq->qlink );
			vv(qlock);
		} 
#endif
		cbuff[n]='\n';
		num = n;
		n = 49;
		while( n < num )
		{
			cbuff[n] = '\n';
			n =+ 50;
		}
		write(2,cbuff,num + 1);
#ifdef	NOS
		if ( (ei_logfd >= 0) && (cbuff[1] == '*') )	/* E/I crash */
		{
			seek( ei_logfd, 0, 2 );
			if ( !start_up )
			{
				ptime();
				write( ei_logfd, "  Export/Import restart: ", 25 );
			}
			write( ei_logfd, cbuff, num+1 );
			if ( !start_up )
			{
				if ( !fork() )
				{
					execl( dq, dq+5, 0 );
					write( 2, "Cannot exec dq!\n", 16 );
					exit( -1 );
				}
				waitx( cbuff );
			}
			else
				start_up = 0;
		}
#endif
	}
}


#ifdef	NOS
ptime()
{
	write( ei_logfd, ijtime()+11, 8 );
}

pday()
{
	write( ei_logfd, ijtime(), 10 );
}

finish()
{
	seek( ei_logfd, 0, 2 );
	ptime();
	write( ei_logfd, "  Batch finished on ", 20 );
	pday();
	write( ei_logfd, "\n", 1 );
	exit( 0 );
}
#endif	NOS
