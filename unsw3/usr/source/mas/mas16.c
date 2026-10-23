#

#include	"mas0.h"


extern	struct expr exptab[];
extern	struct symbol symtab[];
extern	struct fb fbtable[];
extern	struct evalx r;

extern	pc, pc_def;


evalexpr()
{
	register struct expr *x;
	register struct symbol *sym;
	register struct fb *fbp;
	int n,k;

	x = exptab;
	r.r_type = UND;

	switch (x->e_rand)		/* first operand */
	{
    case e_CON:	r.r_val = x->e_val;
		n = ABS;
		break;

    case e_SYM:	sym = &symtab[x->e_val];
		if ((n = sym->s_def) == UND) return;
		r.r_val = sym->s_pc;
		break;

    case e_TL:	fbp = &fbtable[x->e_val];
		if ((n = fbp->fb_def) == UND) return;
		r.r_val = fbp->fb_pc;
		break;

    case e_PC:	r.r_val = pc;
		n = pc_def;
		break;

    default:	syserr();
	}

	if (n == EST)
	{
		if (x->e_rator == 0)
		{
			r.r_type = EST;
			return;
		}
		r.r_type = EXP;
	}

	while (k = x++->e_rator)
	{
		switch (x->e_rand)
		{
	    case e_CON:	comput(k, x->e_val);
			continue;

	    case e_SYM:	sym = &symtab[x->e_val];
			switch (sym->s_def)
			{
		    case UND:	r.r_type = UND; return;

		    case ABS:	comput(k, sym->s_pc); continue;

		    case EST:	r.r_type = EXP; continue;

		    default:	syserr();
			}
			continue;

	    case e_TL:	fbp = &fbtable[x->e_val];
			switch (fbp->fb_def)
			{
		    case UND:	r.r_type = UND; return;

		    case ABS:	comput(k, fbp->fb_pc); continue;

		    case EST:	r.r_type = EXP; continue;

		    default:	syserr();
			}
			continue;

	    case e_PC:	if (pc_def == ABS) comput(k, pc); else r.r_type = EXP;
		}
	}

	if (r.r_type != EXP) r.r_type = ABS;
}



comput(a, b)
{
	if (r.r_type == EXP) return;
	switch (a)
	{
    case e_PLUS:	r.r_val =+ b; return;

    case e_MINUS:	r.r_val =- b; return;

    case e_SLASH:	r.r_val =/ b; return;

    case e_STAR:	r.r_val =* b; return;

    default:		syserr();
	}
}
