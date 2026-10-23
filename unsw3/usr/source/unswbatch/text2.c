#include "batch.h"
/*
 *      q*p*u*t
 */
qput(q,qt)
struct quelt *q;
{
	register struct quelt *p,*r;
	pp(qlock);    /* obtain exclusive use of the queues */
	if( (q->qtype=qt) == free)	{
		njobs--;
		q->qlink = qhdr[free].qptr;
		qhdr[free].qptr = q;
	} else {
		r = &qhdr[qt].qptr;
		while(p = r->qlink) r=p;
		q->qlink=0;
		r->qlink=q;
	}
	vv(&qhdr[qt].qnum);
	if(!--chkcnt)
	{
		chkcnt = 30;
		chkpt();
	}
	vv(qlock);
}
/*
 *      q*g*e*t
 */
struct quelt *qget(qt)
{
	register struct quelt *p;
	if( pp(&qhdr[qt].qnum) == 0 ) return(0);
	pp(qlock);
	p=qhdr[qt].qptr;
	qhdr[qt].qptr = p->qlink;	p->qlink = 1;
	if(qt==free) njobs++;
	if(!--chkcnt)
	{
		chkcnt = 30;
		chkpt();
	}
	vv(qlock);
	return(p);
}
