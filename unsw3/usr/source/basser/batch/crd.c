#include  "batch.h"

#define	CARD_READER
#define	BCR

/*
 *    this program controls the card reader (fd=0). it separates local
 *    jobs and cyber jobs queueing them for the local processor concerned
 *    and for jtd. local jobs
 *
 *    filedescriptors -
 *                  0 -  the card reader
 *                  1 -  work
 *                  2 -  initiating terminal
 *                  3 -  checkpoint file
 *
 *
 *                                           ian j.   march '76
 *						rewritten
 *					     chris m. march '77
 *
 *    qtypes-
 *                  0 - free.
 *                  1 - jobs for cyber.
 *                  n - local processors.
 */
#define   pfd       1
#define qescp 0276
#define qeor  0327
#define qeoi  0326
                              /********/
                              /********/
char m0[] "\n\n\n####  %: illegal kronos card \n\n";
char m1[] "\n\n\n####  %: illegal job card \n\n";
char m2[] "\n\n\n####  job cancelled by operator\n\n";
char m3[] "\n\n\n####  unknown batch function\n\n";
char m4[] "\n\n\ncrd: file create error\n\n";

struct { int integ; };


main(argc)
{
#ifdef	CARD_READER
	char buffer[2 + 81*12];
	register char *card;
	register struct quelt *q;
	register n;
	int qtp;

	if(fstat(3,ccbuf) < 0) exit();

	for(;;)
	{
		card = &buffer[2];
#ifndef	BCR
		while(!read(0,card,81));
#endif
#ifdef	BCR
		while(!bread( card+80 ));
#endif
		q = qget(free);
		q->ndfile = 0; q->nofile[0] = 0;
		if( (q->npfile = crfnm()) < 0)	/* opens pfd */
		{
			prints(m4);
			exit();
		}
		q->rdtime = dtime();
		q->origin = ogcrd;
		scrdflag = 0;
#ifdef	LOCAL
		if( *card == '%' )	/* local */
		{
			n = 7;
			do q->jobn[n - 1] = card[n]; while( --n);
#ifndef	BCR
			if( !read(0,card,81))
#endif
#ifdef	BCR
			if( !bread( card ))
#endif
			{
				nunlink(q->npfile);
				qput(q,free);
				continue;
			}
			while( *++card == ' ');
			for(n = 0; procs[n].pfile; n++)
			{
				if(procs[n].pdesc &&
				    card[0] == procs[n].pdesc[0] &&
				     card[1] == procs[n].pdesc[1] &&
				      card[2] == procs[n].pdesc[2])
				{
					qtp = procs[n].pqtype;
					q->origin =| tyloc;
					goto func;	/* ugh */
				}
			}
			pflush( card , q );
			qtp = print;
			write(pfd,m3,sizeof m3);
			goto enq;
#ifndef	BCR
		func:	while( read(0,card,81))
#endif
#ifdef	BCR
		func:	while( bread( card ))
#endif
			{
				if( (card[0] == 022) && !q->ndfile)
				{
					close(pfd);
					if( (q->ndfile = crfnm()) < 0)
					{
						prints(m4);
						exit();
					}
					continue;
				}
				write(pfd,card,81);
			}
		}
		else	/* cyber */
#endif
		{
			qtp = forcyber;
#ifndef	QEI
			vkkk( q->cjobn , card );	/* produce a kronos card and job number */
			card =+ 80;
#endif
			if( m1[9] = vkjk(q->jobn,card) )
			{
				m1[9] =| '0';
				prints(m1);
				pflush( card , q );
				qtp = print;
				write(pfd,m1,sizeof m1);
			}
			else
			{
				q->origin =| tycyb;
				ncybjobs--;
				card =+ 80;
#ifndef	BCR
				while(read(0,card,81))
#endif
#ifdef	BCR
				while(bread( card ))
#endif
				 if( !scrdflag)
				  switch(card[0])
				 {
			    case 006:	card[2] = card[3] = '*';
			    case 032:		/* from b1726 */
			    case 022:	*card++ = qescp;
					*card = qeor;
					card[79] = ' ';
			    default:
					card =+ 80;
					if(card >= &buffer[962])
					{

						buffer->integ = card - &buffer[2];
						write(pfd,buffer,1024);
						card = &buffer[2];
					}
				}
				*card++ = qescp; *card++ = qeoi;
				n = 80;
				do *card++ = ' '; while( --n);
				write(pfd,buffer,2 + (buffer->integ = card - &buffer[2]));
			}
		}
			/* all jobs */
		if(scrdflag)		/* job cancelled */
		{
			close(pfd);
			nunlink(q->npfile);
			if(q->ndfile) nunlink(q->ndfile);
			qtp = print;
			q->origin =| tyerr;
			q->nofile[0] = crfnm();
			write(pfd,m2,sizeof m2);
		}
	enq:	close(pfd);
#ifdef	QEI
		q->jobn[7] = 0;
#endif
		qput(q,qtp);
	}
#endif
}


#ifdef	CARD_READER
pflush( c , q )
  register char *c;
  register struct quelt *q;
{
	write(pfd,c,81);
	q->origin =| tyerr;
	q->nofile[0] = q->npfile;
	q->npfile = 0;
#ifndef	BCR
	while( read(0,c,81) );
#endif
#ifdef	BCR
	while ( bread( c ) );
#endif
}


/*
 *	validate kronos job kard
 *
 *	errors:
 *		1) job name too long.
 *		2) bad delimiter following job name.
 *		3) time 
 *		4) cm
 *		5) prty
 *		6) rubbish.
 *		7) delimiter.
 *		8) dayfile.
 *		9) too many cyber jobs, or no cyber jobs allowed.
 */
char *str;
int tab[4]	{ 017, 07777, 0120000, 02000 };
char err[4]	{   5,     3,       4,     8 };



octal(num)
char *num;
{
	register char *n,*c;
	register flg;

	flg = n = 0;
	while( (c = *str - '0') < 8 && c >= 0 )
	{
		n = (n<<3) + c;
#ifdef	BIG_BROTHER
		if( n > num) return(1);
#endif
		str++; flg++;
	}
	return(flg == 0);
}


#ifndef	QEI
/*
 *  produce a valid KRONOS card.
 */

vkkk( cjobn , card )
  char *cjobn;
  register char *card;
{
	register char *kp = "kronos,0NNN.";
	register char *ep = card+80;

  mkjobnum( kp+8 );

  *cjobn++ = kp[8];
  *cjobn++ = kp[9];
  *cjobn++ = kp[10];

  while ( *card++ = *kp++ );
  card--;
  while ( card < ep )  *card++ = ' ';

  /**card = '\n';*/
}
#endif



vkjk(nm,string)
char *string,*nm;
{
	register int n, *nmp;	register char *x;

	if( nocyb || (ncybjobs <= 0) ) return(9);	/* daveh */
	nmp = nm;
	str = string;
	*nmp++ = ' 0';
	*nmp = *nmp++ = '  ';
	nm[6] = ' ';
	n = -1;
	while( (x = *str++,++n < 7) && (((x - 'A') < 26) || (( (x - '0') < 10) && n)) )
	{
		if( x & 0100) x =& ~040;
		*nm++ = x;
	}
	if( n == 0) return(1);
	if( (x != ',') && (x != '(') )
		if(x == '.')
			return(0);
		else	return(2);
	n = 0;
	x = *str;
	if( (x - '0') < 8 && x>=0 )
	{
		for( ; n < 4; n++)
		{
			if( octal(tab[n]))
				return(err[n]);
			switch(*str++)
			{
		    case ')':
		    case '.':
				return(0);
		    case ',':
				continue;
		    default:	return(7);
			}
		}
	}
	else
	{
		do
		{
			switch(*str++)
			{
		    case 'T':
				if( (n&1) || octal( 07777) ) return(3);
				n =| 1;
				break;
		    case 'C':
				if( (n&2) || (*str++ != 'M') || octal( 0120000) ) return(4);
				n =| 2;
				break;
		    case 'P':
				if( (n&4) || octal( 017) ) return(5);
				n =| 4;
				break;
		    case 'D':
				if( (n&8) || octal( 02000) ) return(8);
				n =| 8;
				break;
		    default:
				return(6);
			}
		} while( *str++ == ',');
		if( ( (x = *--str) == '.') || (x == ')'))
			return(0);
	}
	return(7);
}
#endif


#ifdef	BCR

bread( card )
  char *card;
{
	static char buf[512];
	static char *bufp;
	static n;

	register char *cp = card;
	register char *ep = cp+81;
	register char *bp = bufp;

  while ( cp < ep )
	if ( bp && (bp < &buf[n]) )  {
		if ( (*cp++ = *bp++) == '\n' )  {
			cp--;  ep--;
			while ( cp < ep )
				*cp++ = ' ';
			/**cp = '\n';*/
			break;
		}
	}else  {
		if ( (n = read( 0 , buf , 512 )) > 0 )
			bufp = bp = buf;
		else
			return( 0 );
	}

  bufp = bp;
  return( cp );
}
#endif
