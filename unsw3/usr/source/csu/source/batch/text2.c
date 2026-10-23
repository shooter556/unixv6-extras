#include "batch.h"
/*
 *      q*p*u*t
 */
qput(q, qt)
struct quelt	*q;
{
	register struct quelt	*p,*r;

	pp(qlock);    /* obtain exclusive use of the queues */

	if (q == 0)	/* urk !! */
		for (;;)
		{
			prints("\nbatch hung on 'qput' - kill processes directly; don't use 'stop'\n");
			sleep(20);
		}
	if ( (q->qtype = qt) == free)
	{
		njobs--;
		q->origin = 0;
		q->npfile = 0;
		q->ndfile = 0;
		q->tlimit = 0;
		q->nofile = 0;
		q->qlink = qhdr[free].qptr;
		qhdr[free].qptr = q;
	}
	else
	{
		r = &qhdr[qt].qptr;
		while (p = r->qlink) r = p;
		q->qlink = 0;
		r->qlink = q;
	}
	vv(&qhdr[qt].qnum);
	chkpt();  vv(qlock);
}
/*
 *      q*g*e*t
 */
struct quelt	*qget(qt)
{
	register struct quelt	*p;

	if ( pp(&qhdr[qt].qnum) == 0 ) return(0);
	pp(qlock);
	p = qhdr[qt].qptr;
	if (p == 0)
		for (;;)
		{
			prints("\nbatch hung on 'qget' - kill processes directly; don't use 'stop'\n");
			sleep(20);
		}
	qhdr[qt].qptr = p->qlink;	p->qlink = 1;
	if (qt == free) njobs++;
	chkpt();
	vv(qlock);
	return(p);
}
/*
 *	getcyb - get job from atcyber q
 */
getcyb(jname)
register char	*jname;
{
	register struct quelt	*p, *r;
	struct quelt	*sp, *sr;
	int	i;

	for (i = 0; i < 7; i++)
		if (jname[i] & 0100)	/* alpha */
			jname[i] =& ~040;	/* force upper case */

	sp = 0; sr = 0;
	pp(qlock);
	r = &qhdr[atcyber];

	if ( p = r->qlink )	/* some jobs in queue */
	{
		do {
			if ( jname[0] == p->jobn[0] && jname[1] == p->jobn[1])	/* match on first 2 chars */
			{
				if ( !p->cjobn[0] )	/* no suffix, let it do until we find the right one */
				{
					sp = p;
					sr = r;
				}
				else
				if (jname[4] == p->cjobn[0] && jname[5] == p->cjobn[1]
				  && jname[6] == p->cjobn[2])	/* suffix match */
				{
					r->qlink = p->qlink;
			uchn:		p->qlink = 1;	/* unlink from queue */
					qhdr[atcyber].qnum--;
					vv(qlock);
					return(p);
				}
			}

		} while (p = (r=p)->qlink);

		if (sp)		/* no suffix, but first 2 chars match */
		{
			sr->qlink = (p=sp)->qlink;
			p->cjobn[0] = jname[4];		/* concoct a suffix */
			p->cjobn[1] = jname[5];
			p->cjobn[2] = jname[6];
			goto uchn;
		}
	}

	vv(qlock);
	return(0);	/* job not found */
}
