#

/*
 * string processing for names
 *	module
 *	modset
 *	group
 */

#include "slup.h"

/*
 * Read a name from a string,
 * check that it is a legal name (characters and size),
 * copy it into a buffer with null fill.
 */
getname(s, abuf)
register char	*s, *abuf;
{
	register int	i;

	if (!alpha(*s))
		return(ERRLEAD);
	clrname(abuf);
	for (i = 0; *s != '\0'; i++) {
		if (i == NAMLENG)
			return(ERRNLNG);
		if (!alpha(*s) && (*s<'0' || '9'<*s))
			return(ERRNMCH);
		*abuf++ = *s++;
	}
	return(NOERR);
}

/*
 * clear out a name buffer
 */
clrname(buf)
register char	*buf;
{
	register int	i;

	i = NAMLENG;
	do {
		*buf++ = '\0';
	} while (--i != 0);
}

/*
 * Copy names:
 *	library module names
 *	group names
 *	modification set names
 */
copyname(s1, s2)
register char	*s1, *s2;
{
	register int	i;

	for (i = 0; i < NAMLENG; i++)
		*s2++ = *s1++;
}

/*
 * Compare two names
 */
cmpname(cp1, cp2)
register char	*cp1, *cp2;
{
	register int	i;

	for (i = 0; i < NAMLENG; i++)
		if (*cp1++ != *cp2++)
			return(0);
	return(1);
}

/*
 * Compare two names
 * but return signed result
 */
cmp0name(cp1, cp2)
register char	*cp1, *cp2;
{
	register int	i;

	for (i = 0; i < NAMLENG; i++) {
		if (*cp1 != *cp2)
			return(*cp1 < *cp2 ? -1 : 1);
		cp1++;
		cp2++;
	}
	return(NOERR);
}

/*
 * Test if argument character is alphabetic; if so return TRUE.
 */
alpha(c)
register char	c;
{
	return(('a'<=c && c<='z') ||
	       ('A'<=c && c<='Z') ||
	       c=='$' || c=='.' || c=='_' || c=='-' || c=='+');
}
