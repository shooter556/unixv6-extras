#
/*
 *	The password file name is altered to that passed
 */

#include	<passwd.h>

char	pwfd, pwfl;
extern char	*etcpasswd;

pwfile(s)
char	*s;
{
	etcpasswd = s;
}
