#include	<stdio.h>

fgets(s, n, iop)
char *s;
register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
	while ((c = getc(iop))>=0 && --n>0) {
		*cs++ = c;
		if (c=='\n')
			break;
	}
	if (c<0 && cs==s)
		return(NULL);
	*cs++ = '\0';
	return(s);
}
