#

#define PAGE	12
#include	"mas0.h"


extern	lfile;
extern	struct segment segopt;
extern	undef, spc;
extern	numsym;
char	bcd1();
char	bcd2();
extern	char *lbp;
extern	char listbuf[];
extern	struct symbol symtab[];
extern	char line[];
extern	char *linep;
extern	char symbuf[];
extern	char inbuf[];
extern	char tch;

extern	opt_sym;
#define	opt_OFF	0


char sline[l_EOL + 1];		/* mod001 */



llabel()
{
	register char *s, *l, c;

	s = symbuf;
	l = &line[l_LAB];
	while (c = *s++) *l++ = c;
	if (tch == COLON) *l++ = COLON;
	linep = l;
}



lmnem()
{
	register char c;
	register char *s,*l;

	s = symbuf;
	l = &line[l_MNE];
	while (c = *s++) *l++ = c;
	linep = l;
}



lrand()
{
	register char *s, *l, c;

	l = linep;
	s = symbuf;
	while (c = *s++)
		if (l < &line[l_EOL]) *l++ = c;
		else
		{
			linep = l;
			return;
		}
	if ((l < &line[l_EOL]) && (tch > SP)) *l++ = tch;
	linep = l;
}



lcomm()			/* for comments at end-of-lines */
{
	register n;
	register char *l;

	l = &line[l_ADR];
	n = spc >> 8;
	n =& 0377;
	*l++ = bcd1(n);
	*l++ = bcd2(n);
	n = spc & 0377;
	*l++ = bcd1(n);
	*l++ = bcd2(n);
	lcomma();
}




lcomma()
{
	register char c, *l;

	if (undef) line[l_UND] = 'U';
	if (linep < &line[l_COM]) l = &line[l_COM]; else l = linep+1;
	if ((c = tch) != NL) c = getnonbl();
	if (c == NL) { *linep++ = NL; lcopy(linep); return; }
	*l++ = c;
	while ((*l++ = getc(inbuf)) != NL)
		if (l >= &line[l_EOL])
		{
			*l++ = NL;
			while (getc(inbuf) != NL);
			break;
		}
	lcopy(l);
}



lcoml()			/* for comment lines */
{
	register n;
	register char *l, c;

	l = &line[l_LAB];
	*l++ = STAR;
/*					mod013
	if ((c = getnonbl()) == NL) { *l++ = c; lcopy(l); return; }
	l = &line[l_MNE];
	*l++ = c;
*/
	while ((*l++ = getc(inbuf)) != NL)
		if (l >= &line[l_EOL])
		{
			*l++ = NL;
			while (getc(inbuf) != NL);
			break;
		}
	lcopy(l);
}



lcopy(eol)
char *eol;
{
	register char *l, *s;

	if (lfile == 0) return;
	s = lbp;
	for (l = line; l < eol; )
	{
		*s++ = *l++;
		if (s >= listbuf+512)
		{
			write(lfile, listbuf, 512);
			s = listbuf;
		}
	}
	lbp = s;
	for (l = line; l <= &line[l_EOL];) *l++ = SP;
}


lcode1(opc)
{
	register n;
	register char *l;

	l = &line[l_C1];
	*l++ = bcd1(n = opc);
	*l++ = bcd2(n);
}



lcode2(ran)
{
	register n;
	register char *l;

	l = &line[l_C2];
	n = ran & 0377;
	*l++ = bcd1(n);
	*l++ = bcd2(n);
}



lcode3(opc)
{
	register n;
	register char *l;

	l = &line[l_C2];
	n = opc >> 8;
	n =& 0377;
	*l++ = bcd1(n);
	*l++ = bcd2(n);
	n = opc & 0377;
	*l++ = bcd1(n);
	*l++ = bcd2(n);
}



lglobals()
{
	register struct symbol *sym;
	register char *s, *l;
	int n, z, k;
	int qcompar();

	if (opt_sym == opt_OFF)
	{
		if (lfile) write(lfile, listbuf, lbp-listbuf);
		return;
	}
	l = sline;
	if (segopt.segcount == 0) *l++ = PAGE;
	n = numsym-1;
	sym = &symtab[1];
	qsort(sym, n, 14, qcompar);
	z = 0;
	while (n--)
	{
		if (sym->s_seg == 0)
		{
			s = &sym->s_name[0];
			if (segopt.segtype == 2) *l++ = segopt.segchar;		/* mod009 */
			while (*l++ = *s++);
			--l;
			*l++ = TAB;
			*l++ = SP;
			if (sym->s_def == UND) *l++ = 'u';
			else
			{
				k = sym->s_pc >> 8;
				k =& 0377;
				*l++ = bcd1(k);
				*l++ = bcd2(k);
				k = sym->s_pc & 0377;
				*l++ = bcd1(k);
				*l++ = bcd2(k);
			}
			if (z++ > 1)
			{
				*l++ = NL;
				slcopy(l);
				z = 0;
				l = sline;
			}
			else { *l++ = TAB; *l++ = TAB; }
		}
		sym++;
	}
	if (z) { --l; --l; *l++ = NL; }
	slcopy(l);
	if (lfile) write(lfile, listbuf, lbp-listbuf);
}



slcopy(eol)
char *eol;
{
	register char *s, *l;

	if (lfile == 0) return;
	s = lbp;
	for (l = sline; l < eol; )
	{
		*s++ = *l++;
		if (s >= listbuf+512)
		{
			write(lfile, listbuf, 512);
			s = listbuf;
		}
	}
	lbp = s;
}


qcompar(s1, s2)
register char *s1, *s2;
{
	register char c;

	while ((c = *s1++) == *s2++) if ( c == '\0' ) return(0);
	return(c < *--s2 ? -1 : 1);
}




llocals(seg)
int seg;
{
	register struct symbol *sym;
	register char *s, *l;
	int n, z, k;

	if (opt_sym == opt_OFF) return;
	l = sline;
	for (k=0; k<4; k++) *l++ = NL;
	n = numsym - 1;
	sym = &symtab[1];
	z = 0;
	while (n--)
	{
		if (sym->s_seg == seg)
		{
			s = &sym->s_name[0];
			if (segopt.segtype == 1) *l++ = segopt.segchar;		/* mod009 */
			while (*l++ = *s++);
			--l;
			*l++ = TAB;
			*l++ = SP;
			if (sym->s_def == UND) *l++ = 'u';
			else
			{
				k = sym->s_pc >> 8;
				k =& 0377;
				*l++ = bcd1(k);
				*l++ = bcd2(k);
				k = sym->s_pc & 0377;
				*l++ = bcd1(k);
				*l++ = bcd2(k);
			}
			if (z++ > 1)
			{
				*l++ = NL;
				slcopy(l);
				z = 0;
				l = sline;
			}
			else { *l++ = TAB; *l++ = TAB; }
		}
		sym++;
	}
	if (z) { --l; --l; *l++ = NL; }
	*l++ = PAGE;
	slcopy(l);
}



lpage()
{
	register char *l;

	l = line;
	*l++ = PAGE;
	lcopy(l);
}
