#include "old.h"

#define	TBAL	060
#define	TLEFT	020
#define	TRIGHT	040

/*
 *	Balanced tree look-up and insert routines
 *	Reference: Knuth, Volume on sorting and searching
 */

struct	symbol	*xs;
struct	symbol	**xt;
struct	symbol	**xw;

/*
 *	symbol look-up routine
 *	returns pointer to symbol or NIL
 *	also sets up necessary pointers to possibly add symbol
 *
 *	For efficiency, the actual routine is coded in assembly
 *		language.


struct symbol *find(sp, np)
struct segment *sp;
char *np;
{
	register struct symbol *p,*q;
	register int t;

	xw = NIL;
	xt = &sp->s_st;

	if((p = *xt) == NIL)
		return(NIL);

	xs = p;
	while(t = scmp(np, p->sname)) {
		if(t == -1)	xw = &p->slson;
		      else	xw = &p->srson;
		if((q = *xw) == NIL)
			return(NIL);
		if(q->sflags & TBAL) {
			xt = xw;
			xs = q;
		}
		p = q;
	}
	return(p);
}
 *	end of look-up routine
 */


/*
 *	string comparison
 *		the actual one is in assembler for speed
 *

scmp(s1, s2)
char *s1,*s2;
{
	register char *sr1,*sr2;
	register int n;

	n = 8; sr1 = s1; sr2 = s2;

	do {
		if(*sr1 < *sr2) 	return(-1);
		if(*sr1 > *sr2)		return( 1);
		if(*sr1 == 0)		return( 0);

		sr1++; sr2++;
	} while(--n);

	return( 0);
}

 *	end of compariosn routine
 */

struct symbol *add(np)
char *np;
{
	register struct symbol *q,*p,*r;

	q = getcore(16);
	copy8(np, q->sname);

	if(xw == NIL)	{ *xt = q; return(q); }

	*xw = q;
	r = p = (scmp(q->sname, xs->sname) == -1 ? xs->slson : xs->srson) ;

	while(p != q) {
		p->sflags =& ~ TBAL;
		if(scmp(q->sname, p->sname) == -1)	{p->sflags =| TLEFT ; p=p->slson; }
			else				{p->sflags =| TRIGHT; p=p->srson; }
	}

	if(scmp(q->sname, xs->sname) == -1) {
		if((xs->sflags & TBAL) == 0)	{xs->sflags =| TLEFT; return(q); }
		if(xs->sflags & TRIGHT)		{xs->sflags =& ~TBAL; return(q); }

		xs->sflags =& ~ TBAL;

		if(r->sflags & TLEFT) {
			p = r;
			xs->slson = r->srson;
			r->srson = xs;
			r->sflags =& ~TBAL;
		} else {
			p = r->srson;
			r->srson = p->slson;
			p->slson = r;
			xs->slson = p->srson;
			p->srson = xs;
			r->sflags =& ~TBAL;
			if(p->sflags & TLEFT ) xs->sflags =| TRIGHT;
			if(p->sflags & TRIGHT) r->sflags =| TLEFT;
			p->sflags =& ~TBAL;
		}
	} else {
		if((xs->sflags & TBAL) == 0)	{xs->sflags =| TRIGHT; return(q); }
		if(xs->sflags & TLEFT )		{xs->sflags =& ~TBAL; return(q); }

		xs->sflags =& ~ TBAL;

		if(r->sflags & TRIGHT) {
			p = r;
			xs->srson = r->slson;
			r->slson = xs;
			r->sflags =& ~TBAL;
		} else {
			p = r->slson;
			r->slson = p->srson;
			p->srson = r;
			xs->srson = p->slson;
			p->slson = xs;
			r->sflags =& ~TBAL;
			if(p->sflags & TRIGHT)	xs->sflags =| TLEFT;
			if(p->sflags & TLEFT )	r->sflags =| TRIGHT;
		}
	}

	*xt = p;
	return(q);
}
