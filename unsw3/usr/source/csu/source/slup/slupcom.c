#

#include "slup.h"

/*
 * Set a module's common status on or off
 */
common(fp, fnm)
register struct filentry	*fp;
char	*fnm;
{
	if (options & OPTCOMM)
		fp->f_flags =| FXCOMMN;
	else
		fp->f_flags =& ~FXCOMMN;
	libmods = 1;
	return(NOERR);
}
