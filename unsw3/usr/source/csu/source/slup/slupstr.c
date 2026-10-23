#

/*
 * general string processing
 */

#include "slup.h"

/*
 * Compare two strings.
 * True (1) is returned if the strings are identical.
 */
streq(s1, s2)
register char	*s1, *s2;
{
	if (s1==NULL || s2==NULL)
		return(0);	/* missing string */

	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return(1);
	return(0);
}

/*
 * copy a string from a to b
 */
copys(s1, s2)
register char	*s1, *s2;
{
	while ((*s2++ = *s1++) != '\0');
}

/*
 * Find any entry in the command table
 */
match(acmds, acmd)
register struct cmds	*acmds;
register char	*acmd;
{
	for (; acmds->cmd >= 0; acmds++)
		if (streq(acmds->name1, acmd) ||
		    streq(acmds->name2, acmd))
			return(acmds->cmd);
	return(ERRDIRC);
}
