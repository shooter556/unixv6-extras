#
#include	<passwd.h>

strlen(s)
register char	*s;
{
/*
 *	Passed a null terminated string and returns its length including
 *	the null termination.
 */

register	i;

for (i=1 ;*s++ != '\0'; i++);

return(i);
}
