#include "batch.h"
/*
 *	purge - delete jobs from specified queue
 *
 *	usage - purge q		purge entire q after confirming
 *		purge q job ... purge "job" from queue
 *		purge q -	interactive purge (like dsw)
 *
 *				"job" may be preceded by "-n"
 *				-> n'th occurrence of job gets purged.
 */

char	*dir	"/spool/batch/.work";
char	*ckp	".checkpoint";
int	cyb;

main( argc, argv ) char **argv;
{
	register struct quelt *q, *lq;
	register int qt;
	int i, count, num;

	if( argc < 2 ) error("Usage: purge q [ - ] [ [ -n ] job ] ...\n");

	switch( argv[1][0] ){		/* q type */

		case 'c':	qt = forcyber; cyb++; break;
		case 'a':	qt = atcyber; cyb++; break;
/*		case 'm':	qt = macro; break;
		case 'f':	qt = ftn; break;
		case 'l':	qt = list; break;
 */
		default:	error("Queue must be [acflm]\n");
	}

	if( chdir(dir) ) error("Can't access directory\n");
	if( open(ckp,2) != 3 ) error("Can't open ckp file\n");

	if( actflag < 0 )	/* batch inactive - read in file */
		if( read(3, chkptp, chkptl) != chkptl ) error("Can't read ckp file\n");

	pp(qlock);	/* lock the queues */

	if( (num = qhdr[qt].qnum) == 0 ){
		vv(qlock);
		error("Queue empty\n");
	}
	if( argc == 2 && confirm() ){	/* purge whole q */
		lq = &qhdr[qt]; q = lq->qptr;
		for( ;; ){
			clear(q);
			if( q->qlink == 0 ) break;	/* last one */
			q = q->qlink;
		}
		/* link these onto free q */
		q->qlink = qhdr[free].qptr;
		qhdr[free].qptr = lq->qptr;
		qhdr[free].qnum =+ num;
		njobs =- num;
		if( cyb ) ncybjobs =+ num;
		qhdr[qt].qnum = qhdr[qt].qptr = 0;
	} else
	if( argc == 3 && argv[2][0] == '-' ){	/* interactive */
		lq = &qhdr[qt]; q = lq->qptr;

		do {
			write(2, &q->jobn, 7);
			switch( getchar() ){

			case 'x':		/* exit */
				q = 0;
			default:		/* repeat */
				while( getchar() != '\n' );
				continue;
			case '\n':		/* next */
				lq = q; q = q->qlink;
				continue;
			case 'y':		/* delete */
				clear(q);
				q = q->qlink;
				delink(lq, qt);
				while( getchar() != '\n' );
			}
		} while( q );
	} else
	/* purge job */
	for( i=2; i<argc; i++ ){
		count = 1;
		if( argv[i][0] == '-' ){
			count = atoi(&argv[i][1]);
			if( ++i >= argc ) break;
		}
		lq = &qhdr[qt]; q = lq->qptr;
		do {
			if( equal(argv[i], &q->jobn) && --count == 0 ){
				clear(q);
				q = q->qlink;
				delink(lq, qt);
				break;
			} else {
				lq = q;
				q = q->qlink;
			}
		} while( q );

		if( count ){
			prints(argv[i]);
			prints(": not found\n");
		}
	}

	chkpt();	/* preserve our efforts */
	vv(qlock);	/* release queues */
	exit(0);
}
error( s ) char *s;
{
	prints(s);
	exit(1);
}
confirm()
{
	register int y;
	prints("Purge of entire queue?  Confirm: ");
	y = 0;
	if( getchar() == 'y' ) y++;
	while( getchar() != '\n' );	/* flush rest of line */
	return(y);
}
equal( a, j ) char *a, *j;
{
	register char *ra, *rj, *rk;
	ra = a; rj = j; rk = rj+6;
	while( *ra++ == ( *rj++ | 040 ) )
		if( !*ra && (*rj == 040 || rj > rk ) ) return(1);
	return(0);
}
clear( q ) struct quelt *q;
{
	register struct quelt *qp;

	qp = q;

	qp->qtype = 0; qp->origin = 0;
	qp->jobn[0] = 0;
	if( qp->npfile ) {nunlink(qp->npfile); qp->npfile = 0;}
	if( qp->ndfile ) {nunlink(qp->ndfile); qp->ndfile = 0;}
	if( qp->nofile[0] ){ nunlink(qp->nofile[0]); qp->nofile[0] = 0;}
}
delink( lq, qt) struct quelt *lq; int qt;
{
	register struct quelt *rq, *rlq;

	rlq = lq; rq = rlq->qlink;
	rlq->qlink = rq->qlink;
	qhdr[qt].qnum--;
	if( cyb ) ncybjobs++;
	njobs--;
	rq->qlink = qhdr[free].qptr;
	qhdr[free].qnum++;
	qhdr[free].qptr = rq;
}
