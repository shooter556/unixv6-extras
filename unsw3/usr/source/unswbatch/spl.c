#include "batch.h"
#define BUFSIZ	1100
/*
 *	this program controls all the print output that comes of the
 *	line.  it identifies all jobs and queues them for printing.
 *
 *	filedescriptors -
 *		      0 - <- cyber
 *		      1	- work
 *		      2 - initiating terminal
 *		      3 - checkpoint file
 *
 *
 *						ian j. march '76
 *
 *
 *	qtypes-
 *		      0 - free
 *		      4 - print
 *
 */

char m1[] "\n\n\nspl: file create error\n";

char cbuf[36];
struct quelt *qq;

init()	/* reinits */
{
	close(1);
	if(qq)
	{
		nunlink(qq->nofile[0]);
		qput(qq,atcyber);
	}
	reset();
}

struct quelt *jnget(ptr, n)
char *ptr;
{
	char jname[7];
	register char *c;
	char *ebuf;
	int register i;
	register struct quelt *p, *r;

	/* if we can't find the message "JOB SYSNAME = AB00XYZ"
	 * in this buffer then give up & call job format?
	 */

	ebuf = &ptr[n - 21];
	for (c = ptr; c < ebuf; c++ )
		if ( c[0] == 'J' && c[4] == 'S' &&
			c[7] == 'N' && c[12] == '=' )
		{
			i = 7;
			do jname[i-1] = c[i+13]; while (--i );
			goto gotit;
		}

		prints("spl: format error??\n");
		i = 7;
		do jname[i-1] = "format?"[i-1]; while(--i);
		goto ferr;
gotit:	pp(qlock);	/* obtain exclusive use of the queues */
	c = &qhdr[atcyber];
	r = 0;
	while( p = c->qlink)
	{
		if( (jname[0] == p->jobn[0] ) &&
			(jname[1] == p->jobn[1]) )
		{
			if( !p->cjobn[0] )
			{
				r = c;
			}
		else	if( ( jname[4] == p->cjobn[0] ) &&
			    ( jname[5] == p->cjobn[1] ) &&
			    ( jname[6] == p->cjobn[2] ) )
			{
				c->qlink = p->qlink;
		uchn:		p->qlink = 1;
				qhdr[atcyber].qnum--;
				vv(qlock); goto found;
			}
		}
		c = p;
	}
	if(r) {
		p = r->qlink;
		r->qlink = p->qlink;
		p->cjobn[0] = jname[4];
		p->cjobn[1] = jname[5];
		p->cjobn[2] = jname[6];
		goto uchn;
	}
	vv(qlock);
ferr:	p = qget(free);
	p->origin = ogcyb|tycyb;
	p->qtype = atcyber;
	ncybjobs--;
	for(i=0;i<7;i++)
		p->jobn[i] = p->cjobn[i-4] = jname[i];
found:	return(p);
}


main(argc,argv)
char **argv; int argc;
{	register n;
	register struct quelt *q;
	register char *b;
	int	j;
	char buff[ BUFSIZ ] ;
	/*
	 *
	 */
	if(fstat(3,cbuf) < 0) exit();
	if( local ) exit();	/* daveh */
	setexit();
	signal(1, init);
	b = buff;
	for(;;)
	{
		qq = 0;
		if( (j = crfnm()) < 0)
			prints(m1), exit();	/* allocate a new file */
		while( (n = read(0, b, BUFSIZ)) == 0); /* banner page */
		qq = q = jnget(b, n);
		q->nofile[0] = j;
		qwrite(1, b, n);
		while((n = read(0, b, BUFSIZ)) >= 0)
		{
			if(n > 0) qwrite(1, b, n);
		}
		close(1);
		q->ndtime = dtime();
		qput(q, print);
	}
}
