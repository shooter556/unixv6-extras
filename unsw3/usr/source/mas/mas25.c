#

#include	"mas0.h"


extern	char *mnemtab[];
extern	struct symbol symtab[];
extern	hashtab[];
extern	struct segment segopt;
extern	int numsym;
extern	char symbuf[];




mnemlook()
{
	register char *s;
	register char c;
	register j;
	char *s1;

	s = symbuf;
	while (c = *s) *s++ = c | 040;
	s = symbuf;
	j = (*s++ - 80) * 188;
	j =+ (*s++ - 80) * 15;
	c = *s++ - 80;
	c =~ c;
	j =+ c << 5;
	j =% 511;
	if (j < 0) j = -j;
	if (symmat(mnemtab[j])) return(j);
	syserr();
}




lookup()
{
	register j;
	register struct symbol *sym  ;
	register char *s;
	char loc;

	loc = 0;
	s = symbuf;
	if ((j = segopt.segtype) == 1)
	{
		if (*s == segopt.segchar)
		{
			loc = segopt.segcount;
			s++;
		}
	}
	else if (j == 2)
		if (*s == segopt.segchar) s++; else loc = segopt.segcount;
	j = hashtab[hash(s)];
	do
	{
		sym = &symtab[j];
		if ((sym->s_seg == loc) && compar(&sym->s_name[0],s)) return(j);
	} while ((j = sym->s_chain) != 0);
	syserr();
}
 
 
 
hash(a)
char *a;
{
	register char *s;
	register char c;
	register h;

	h = 0;
	s = a;
	while ((c = *s++) != '\0') h =+ (c - 61) * 29;
	if (h < 0) h = -h;
	return(h % 127);
}
 
 
 
compar(s1, s2)
register char *s1, *s2;
{
register char c;

	while ((c = *s1++) == *s2++) if ( c == '\0' ) return(1);
	return(0);
}
