#include  "batch.h"
/*
 *	this program enables users to enter jobs into the queues. it separates local
 *	jobs and cyber jobs queueing them for the local processor concerned
 *	and for jtd.
 *
 *	since this is based on "crd", the comments in same apply here also.
 *
 *	filedescriptors -
 *		1 -  work
 *		2 -  initiating terminal
 *		3 -  checkpoint file
 *						ianj.	march '76
 *						daveh.	march '78
 */


#define	pfd	1
#define	qescp	0276
#define	qeor	0327
#define	qeoi	0326
#define	eor	022
#define	eof	006

char m1[] "\n####  illegal job card\n";
char m2[] "\nSubmit: Usage: submit [-] file\n";
char m3[] "\n####  unknown batch function\n";
char m4[] "\nsubmit: file create error\n";
char m5[] "\nsubmit: can't access batch directory\n";
char m6[] "\nsubmit: checkpoint file open error\n";
char m7[] "\n####  too many lines in job\n";
char m8[] "\nsubmit: batch not active\n";
char m9[] "..., ******* possible missing job card\n";
char m10[] "\nsubmit: not allowed\n";
char m11[] "\n####  lines exceed 80 characters\n";

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

int	ibuf[259];	/* for getc */
int	eofsw	0;
int	dropped	0;
int	lineno	0;
int	fout;
int	found;

/*
 *	getcard - read a line & pad to 80 chars
 */

getcard(card)
char	*card;
{
	register char	*c;
	register int	i;

	i = 81;
	c = card;
	lineno++;

	while (--i > 0)
	{
		if ( (( *c = getc(ibuf) ) == '\n' ) ||
			(eofsw = (*c <= 0)) ) goto eoln;

		if ( *c == '\t' )	/* expand tabs */
		{
			do *c++ = ' '; while ( (81 - i--) & 07 );
			i++;
		}
		else
			c++;
	}
	if ( (*c = getc(ibuf)) != '\n' )	/* long line */
	{
		if ( !dropped )
		{
			dropped++;
			prints("\nSubmit: Following line numbers too long; job flushed to printer:\n");
		}
		printf("%d\n", lineno);
		while (getc(ibuf) != '\n');	/* flush line */
		return(81);
	}

eoln:	for ( ; i > 0; i-- ) *c++ = ' ';
	*c = '\n';
	return( eofsw ? 0 : 81 );
}

main(argc, argv)
char	**argv;
{
	char	buffer[2 + 81*12];
	register char	*card;
	register struct quelt	*q;
	register	n;
	int	qtp;
	int	altdest;	/* set on '-' flag */

	fout = 2;

	if ( nosub )
		error(m10);

	if ( actflag != 0 )
		error(m8);

	if (argc == 3 && argv[1][0] == '-')	/* alt dest */
	{
		altdest++;
		argc--;
		argv++;
	}

	if ( argc != 2 )
		error(m2);

	if ( open(fullckp, 2) != 3 )
		error(m6);

	if (fopen(argv[1], ibuf) < 0 )
	{
		prints(argv[1]);
		prints(": file not found\n");
		exit(1);
	}

	if ( chdir(workdir) < 0 )
		error(m5);

	close(pfd);

	card = &buffer[2];

	if ( !getcard(card) )
	{
		prints(argv[1]);
		prints(": empty file\n");
		exit(1);
	}
	if ((q = qget(free)) == 0)
		exit(0);
	if ( (q->npfile = crfnm()) < 0 )
		error(m4);

	q->rdtime = dtime();
	q->origin = ogterm;
	found = 0;
	if( *card == '%' )	/* local */
	{
		if (n = vljc(q, card+1))	/* error */
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
		else
		{
			if ( !getcard(card) )
			{
				nunlink(q->npfile);
				qput(q, free);
				exit(1);
			}
			for (n = lqstart; n <= lqend; n++)
			{
				if (card[1] == procs[n].pdesc[0] &&
				     card[2] == procs[n].pdesc[1] &&
				      card[3] == procs[n].pdesc[2])
				{
					qtp = procs[n].pqtype;
					q->origin =| tyloc;
					found++;
					break;
				}
			}
			chkpt();
			if (!found)
			{
				write(pfd, card, 81);
				prints(m3);
				qtp = print;
				write(pfd, m3, sizeof m3);
				flushit(q);
			}
			else
			while ( getcard(card) )
			{
				if ( (card[0] == eor) && !q->ndfile )
				{
					close(pfd);
					if ( (q->ndfile = crfnm()) < 0 )
					{
						prints(m4);
						exit(1);
					}
					continue;
				}
				write(pfd, card, 81);
			}
		}
	}
	else	/* cyber */
	{
		qtp = forcyber;
		if ( n = vkjk(q->jobn, card) )
		{
			n--;
			prints(m1);
			prints(jcerrs[n]);
			if ( card[0] == 'u' && card[1] == 's' &&
			   card[2] == 'e' && card[3] == 'r' &&
			   (card[4] == ',' || card[4] == '(' ) )
			{
				write(pfd, card, 9);
				write(pfd, m9, sizeof m9);
			}
			else
				write(pfd, card, 81);
			qtp = print;
			write(pfd, m1, sizeof m1);
			write(pfd, jcerrs[n], 10);
			flushit(q);
		}
		else
		{
			q->origin =| tycyb;
			if (altdest)
			{
				q->tlimit = getuid() & 0377;	/* real user */
				q->origin =| altd;
			}
			ncybjobs++;
			card =+ 80;
			chkpt();
			while (getcard(card))
			{
				if (lineno > sbmtlimit)
				{
					prints(m7);
					dropped = 2;
					break;
				}
				switch( card[0] )
				{

				case eof:
					card[2] = card[3] = '*';

				case eor:
					*card++ = qescp;
					*card = qeor;
					card[79] = ' ';

				default:
					card =+ 80;
					if ( card >= &buffer[962] )
					{
						buffer->integ = card - &buffer[2];
						write(pfd, buffer, 1024);
						card = &buffer[2];
					}
				}
			}
			/* eoi */

			*card++ = qescp; *card++ = qeoi;
			n = 79;
			do *card++ = ' '; while ( --n);
			write(pfd, buffer, 2 + (buffer->integ = card - &buffer[2]));
		}
	}
	if (dropped)
	{
		close(pfd);
		nunlink(q->npfile);
		nunlink(q->ndfile);
		qtp = print;
		q->nofile = crfnm();
		if (dropped == 1)
			write(pfd, m11, sizeof m11);
		else
			write(pfd, m7, sizeof m7);
		q->origin =| tyerr;
	}
	close(pfd);
	q->jobn[7] = 0;
	qput(q, qtp);
}

error(s)
char	*s;
{
	prints(s);
	exit(1);
}

flushit(q)
struct quelt	*q;
{
	q->origin =| tyerr;
	q->nofile = q->npfile;
	q->npfile = 0;
}

#include	"validate.h"
