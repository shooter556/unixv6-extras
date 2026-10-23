#

/*
 * More of the update routines:
 * character and string manipulation routines
 */

#include "slup.h"

/*
 * Extract the next substring from the string given a
 * pointer to a string pointer.
 * The string pointer is left updated.
 * Substrings are delimited by blanks, tabs and commas.
 */
getstring(lp)
register char	**lp;
{
	register char	*s, *s1;

	if (*(s = s1 = *lp) == '\0')
		return(NULL);

	while (*s!=' ' && *s!='\t' && *s!=',' && *s!='\0')
		s++;
	if (*s != '\0')
		*s++ = '\0';

	while (*s==' ' || *s=='\t' || *s==',')
		s++;
	*lp = s;	/* update pointer */

	return(s1);
}

/*
 * Analyse an update directive tag.
 * Should be of the format:
 *	[modset.]lineno
 */
gettag(as, tag)
char	*as;	/* tag string */
struct tag	*tag;
{
	register char	*s, *s1;
	register int	i;
	char	*nump;
	int	nameflag;

	nump = as;
	nameflag = 0;
	for (s = as; *s != '\0';)
		if (*s++ == '.') {
			nameflag = 1;
			nump = s;
		}

	for (i = 0, s = nump; *s != '\0';) {
		if (*s<'0' || '9'<*s) return(ERRNDIG);
		i = i * 10 + (*s++ - '0');
	}
	tag->t_line = i;

	if (nameflag) {
		i = 0;
		s = as;
		s1 = tag->t_name;
		clrname(s1);
		if (s < --nump) {
			if (!alpha(*s))
				return(ERRLEAD);
			do {
				if (i == NAMLENG)
					return(ERRNLNG);
				if (!alpha(*s) && (*s<'0' || '9'<*s))
					return(ERRNMCH);
				*s1++ = *s++;
				i++;
			} while (s != nump);
		}
	} else
		/* use default modset name */
		copyname(dfltmod, tag->t_name);
	return(NOERR);
}

/*
 * convert a specified string into the compressed format.
 * copies from (s1) to (s2).
 */
modcompress(s1, s2)
register char	*s1, *s2;
{
	register int	compress;

	for (compress = 0; *s1 != '\0'; s1++) {
		if (*s1<'\0' || *s1>'\176')
			return(ERRCODE);
		if (*s1 < ' ')
			switch (*s1) {

			case '\r':
				continue;

			default:
				break;
			}
		else if (*s1 == ' ') {
			if (++compress == 0177) {
				*s2++ = '\377';
				compress = 0;
			}
			continue;
		}
		if (compress != 0) {
			*s2++ = 0200 | compress;
			compress = 0;
		}
		*s2++ = *s1;
	}
	if (compress != 0)
		*s2++ = 0200 | compress;
	*s2++ = '\0';
	return(NOERR);
}
