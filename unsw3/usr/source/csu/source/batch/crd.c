#include  "batch.h"
/*
 *	this program controls the card reader (fd=0). it separates local
 *	jobs and cyber jobs queueing them for the local processor concerned
 *	and for jtd.
 *	filedescriptors -
 *		0 -  the card reader
 *		1 -  work
 *		2 -  initiating terminal
 *		3 -  checkpoint file
 *						ian j.   march '76
 *						chris m. march '77
 *						dave h.  march '78
 */
#define	pfd	1	/* file descr assumed for output */
#define	qescp	0276
#define	qeor	0327
#define	qeoi	0326
#define	eof	006
#define	eor	022

char m1[] "\n####  illegal job card\n";
char m2[] "\n####  job cancelled by operator\n";
char m3[] "\n####  unknown batch function\n";
char m4[] "\ncrd: file create error\n";
char m5[] "..., ******* possible missing job card\n";
char m6[] "\n####  Too many cards in deck\n";
char *jcerrs[]
{
	"job name \n",	/* 1 */
	"delimiter\n",	/* 2 */
	"bad param\n",	/* 3 */
	"rubbish ?\n",	/* 4 */
	"max. jobs\n",	/* 5 */
	"disabled \n",	/* 6 */
};

struct { int integ; };

main()
{
	char	buffer[2 + 81*12];
	register char	*card;
	register struct quelt	*q;
	register	n;
	int	qtp;
	int	cardcount;
	int	found;

	if ( fstat(3, buffer) < 0 || nocrd )
		exit(1);
	for ( ;; )
	{
		card = &buffer[2];
		scrdflag = 0;
		while ( !read(0, card, 81) );	/* flush any leading eoi's */
		if ((q = qget(free)) == 0)
			exit(0);
		if ( (q->npfile = crfnm()) < 0 )	/* opens pfd */
		{
			prints(m4);
			exit(1);
		}
		q->rdtime = dtime();
		q->origin = ogcrd;
		found = 0;
		if ( *card == '%' )	/* local */
		{
			if (n = vljc(q, card+1))	/* some error */
			{
				n--;
				prints(m1);
				prints(jcerrs[n]);
				write(pfd, card, 81);
				write(pfd, m1, sizeof m1);
				write(pfd, jcerrs[n], 10);
				qtp = print;
				flushit(q);
			}
			else	/* ok */
			{
				if ( !read(0, card, 81 ) )	/* job card all by itself - ignore */
				{
					nunlink(q->npfile);
					qput(q, free);
					continue;
				}

				for (n = lqstart; n <= lqend; n++)	/* look thru table for process name */
				{
					if (card[1] == procs[n].pdesc[0] &&
					     card[2] == procs[n].pdesc[1] &&
					      card[3] == procs[n].pdesc[2])
					{
						qtp = procs[n].pqtype;	/* this one */
						q->origin =| tyloc;
						found++;
						break;
					}
				}
				chkpt();
				if (!found)
				{
					/* seems we couldn't recognize it */

					write(pfd, card, 81);
					prints(m3);
					qtp = print;
					write(pfd, m3, sizeof m3);
					flushit(q);
				}
				else	 /* ok */
				while ( read(0, card, 81) )
				{
					if ( (card[0] == eor) && !q->ndfile )	/* create data file if haven't already */
					{
						close(pfd);
						if ( (q->ndfile = crfnm()) < 0 )
						{
							prints(m4);
							exit();
						}
						continue;
					} /* of create data */
					write(pfd, card, 81);
				} /* eof local */
			} /* local jobcard ok */
		} /* local jobs */
		else	/* cyber */
		{
			qtp = forcyber;

			if ( n = vkjk(q->jobn, card) )	/* some error on job card */
			{
				n--;
				prints(m1);
				prints(jcerrs[n]);
				if ( card[0] == 'u' && card[1] == 's' &&
				   card[2] == 'e' && card[3] == 'r' &&
				   (card[4] == ',' || card[4] == '(') )
				{
					/*
					 * probably the silly twit
					 * left off his job card, so we
					 * don't want the 'user' card shown
					 * in all its glory ...
					 */
					write(pfd, card, 9);
					write(pfd, m5, sizeof m5);
				}
				else
					write(pfd, card, 81);
				qtp = print;
				write(pfd, m1, sizeof m1);
				write(pfd, jcerrs[n], 10);
				flushit(q);
			}
			else	/* cyber job ok */
			{
				q->origin =| tycyb;
				ncybjobs++;
				card =+ 80;
				cardcount = 0;
				chkpt();
				while (read(0, card, 81))
				{
					if ( ++cardcount > cardlimit && !scrdflag )	/* too many cards in deck */
					{
						scrdflag = 2;	/* pick up later */
						prints(m6);
					}
					if ( !scrdflag )
					{
						switch ( card[0] )
						{

						case eof:
							card[2] = card[3] = '*';

						case eor:
							*card++ = qescp;
							*card = qeor;
							card[79] = ' ';

						default:
							card =+ 80;

							if ( card >= &buffer[962] )	/* full buffer */
							{
								buffer->integ = card - &buffer[2];
								write(pfd, buffer, 1024);
								card = &buffer[2];
							}
						} /* of switch */
					}
				} /* eoi cyber */

				*card++ = qescp; *card++ = qeoi;
				n = 79;
				do *card++ = ' '; while ( --n);	/* construct an 'eoi' card */
				write(pfd, buffer, 2 + (buffer->integ = card - &buffer[2]));
			} /* ok cyber */
		} /* end cyber */

		/* all jobs */

		if ( scrdflag )		/* job cancelled */
		{
			close(pfd);
			nunlink(q->npfile);
			nunlink(q->ndfile);
			qtp = print;
			q->nofile = crfnm();
			if (scrdflag == 1) write(pfd, m2, sizeof m2); else write(pfd, m6, sizeof m6);
			q->origin =| tyerr;
		}
		close(pfd);
		q->jobn[7] = 0;
		qput(q, qtp);
	} /* loop */
}
flushit(q)
register struct quelt	*q;
{
	char	buffer[81];

	q->origin =| tyerr;
	q->nofile = q->npfile;
	q->npfile = 0;
	while (read(0, buffer, 81));
}
#include	"validate.h"
