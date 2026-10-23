#

#include	"mas0.h"
extern	syntax();
extern	error();

extern	pc, pc_def;
extern	int curfb[];
extern	char inbuf[];
extern	lc, accvalid;
extern	char symbuf[];
extern	char tch;


extern	struct fb fbtable[];
extern	struct expr exptab[];


extern	struct evalx r;






hex(ch)
register char ch;
{
	ch =| 040;
	return(DIGIT(ch) || ch >= 'a' && ch <= 'f');
}


getsym(ch)
char ch;
{
	register char *s,c;
	register n;

	s = symbuf;
	n = 0;
	if ((c = ch) == '\0') c = getch();
	for (;;)
	{
		if (c >= 'a' || DIGIT(c) || c == '$' || c == '_' || c >= 'A' && c <= 'Z')
		{
			if (++n <= LMAX) *s++ = c;		/* mod006 */
			c = getch();
		}
		else
		if (c == EOF) eoferr();
		else
		{
			tch = c;
			*s = '\0';
			return(n);
		}
	}
}



getch()
{
	register char c;

	if ((c = getc(inbuf)) > 'z' ||  c < SP && c != NL && c != TAB && c != EOF)		/* mod005 */
	{
		error("garbage char");
		return(getch());
	}
	return(c);
}


getitem(ch)
char ch;
{
	register char c;
	register l;
	register struct expr *x;
	char c1;
	int n;

	x = exptab;
	if ((c = ch) == MINUS)
	{
		if ((c = getch()) == MINUS) syntax();
		x->e_rand = e_CON;
		x->e_rator = e_MINUS;
		x->e_val = 0;
		x++;
	}
	for (;;)
	{
		if ((l = getsym(c)) == 0)
		{
			if ((c = tch) == STAR)
			{
				if ((l = getsym(0)) != 0) syntax();
				x->e_rand = e_PC;
			}
			else
			if (c == QUOTE)
			{
				if ((l = getsym(0)) == 0)
				{
					if (tch <= SP) charerr();
					if (tch == BACKSL)
					{
						if ((l = getsym(0)) == 0) c = BACKSL;
						else
						if (l != 1) charerr();
						else
						{
							c = symbuf[0];
							if ((c = spech(c)) == BACKSL) charerr();
						}
					}
					else
					{
						c = tch;
						if ((l = getsym(0)) != 0) charerr();
					}
				}
				else if (l != 1) charerr(); else c = symbuf[0];
				x->e_rand = e_CON;
				x->e_val = c;
			}
			else return(0);
		}
		else
		{
			c = symbuf[0];
			if (tch == QUOTE)	/* hopefully hex, oct, bin */
			{
				if (l != 1) syntax();
				x->e_rand = e_CON;
				x->e_val = getnum(c);
			}
			else
			if (DIGIT(c))
			{
				c1 = symbuf[1] | 040;
				if (l == 2 && (c1 == 'f' || c1 == 'b'))
				{
					if (c1 == 'f') c =+ 10;
					x->e_rand = e_TL;
					n = x->e_val = curfb[c-'0'];
					if (fbtable[n].fb_def == UND && fbtable[n].fb_pc == 0)
						fbtable[n].fb_pc = lc;
				}
				else
				if (l > 5) numerr();
				else
				{
					x->e_val = getdec(&symbuf[l]);
					x->e_rand = e_CON;
				}
			}
			else		/* first char alphabetic */
			if (accsym(l,c))
			{
				if (accvalid && x == exptab && tch <= SP) return(-1);
				syntax();
			}
			else
			{
				x->e_val = lookup();
				x->e_rand = e_SYM;
			}
		}
	
		switch (tch)
		{
	    case SLASH:	x->e_rator = e_SLASH; break;
	
	    case MINUS:	x->e_rator = e_MINUS; break;
	
	    case PLUS:	x->e_rator = e_PLUS; break;
	
	    case STAR:	x->e_rator = e_STAR; break;
	
	    default:	x->e_rator = e_TERM; return(1);
		}
		x++;
		c = 0;
	}
}

getnum(ch)
char ch;
{
	register char *s;
	register char c;
	register number;
	int l, fac;

	number = fac = 0;
	s = symbuf;
	switch (ch)
	{
    case 'x':
    case 'X':	if ((l = getsym(0)) > 4 || l == 0) numerr();
		s =+ l;
		while (--s >= symbuf)
		{
			c = *s;			/* mod012 */
			if (DIGIT(c)) number =+ (c-'0') << fac;
			else
			{
				c =| 040;
				if (c >= 'a' && c <= 'f') number =+ (c-'W') << fac;
				else numerr();
			}
			fac =+ 4;
		}
		break;

    case 'b':
    case 'B':	if ((l = getbin()) > 16 || l == 0) numerr();
		s =+ l;
		while (--s >= symbuf) number =+ (*s-'0') << fac++;
		break;

    case 'o':
    case 'O': 	if ((l = getsym(0)) > 6 || l == 0) numerr();
		s =+ l;
		while (--s >= symbuf)
		{
			if ((c = *s) >= '0' && c <= '7') number =+ (c-'0') << fac;
			else numerr();
			fac =+ 3;
		}
		break;

    default:	syntax();
	}

	if (tch == QUOTE) if ( l = getsym(0) ) syntax();
	return(number);
}




getbin()
{
	register char *s, c;
	register n;

	s = symbuf;
	n = 0;
	c = getch();
	for (;;)
		if (c == '1' || c == '0')
		{
			if (++n < 17) *s++ = c;
			c = getch();
		}
		else if (c == EOF) eoferr();
		else if (c >= 'a' || DIGIT(c) || c == '$' || c == '_' || c >= 'A' && c <= 'Z') numerr();
		else
		{
			tch = c;
			*s = '\0';
			return(n);
		}
}






accsym(len, ch)
char ch;
{
	register char c;

	if (len == 1)
	{
		c = ch | 040;
		return(c == 'a' ? 1 : c == 'b' ? 2 : 0);
	}
	return(0);
}



getacc()
{
	register char c;

	c = getnonbl();
	if (accsym(getsym(c), c) == 0) syntax();
}



getnonbl()
{
	register char c;

	if ((c = tch) != SP && c != TAB) syntax();
	while ((c = getch()) == SP || c == TAB);
	if (c == NL) syntax();
	if (c == EOF) eoferr();
	return(c);
}



getimm(pcinc)
{
	register char c;
	register l;

	if ((c = getnonbl()) == PERCEN)
	{
		if (getitem(0) == 0 || tch > SP) syntax();
		pc =+ pcinc;
	}
	else getdirec(c);
}



getdec(st)
char *st;
{
	register char *s, c;
	register n;
	int fac;

	n = 0;
	s = st;
	fac = 1;
	while (--s >= symbuf)
	{
		c = *s;					/* mod012 */
		if (DIGIT(c)) n =+ (c-'0') * fac;
		else numerr();
		fac =* 10;
	}
	return(n);
}





getdirec(c)
char c;
{
	register n;

	getind(c);
	if (tch != RPAR)
	{
		pc =- 3;	/* reset to usual */
		evalexpr();
		if (r.r_type == ABS)
		{
			if (((n = r.r_val) < 256) && (n >= 0))
			{
				outaform(a_DIR);
				pc =+ 2;
				return;
			}
		}
		pc =+ 3;
	}
}



getind(ch)
char ch;
{
	register char c;
	register n;

	if ((c = ch) == 0) c = getnonbl();
	if ((n = getitem(c)) == 0 && tch != LPAR) syntax();
	if (n == -1)		/* for acc symbol */
	{
		pc++;
		return;
	}
	if (tch == LPAR)
	{
		if ((tch = getch()) != RPAR) parerr();
		pc =+ 2;
	}
	else if (tch <= SP) pc =+ 3; else syntax();
}




getexpr()
{
	register char c;

	c = getnonbl();
	if (getitem(c) == 0 || tch > SP) syntax();
}



getlis()
{
	register char c;
	register i, n;

	c = getnonbl();
	for (i=0; ; i++)
	{
		n = getitem(c);
		if ((c = tch) <= SP)
		{
			if (n == 0) syntax();
			return(++i);
		}
		if (c != COMMA) syntax();
		c = getch();
	}
}



spech(ch)
char ch;
{
	switch (ch)
	{
    case 'n':	return(NL);

    case 's':	return(SP);

    case 't':	return(TAB);

    case '0':	return('\0');

    case 'r':	return(CR);		/* mod004 */

    case 'b':	return(BACKSP);		/* mod004 */
	}

	return(BACKSL);
}



getcomm()
{
	register char c;

	for (c = tch; c != NL; c = getc(inbuf)) if (c == EOF) eoferr();
}



getnl()
{
	getcomm();
	reset();
}
