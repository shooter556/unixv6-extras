#

/*
 * string processing for filenames
 */

#include "slup.h"

/*
 * Find the suffix of the file name.
 * "NULL" is returned if there is no suffix.
 * If there is a suffix, a pointer to the dot is returned.
 */
getsuf(s)
register char	s[];
{
	register int	c;
	register char	*suf;
	int	gotchar;

	gotchar = 0;
	suf = NULL;
	while ((c = *s++) != '\0') {
		switch (c) {

		case '/':
			gotchar = 0;
			suf = NULL;
			break;

		case '.':
			if (gotchar)
				suf = s;
		default:
			gotchar = 1;
			break;
		}
	}
	return(suf);
}

/*
 * Set the suffix of the file name to the supplied one.
 * If the file name does not already possess a suffix
 * then the supplied one is appended to the end.
 */
setsuf(as, asuf)
char	as[], asuf[];
{
	register char	*s, *s1;

	s1 = workbuf;
	for (s = as; (*s1 = *s++) != '\0'; s1++);
	if ((s = getsuf(workbuf)) != NULL)
		s1 = s;
	else
		*s1++ = '.';
	for (s = asuf; (*s1++ = *s++) != '\0';);
	return(workbuf);
}

/*
 * Get last component of pathname
 */
endpath(s)
register char	*s;
{
	register char	*s0;

	for (s0 = s; *s != '\0';)
		if (*s++ == '/')
			s0 = s;
	return(s0);
}

/*
 * Generate a set of temporary file-names
 */
tmpfiles()
{
	register char	*s1, *s2, *ss;
	static char	abuf[5*17];

	ss = mktemp("/tmp/slupaXXXXXX");
	s1 = abuf;
#define	TEMPGEN(p,c)	(p) = s1; ss[9] = (c); for (s2 = ss; (*s1++ = *s2++)!='\0';)

	TEMPGEN(scrtemp, 's');
	TEMPGEN(worktemp, 'x');
	TEMPGEN(utemp, '1');
	TEMPGEN(lstname, 'p');
	TEMPGEN(docname, 'd');
}
