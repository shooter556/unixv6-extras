#include "batch.h"
main()
{
	char ebuf[1000]; register n,k; register struct quelt *pq;
	int first;	/* first time switch */

	if( local || fstat(3, ebuf)<0 ) exit(1);
	first = 1;

	for(;;)
	{
readit:		n = read(0,ebuf,sizeof ebuf);
		/* 'l' 'j' 'r' are capital letters the cyber is not low enough */
		if((ebuf[1]=='L')&&(ebuf[6]=='J')&&(ebuf[10]=='R'))
		{
			pp(qlock);
			if( !(pq = qhdr[atcyber].qptr) ) goto over;
			do
			{
				if(!pq->cjobn[0]) 
					if((pq->jobn[0]==ebuf[16])&&(pq->jobn[1]==ebuf[17]))
					{
						pq->cjobn[0]=ebuf[20];
						pq->cjobn[1]=ebuf[21];
						pq->cjobn[2]=ebuf[22];
						vv(qlock);
						goto readit;
					}
			} while( pq = pq->qlink );
over:
			vv(qlock);
		} 

		if ( ebuf[0] == 'K' && ebuf[1] == 'R' && ebuf[2] == 'O'
		&& ebuf[3] == 'N' && !bsw && !first )	/* seems we lost export */
		{
			prints("\nAuto log on initiated ...\n");
			while(open("/dev/cyb.cmdsu",1) < 0 ) sleep(1);
			pp(qlock);
			kill(jtdpid, 1);
			kill(splpid, 1);
			while(write(1,"l,00elecb",9) != 9) sleep(1);
			close(1);
			vv(qlock);
		}
		else
		{
			ebuf[n]='\n';
			for(k= -1;k<n;) ebuf[k=+50] = '\n';
				write(2,ebuf,n+1);
		}
	first = 0;
	}
}
