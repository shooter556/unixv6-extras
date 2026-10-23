#

/*
 * Storage allocation routines
 */

#include "slup.h"

/**********************************************************************/

/*
 * Allocate space for source directory
 */
growsrc(amount)
int	amount;
{
	register int	i;
	register struct filentry	*fp1, *fp2;
	char	*fpnew;

	if ((fpnew = alloc(amount * (sizeof *filentry))) == -1)
		return(ERRMEM);
	for (i = 0, fp1 = fpnew, fp2 = filentry; i < nfiles; i++, fp1++, fp2++) {
		copyname(fp2->f_name, fp1->f_name);
		fp1->f_date = fp2->f_date;
		fp1->f_base = fp2->f_base;
		fp1->f_size = fp2->f_size;
		fp1->f_mods = fp2->f_mods;
		fp1->f_groups = fp2->f_groups;
		fp1->f_flags = fp2->f_flags;
	}
	if (mxnfiles != 0)
		free(filentry);
	mxnfiles = amount;
	filentry = fpnew;
	return(NOERR);
}

/**********************************************************************/

/*
 * Allocate space for modification directory
 */
growmods(amount)
int	amount;
{
	register int	i;
	register struct modentry	*mp1, *mp2;
	char	*mpnew;

	if ((mpnew = alloc(amount * (sizeof *modentry))) == -1)
		return(ERRMEM);
	for (i = 0, mp1 = mpnew, mp2 = modentry; i < nmods; i++, mp1++, mp2++) {
		copyname(mp2->m_name, mp1->m_name);
		mp1->m_date = mp2->m_date;
		mp1->m_refs = mp2->m_refs;
		mp1->m_flags = mp2->m_flags;
		mp1->m_base = mp2->m_base;
		mp1->m_size = mp2->m_size;
	}
	if (mxnmods != 0)
		free(modentry);
	mxnmods = amount;
	modentry = mpnew;
	return(NOERR);
}

/**********************************************************************/

/*
 * Allocate space for work list (update)
 */
growwork(amount)
int	amount;
{
	register int	i;
	register struct	uwork	*wp1, *wp2;
	char	*wpnew;

	if ((wpnew = alloc(amount * (sizeof *uwork))) == -1)
		return(ERRMEM);
	for (i = 0, wp1 = wpnew, wp2 = uwork; i < works; i++, wp1++, wp2++) {
		copyname(wp2->w_name, wp1->w_name);
		wp1->w_flags = wp2->w_flags;
		wp1->w_lines = wp2->w_lines;
		wp1->w_cmd = wp2->w_cmd;
		wp1->w_modfind = wp2->w_modfind;
		wp1->w_count = wp2->w_count;
	}
	if (mxworks != 0)
		free(uwork);
	mxworks = amount;
	uwork = wpnew;
	return(NOERR);
}

/**********************************************************************/

/*
 * Allocate space for name list
 * These lists are used for:
 *	group names
 *	modset names
 */
growlist(amount, thislist)
int	amount;
struct namelist	*thislist;
{
	register int	i;
	register struct listitem	*lp1, *lp2;
	char	*newlist;

	if ((newlist = alloc(amount * (sizeof *lp1))) == -1)
		return(ERRMEM);
	lp2 = thislist->s_list;
	for (i = 0, lp1 = newlist; i < thislist->s_cnt; i++, lp1++, lp2++)
		copyname(lp2->s_name, lp1->s_name);
	if (thislist->s_mxcnt != 0)
		free(thislist->s_list);
	thislist->s_mxcnt = amount;
	thislist->s_list = newlist;
	return(NOERR);
}
