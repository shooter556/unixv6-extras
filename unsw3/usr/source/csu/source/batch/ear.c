#include "batch.h"
/*
 *	listen to ravings of export - trap certain messages.
 *
 *					daveh.	march '78
 */

#define	msgbit	010000		/* filter bit in swr */

main()
{
	char	cbuf[1000];
	register	k;
	register char	*c;
	register struct quelt	*pq;
	int	n;
	int	first;	/* first time switch */

	if ( local || fstat(3, cbuf) < 0 ) exit(1);
	first = 1;
	for (;;)
	{
readit:		while ((n = read(0, cbuf, sizeof cbuf)) <= 0);
		if ((cbuf[1]=='L') && (cbuf[6]=='J') && (cbuf[10]=='R'))
		{
		/*
		 * last job read ...
		 * pick out name & see if we are waiting for it
		 */
			pp(qlock);
			if ( !(pq = qhdr[atcyber].qptr) ) goto over;
			do
			{
				if (!pq->cjobn[0]) 
					if ((pq->jobn[0] == cbuf[16]) && (pq->jobn[1] == cbuf[17]))
					{
						pq->cjobn[0] = cbuf[20];
						pq->cjobn[1] = cbuf[21];
						pq->cjobn[2] = cbuf[22];
						chkpt();
						vv(qlock);
						goto readit;
					}
			} while ( pq = pq->qlink );
	over:		vv(qlock);
			goto printit;
		} 

		for (c = cbuf; c < &cbuf[sizeof cbuf - 12]; c++)
			if ( c[0] == 'K' && c[1] == 'R' && c[2] == 'O'
			&& c[3] == 'N' && !bsw && !first )
			{
			/*
			 * 'kronos ...'
			 * seems export just rejoined us ...
			 * signal appropriate daemons.
			 */
				pp(qlock);
				prints("\nAuto log on initiated ...\n");
				if ( !fork() ) execl(".logon", ".logon", 0);
				else
					waitx(cbuf);
				prints("\nAuto log on completed !!!\n");
				kill(jtdpid, 6);
				kill(splpid, 6);
				sleep(5);
				vv(qlock);
				goto readit;
			}
		if ( cbuf[1] == 'T' && cbuf[2] == 'O' && cbuf[3] == 'O'
			&& (getcsw() & msgbit) )
		/*
		 * ' too many jobs - try later'
		 * if bit is set, then set switch, otherwise
		 * let it print.
		 */
			tmjtl++;	/* dq will pick it up */
		else
		if (cbuf[0] == '*' && cbuf[1] == '*')
		{
		/*
		 * broadcast message from cyber.
		 * leave for 'dq' to collect.
		 */
			for (k = 0; k < 48 && k < n; k++)
				cybmsg[k] = cbuf[k];
			cybmsg[k++] = '\n';
			cybmsg[k] = '\0';
		}
		else
		/*
		 * more traps here.
		 */

		{
			/* fix up display */
	printit:	cbuf[n] = '\n';
			for ( k = -1; k < n; )
				cbuf[k =+ 50] = '\n';
			write(2, cbuf, n+1);
		}
	first = 0;
	}
}
