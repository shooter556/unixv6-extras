#

/*
 * Group handling
 */

#include "slup.h"

int	zapgset;		/* changes to group selections */

/*
 * handle the startup procedures for groups
 */
dogroups()
{
	register int	i;
	int	errrtn;

	/*
	 * drop any group names
	 */
	if ((errrtn = losegroups()) < 0)
		return(errrtn);

	/*
	 * add any group names
	 */
	if ((errrtn = makegroups()) < 0)
		return(errrtn);

	/*
	 * if groups required, find them now
	 */
	if (options & OPTGRPS)
		if ((errrtn = makemask(&grpmask, &grplist)) < 0)
			return(errrtn);

	/* setup zapping masks if required */
	switch (func) {

	default:
		break;

	case INSGRP:
		if ((errrtn = makemask(&zapgset, &igrplist)) < 0)
			return(errrtn);
		break;

	case DELGRP:
		if ((errrtn = makemask(&zapgset, &dgrplist)) < 0)
			return(errrtn);
		break;
	}
	return(NOERR);
}

/*
 * make group mask
 */
makemask(newmask, list)
int	*newmask;
register struct namelist	*list;
{
	register int	i;
	register struct listitem	*lp;
	int	j;

	*newmask = 0;
	for (i = 0, lp = list->s_list; i < list->s_cnt; i++, lp++) {
		if ((j = findgrp(lp->s_name)) < 0)
			return(j);
		*newmask =| 1 << j;
	}
	return(NOERR);
}

/*
 * Delete the named entries from the group list
 * and all references in the directory.
 */
losegroups()
{
	register int	i, mask;
	int	j;
	register struct filentry	*fp;

	/*
	 * find the mask of named groups
	 */
	mask = 0;
	for (i = 0; i < rgrplist.s_cnt; i++) {
		if ((j = findgrp(rgrplist.s_list[i].s_name)) < 0)
			return(j);
		clrname(lheader.l_groups[j]);
		mask =| 1 << j;
		libmods = 1;
	}
	/*
	 * remove references from the files
	 */
	for (i = 0, fp = filentry; i < nfiles; i++, fp++)
		fp->f_groups =& ~mask;
	return(NOERR);
}

/*
 * Add the list of names to the group list
 */
makegroups()
{
	register int	i, j;
	register struct listitem	*lp;

	/* look for duplicates */
	lp = mgrplist.s_list;
	for (i = 0; i < mgrplist.s_cnt; i++) {
		if (findgrp(lp->s_name) >= 0)
			return(ERRGDUP);
		/* find empty slot */
		for (j = 0; j < 16; j++)
			if (lheader.l_groups[j][0] == '\0') goto l1;
		return(terror("Too many group names"));
	l1:
		copyname(lp->s_name, lheader.l_groups[j]);
		libmods = 1;
		lp++;
	}
	return(NOERR);
}

/*
 * Include the list in the named groups
 */
insgroup(fp, fnm)
struct filentry	*fp;
char	*fnm;
{
	fp->f_groups =| zapgset;
	libmods = 1;
	return(NOERR);
}

/*
 * drop files from specified groups
 */
delgroup(fp, fnm)
register struct filentry	*fp;
char	*fnm;
{
	fp->f_groups =& ~zapgset;
	libmods = 1;
	return(NOERR);
}

/*
 * read a list of names:
 * [/groupname]...
 */
getlist(cp, thislist)
char	*cp;
register struct namelist	*thislist;
{
	register struct listitem	*lp;
	register int	i;
	char	modset[NAMLENG];
	int	errrtn;

	while (*cp == '/') {
		cp++;
		/* make sure that there is enough space */
		if (thislist->s_mxcnt == thislist->s_cnt)
			if ((errrtn = growlist(thislist->s_cnt + 10, thislist)) < 0)
				return(errrtn);

		errrtn = getitem(&cp, modset);
		for (i = 0, lp = thislist->s_list; i < thislist->s_cnt; i++, lp++)
			if (cmpname(modset, lp->s_name))
				return(ERRGDUP);
		copyname(modset, lp->s_name);
		thislist->s_cnt++;
	}
	return(*cp == '\0' ? NOERR : ERRNLNG);
}

/*
 * Read a modset name from the string pointed to by 'cp'.
 * into 'modset'.  The returned value is the next character
 * after the name.
 * Any characters that are outside the name are stripped off.
 */
getitem(wrkp, namebuf)
char	**wrkp;
register char	*namebuf;
{
	register int	i;
	register char	*cp;

	cp = *wrkp;
	if (!alpha(*cp))
		return(ERRLEAD);
	clrname(namebuf);
	i = NAMLENG;
	do {
		if (--i < 0)
			return(ERRNLNG);
		*namebuf++ = *cp++;
	} while (alpha(*cp) || ('0'<=*cp && *cp<='9'));
	*wrkp = cp;
	return(NOERR);
}
