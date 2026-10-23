#

/*
 * Deletion routines.
 */

#include "slup.h"

/*
 * Delete modules from a source library
 */
delete(fp, fnm)
register struct filentry	*fp;
char	*fnm;
{
	register struct filentry	*fpx;
	int	errrtn;

	/* must be in original file; don't free scratch space */
	if ((errrtn = delrefs(fp)) < 0)
		return(errrtn);
	/* take up directory space */
	libmods = 1;
	for (fpx = &filentry[--nfiles]; fpx != fp; fp++) {
		copyname(fp[1].f_name, fp->f_name);
		fp->f_date = fp[1].f_date;
		fp->f_base = fp[1].f_base;
		fp->f_size = fp[1].f_size;
		fp->f_mods = fp[1].f_mods;
		fp->f_groups = fp[1].f_groups;
		fp->f_flags = fp[1].f_flags;
	}
	return(NOERR);
}
