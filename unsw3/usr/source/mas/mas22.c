#


#include	"mas0.h"


#define	IMMED	000
#define	DIRECT	020
#define	INDEX	040
#define	EXTEND	060



extern	char symbuf[];
extern	curfb[];
extern	pc;
extern	struct symbol symtab[];
extern	struct fb fbtable[];
extern	char *linep;
extern	char tch;
extern	char line[];


extern	char inbuf[];
extern	undef;
extern	operand;

int	accum;



getsym(ch)
char ch;
{
	register char c, *s;
	register n;

	if ((c = ch) == '\0') c = getc(inbuf);
	s = symbuf;
	n = 0;
	for (;;)
		if (c >= 'a' || DIGIT(c) || c == '$' || c == '_' || c >= 'A' && c <= 'Z')
		{
			if (++n <= LMAX) *s++ = c;		/* mod006 */
			c = getc(inbuf);
		}
		else
		{
			tch = c;
			*s = '\0';
			return(n);
		}
}




getnonbl()
{
	register char c;

	while ((c = getc(inbuf)) == SP || c == TAB);
	return(c);
}



getacc()
{
	register char c;
	register char *l;

	l = &line[l_ACC];
	*l++ = c = getnonbl();
	linep = l;
	tch = getc(inbuf);
	c =| 040;
	return(c - 0140);
}



getitem(ch)
char ch;
{
	register char c;
	register n, rator;
	char c1;
	int total;

	accum = undef = total = rator = n = 0;
	if ((c = ch) == MINUS)
	{
		rator = *linep++ = c;
		c = getc(inbuf);
	}
	for (;;)
	{
		if (getsym(c) == 0)
		{
			c = tch;
			if (linep < &line[l_EOL]) *linep++ = c;
			if (c == STAR)
			{
				tch = getc(inbuf);
				if (linep < &line[l_EOL]) *linep++ = tch;
				n = pc;
			}
			else if (c == QUOTE)
			{
				if (getsym(0) == 0)
				{
					lrand();
					n = tch;
					getsym(0);
					if (n == BACKSL) n = spech(symbuf[0]);
				}
				else n = symbuf[0];
				lrand();
			}
			else return(0);
		}
		else
		{
			c = symbuf[0];
			if (tch == QUOTE)
			{
				lrand();
				n = getnum(c);
				if (tch == QUOTE)
				{
					tch = getc(inbuf);
					if (linep < &line[l_EOL]) *linep++ = tch;
				}
			}
			else if (DIGIT(c))
			{
				c1 = symbuf[1] | 040;
				if (c1 == 'f' || c1 == 'b')
				{
					if (c1 == 'f') c =+ 10;
					lrand();
					n = curfb[c-'0'];
					if (fbtable[n].fb_def == UND)
					{
						if (fbtable[n].fb_pc)
						{
							fbund();
							fbtable[n].fb_pc = 0;
						}
						n = 0;
						undef++;
					}
					n = fbtable[n].fb_pc;
				}
				else n = getdec();
			}
			else if (accsym())
			{
				linep = &line[l_ACC];
				*linep++ = c;
				accum = 1;
				return(-1);
			}
			else
			{
				n = lookup();		/* symbol table index */
				if (symtab[n].s_def == UND)
				{
					if (symtab[n].s_pc)
					{
						symtab[n].s_pc = 0;
						underr();
					}
					n = 0;
					undef++;
				}
				else n = symtab[n].s_pc;
				lrand();
			}
		}
		switch (rator)
		{
	    case 0:	total = n; break;
	
	    case PLUS:	total =+ n; break;
	
	    case MINUS:	total =- n; break;
	
	    case SLASH:	total =/ n; break;
	
	    case STAR:	total =* n;
		}
		switch (c = tch)
		{
	    case PLUS:
	    case MINUS:
	    case STAR:
	    case SLASH:	rator = c;
			c = 0;
			continue;
	
	    default:	return(undef ? 0 : total);
		}
	}
}




getnum(ch)
char ch;
{
	register char *s;
	register char c;
	register number;
	int fac;

	number = fac = 0;
	s = symbuf;
	switch (ch)
	{
    case 'x':
    case 'X':	s =+ getsym(0);
		while (--s >= symbuf)
		{
			c = *s;					/* mod012 */
			if (DIGIT(c)) number =+ (c-'0') << fac;
			else { c =| 040; number =+ (c-'W') << fac; }
			fac =+ 4;
		}
		break;

    case 'b':
    case 'B':	s =+ getbin();
		while (--s >= symbuf) number =+ (*s-'0') << fac++;
		break;

    case 'o':
    case 'O':	s =+ getsym(0);
		while (--s >= symbuf) { number =+ (*s-'0') << fac; fac =+ 3; }
	}

	lrand();
	return(number);
}




getbin()
{
	register char *s, c;
	register n;

	s = symbuf;
	n = 0;
	c = getc(inbuf);
	for (;;)
		if (c == '1' || c == '0')
		{
			n++;
			*s++ = c;
			c = getc(inbuf);
		}
		else
		{
			tch = c;
			*s = '\0';
			return(n);
		}
}



getdec()
{
	register char *s;
	register n, fac;

	s = symbuf;
	while (*s++);
	--s;
	n = 0;
	fac = 1;
	while (--s >= symbuf) { n =+ (*s-'0') * fac; fac =* 10; }
	lrand();
	return(n);
}




getimm(pcinc)
{
	register char c;
	register char *l;
	register n;
	int k;

	if ((c = getnonbl()) != PERCEN) return(getdirec(c));
	l = &line[l_OPR];
	*l++ = c;
	linep = l;
	operand = getitem(getc(inbuf));
	pc =+ pcinc;
	return(IMMED);
}




getdirec(c)
char c;
{
	linep = &line[l_OPR];
	if (getind(c) == EXTEND)
	{
		pc =- 3;
		if (iins(i_DIR)) { pc =+ 2; return(DIRECT); }
		pc =+ 3;
		return(EXTEND);
	}
	return(INDEX);
}




getind(ch)
char ch;
{
	register char c;
	register n;

	linep = &line[l_OPR];
	if ((c = ch) == 0) c = getnonbl();
	if ((operand = getitem(c)) == -1 && accum)
	{
		pc++;
		c =| 040;
		if (c == 'a') return(0);
		return(020);
	}
	if (tch == LPAR)
	{
		pc =+ 2;
		if (linep < &line[l_EOL]) *linep++ = getc(inbuf);
		tch = getc(inbuf);
		return(INDEX);
	}
	pc =+ 3;
	return(EXTEND);
}




getexpr()
{
	linep = &line[l_OPR];
	return(getitem(getnonbl()));
}




spech(ch)
char ch;
{
	switch (ch)
	{
    case BACKSL:	return(BACKSL);

    case 'n':		return(NL);

    case 's':		return(SP);

    case 't':		return(TAB);

    case '0':		return('\0');

    case 'r':		return(CR);		/* mod004 */

    case 'b':		return(BACKSP);		/* mod004 */
	}
	return(BACKSL);
}



accsym()
{
	register char c;

	if (symbuf[1] == '\0')
	{
		c = symbuf[0] | 040;
		if (c == 'a' || c == 'b') return(1);
	}
	return(0);
}
