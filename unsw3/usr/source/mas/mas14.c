#

#include	"mas0.h"


extern	char *atmp;

extern	struct expr exptab[];
extern	struct iform ibuf[], *iptr;
extern	icount;
extern	struct aform abuf[], *abptr;
extern	struct symbol symtab[];
extern	struct fb fbtable[];
extern	struct evalx r;

extern	afile, efile, ifile;
extern	numsym, nextfb;

extern	pc, pc_def;
extern	acount;




pass1a()
{
	register struct aform *ap;
	register n,k;
	int pcsave, bytes, bsav, m;
	struct expr sexp[10];

	savexp(exptab, sexp);
	pcsave = pc;
	pc_def = ABS;
	write(afile, abuf, acount<<2);
	acount = 0;
	abptr = abuf;
	seek(afile, 0, 0);
	seek(efile, 0, 0);
	bytes = 0;
	do
	{
		m = n = read(afile, abuf, 512);
		ap = abuf;
		n =>> 2;	/* no. of entries */
		while (n--)
		{
			pc = ap->a_pc - bytes;
			switch (bsav = ap->a_def)
			{
		    case a_DIR:		newif(i_DIR);
					break;

		    case a_JBRS:
		    case a_JEQS:	rexpr();
					evalexpr();
					if (r.r_type > EXP)
					{				/* subtract 2 to simulate pc */
						k = r.r_val - pc;	/* increment before relative */
						k =- 2;			/* address is calculated */
						if ((k >= -128) && (k <= 127))
						{
							bytes =+ bsav;
							adjust(bsav);
							break;
						}
					}
					newif(i_LONG);
					break;

		    default:		rexpr();
					evalexpr();
					if (r.r_type > EXP) equsym(bsav-100);
			}
			ap++;
		}
	} while (m == 512);

	fadjust();
	pc = pcsave - bytes;
	savexp(sexp, exptab);

	seek(efile,0,0);
	close(afile); unlink(atmp);
	afile = creat(atmp,0600); close(afile); afile = open(atmp,2);
}



equsym(symind)
{
	register struct symbol *sym;

	sym = &symtab[symind];
	sym->s_pc = r.r_val;
	sym->s_def = r.r_type;
}



fadjust()
{
	register struct symbol *sym;
	register struct fb *fbp;
	register i;

	for (i=0, sym = &symtab[1]; i<numsym; i++, sym++)
		if (sym->s_def == EST) sym->s_def = ABS;

	for (i=10, fbp = &fbtable[10]; i<nextfb; i++, fbp++)
		if (fbp->fb_def == EST) fbp->fb_def = ABS;
}



adjust(bs)
{
	register struct symbol *sym;
	register struct fb *fbp;
	register i;

	for (i=1, sym = &symtab[1]; i<numsym; i++, sym++)
		if (sym->s_def == EST)
			if (sym->s_pc > pc) sym->s_pc =- bs;
			else sym->s_def = ABS;

	for (i=10, fbp = &fbtable[10]; i<nextfb; i++, fbp++)
		if (fbp->fb_def == EST)
			if (fbp->fb_pc > pc) fbp->fb_pc =- bs;
			else fbp->fb_def = ABS;
}




newif(n)
{
	iptr->i_pc = pc;
	iptr->i_def = n;
	iptr++;
	if (++icount >= 128)
	{
		write(ifile, ibuf, 512);
		icount = 0;
		iptr = ibuf;
	}
}



rexpr()
{
	register struct expr *x;
	register n;

	x = exptab;
	do
	{
		n = read(efile, x, 4);
		if (n != 4) syserr();
	} while (x++->e_rator);
}



savexp(s1, s2)
struct expr *s1, *s2;
{
	register struct expr *e1, *e2;

	e1 = s1;
	e2 = s2;
	do
	{
		e2->e_rator = e1->e_rator;
		e2->e_val = e1->e_val;
	} while (e2++->e_rand = e1++->e_rand);
}
