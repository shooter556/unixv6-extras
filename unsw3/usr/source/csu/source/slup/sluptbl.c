#

/*
 * Symbol table manipulation
 */

#include "slup.h"

/**********************************************************************/

/*
 * Subfile directory manipulation
 */

/*
 * Search for entry in subfile directory
 */
findfile(name)
char	*name;
{
	register int	i;
	register struct filentry	*fp;

	for (i = 0, fp = filentry; i < nfiles; i++, fp++)
		if (cmpname(fp->f_name, name))
			return(fp);
	return(NULL);
}

/*
 * Generate new entry in subfile directory
 */
newfile(name)
char	*name;
{
	register int	i;
	register struct filentry	*fp, *fpx;

	for (i = 0, fpx = filentry; i < nfiles; i++, fpx++)
		if (cmp0name(fpx->f_name, name) > 0)
			break;
	for (fp = &filentry[nfiles]; i < nfiles; i++, fp--) {
		copyname(fp[-1].f_name, fp->f_name);
		fp->f_date = fp[-1].f_date;
		fp->f_base = fp[-1].f_base;
		fp->f_size = fp[-1].f_size;
		fp->f_mods = fp[-1].f_mods;
		fp->f_groups = fp[-1].f_groups;
		fp->f_flags = fp[-1].f_flags;
	}
	nfiles++;
	copyname(name, fpx->f_name);
	fpx->f_date = now;
	fpx->f_mods = 0;
	fpx->f_groups = 0;
	fpx->f_flags = 0;
	fpx->f_base = 0;
	fpx->f_size = 0;
	return(fpx);
}

/**********************************************************************/

/*
 * Modification directory manipulation
 */

/*
 * Search for entry in modification directory
 */
findmod(name)
char	name[NAMLENG];
{
	register int	i;
	register struct modentry	*mp;

	for (i = 0, mp = modentry; i < nmods; i++, mp++)
		if (cmpname(mp->m_name, name))
			return(mp);
	return(NULL);
}

/**********************************************************************/

/*
 * Update work table manipulation
 */

/*
 * Look through the current update entry list and search
 * for a particular name
 */
findupdt(name, wbase, wcnt)
register char	name[NAMLENG];
struct uwork	*wbase;
int	wcnt;
{
	register int	i;
	register struct uwork	*wp;

	for (i = 0, wp = wbase; i < wcnt; i++, wp++)
		if (cmpname(name, wp->w_name))
			return(wp);
	return(NULL);
}

/**********************************************************************/

/*
 * Name-list manipulation
 */

/*
 * Try and match group name in library header.
 */
findgrp(s)
register char	*s;
{
	register struct listitem	*lp;
	register int	i;

	for (i = 0, lp = lheader.l_groups; i < 16; i++, lp++)
		if (lp->s_name[0] != '\0')
			if (cmpname(lp->s_name, s))
				return(i);
	return(ERRGFND);
}
