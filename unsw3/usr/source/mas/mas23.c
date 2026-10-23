#

#define	opt_OFF	0
#define	opt_ON	1
int	opt_fcc;

#include	"mas0.h"


extern	struct symbol symtab[];
extern	lab;
extern	noaddr;
extern	struct segment segopt;

extern	char tch, line[], *linep;
extern	char symbuf[];
char	tsymbuf[32];				/* mod011 */
extern	char inbuf[];
char	*fins;					/* mod011 */
int	tbuf[5], lc;				/* mod011 */
extern	npc, spc;
extern	bytec;




pseudop(opc)
{
	register char c;
	register char *l;
	register n;
	int *p, *tp, k, m;
	int save[100];

	switch (opc)
	{
    case o_SEG:	llocals(segopt.segcount++);
		lcomma();
		return;

    case o_EQU:	n = getexpr();
		if (symtab[lab].s_def <= EXP)
		{
			symtab[lab].s_def = ABS;
			symtab[lab].s_pc = n;
		}
		else if (symtab[lab].s_pc != n) syserr();
		lcode3(n);
		lcomma();
		return;

    case o_ORG:	if (iins(i_REL) == 0) syserr();
		npc = spc = getexpr();
		if (bytec > 3) outrec();		/* mod001 */
		newrec();
		lcomm();
		return;

    case o_FCB:	c = getnonbl();
		linep = &line[l_OPR];
		n = getitem(c);
		outbyte(n);
		lcode2(n);
		k = 0;
		p = save;
		while (tch == COMMA)
		{
			n = getitem(getc(inbuf));
			outbyte(n);
			if (k < 100) { *p++ = n; k++; }
		}
		lcomm();
		p = save;
		tch = NL;
		while (k--)
		{
			lcode2(*p++);
			linep = &line[l_C2+2];
			spc++;
			lcomm();
		}
		return;

    case o_FDB:	c = getnonbl();
		linep = &line[l_OPR];
		n = getitem(c);
		outword(n);
		lcode3(n);
		k = 0;
		p = save;
		while (tch == COMMA)
		{
			n = getitem(getc(inbuf));
			outword(n);
			if (k < 100) { k++; *p++ = n; }
		}
		lcomm();
		p = save;
		tch = NL;
		while (k--)
		{
			lcode3(*p++);
			linep = &line[l_C2+4];
			spc++; spc++;
			lcomm();
		}
		return;

    case o_FCC:	l = linep = &line[l_OPR];
		p = save;
		k = 0;
		c = *l++ = getnonbl();
		while ((c = getc(inbuf)) != SLASH)
		{
			if (l < &line[l_EOL]) *l++ = c;
			if ((n = c) == BACKSL)
			{
				if ((n = fcchar(c = getc(inbuf))) == -1)
				{
					outbyte(BACKSL);
					if (k < 100) { k++; *p++ = BACKSL; }
					n = c;
				}
				if (l < &line[l_EOL]) *l++ = c;
			}
			outbyte(n);
			if (k < 100) { k++; *p++ = n; }
		}
		if (l < &line[l_EOL]) *l++ = c;		/* terminating slash */
		linep = l;
		p = save;
		lcode2(*p++);
		--k;
		lcomm();
		tch = NL;
		if (opt_fcc == opt_ON)
			while (k--)
			{
				lcode2(*p++);
				spc++;
				linep = &line[l_C2+2];
				lcomm();
			}
		else spc =+ k;
		return;

    case o_RMB:	n = getexpr();
		if (bytec > 3) outrec();		/* mod001 */
		npc =+ n;
		newrec();
		lcomm();
		return;

    case o_ZMB:	n = getexpr();
		while (n--) outbyte(0);
		lcomm();
		return;

    case o_ICL:	c = getnonbl();				/* mod011 */
		l = tsymbuf;
		while (c != SP && c != TAB && c != NL)
		{
			*l++ = c;
			c = getc(inbuf);
		}
		*l = '\0';
		if (c != NL) while (getc(inbuf) != NL);
		lcopy(line);
		p = inbuf;
		seek(*p, &inbuf[6]-p[1]-p[2], 1);	/* beginning of current block */
		tp = tbuf;
		*tp++ = *p++; *tp++ = *p++; *tp++ = *p++;
		*tp++ = lc; *tp++ = fins;
		fopen(fins = tsymbuf, inbuf);
		lc = 0;
		return;

    case o_OPT:	if ((c = tch) != NL) c = getnonbl();
		if ((tch = c) != NL)
		{
			getsym(c);
			linep = &line[l_OPR];
			lrand();
			options();
		}

    default:	lcomma();
		if (opc == o_PAG) lpage();
		return;
	}
}




fcchar(ch)
register char ch;
{
	switch (ch)
	{
    case BACKSL: return(BACKSL);

    case SLASH:	return(SLASH);

    case 'n':	return(NL);

    case 's':	return(SP);

    case 't':	return(TAB);

    case '0':	return('\0');

    case 'r':	return(CR);		/* mod004 */

    case 'b':	return(BACKSP);		/* mod004 */
	}
	return(-1);
}



struct option
{
	char *optstr;
	int opttype;
};

#define FCCOFF	1
#define FCCON	2
#define SYMOFF	3
#define SYMON	4

struct option opttab[]
{
	"fccoff",	FCCOFF,
	"fccon",	FCCON,
	"symoff",	SYMOFF,
	"symon",	SYMON,
	0,	0,
};

int opt_sym 1;
int opt_fcc 1;

options()
{
	register struct option *o;
	register char *s, c;

	s = symbuf;
	while (c = *s) *s++ = c | 040;
	o = opttab;
	for (s = o->optstr; s != 0; s = (++o)->optstr)
		if (symmat(s))
			switch (o->opttype)
			{
		    case FCCOFF:	opt_fcc = opt_OFF; return;

		    case FCCON:		opt_fcc = opt_ON; return;

		    case SYMOFF:	opt_sym = opt_OFF; return;

		    case SYMON:		opt_sym = opt_ON; return;
			}
}


symmat(s1)
register char *s1;
{
	register char *s, c;

	s = symbuf;
	while ((c = *s++) == *s1++) if (c == '\0') return(1);
	return(0);
}
