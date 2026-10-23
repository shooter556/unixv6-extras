#
/*
 *	The passwd file is closed and the flag reset.
 */

#include	<passwd.h>

char	pwfd, pwfl;

pwclose()
{
	close(pwfd);
	pwfl=0;
}
