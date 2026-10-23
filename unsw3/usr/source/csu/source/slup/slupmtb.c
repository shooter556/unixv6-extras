#

/*
 * Access modification documentation and cross-reference
 */

#include "slup.h"

getmods(mp, rwflag)
register struct modentry	*mp;
int	rwflag;
{
	register int	i, f;

	if (mp->m_refs == 0) {
		docleng = 0;
		if (rwflag)
			REWIND(docfile);
		return(NOERR);
	}
	if ((i = growlist(mp->m_refs, &modfiles)) < 0)
		return(i);
	f = (mp->m_flags & MUUPDAT) ? scrfid : libfile;
	lseek(f, mp->m_base, 0);
	i = mp->m_refs * NAMLENG;
	if (read(f, modfiles.s_list, i) != i)
		return(ERRIO);
	modfiles.s_cnt = mp->m_refs;
	docleng = mp->m_size - i;
	if (rwflag) {
		REWIND(docfile);
		if ((i = copyf(f, docfile, docleng)) < 0)
			return(i);
	}
	return(NOERR);
}

putmods(mp)
register struct modentry	*mp;
{
	register int	i;

	if (mp->m_flags & MUUPDAT)
		if ((i = bfree(mp->m_base, mp->m_size)) < 0)
			return(i);
	if ((mp->m_refs = modfiles.s_cnt) == 0)
		/* it is no more */
		for (i = mp - modentry, nmods--; i < nmods; i++, mp++) {
			copyname(mp[1].m_name, mp->m_name);
			mp->m_date = mp[1].m_date;
			mp->m_refs = mp[1].m_refs;
			mp->m_flags = mp[1].m_flags;
			mp->m_base = mp[1].m_base;
			mp->m_size = mp[1].m_size;
		}
	else {
		mp->m_size = modfiles.s_cnt * NAMLENG + docleng;
		if ((i = balloc(mp->m_size)) < 0)
			return(i);
		mp->m_base = blkbase;
		mp->m_flags =| MUUPDAT;
		i = modfiles.s_cnt * NAMLENG;
		if (write(scrfid, modfiles.s_list, i) != i)
			return(ERRIO);
		REWIND(docfile);
		if ((i = copyf(docfile, scrfid, docleng)) < 0)
			return(i);
	}
	if (modfiles.s_mxcnt != 0) {
		free(modfiles.s_list);
		modfiles.s_cnt = 0;
		modfiles.s_mxcnt = 0;
	}
	return(NOERR);
}

delmod(name)
char	name[NAMLENG];
{
	register int	i;
	register struct listitem	*lp;

	for (i = 0, lp = modfiles.s_list; i < modfiles.s_cnt; i++, lp++)
		if (cmpname(name, lp->s_name)) {
			for (modfiles.s_cnt--; i < modfiles.s_cnt; i++, lp++)
				copyname(lp[1].s_name, lp->s_name);
			break;
		}
	return(NOERR);
}

addmod(name)
char	name[NAMLENG];
{
	register int	errrtn;

	if (modfiles.s_cnt == modfiles.s_mxcnt)
		if ((errrtn = growlist(modfiles.s_mxcnt + 10, &modfiles)) < 0)
			return(errrtn);
	copyname(name, modfiles.s_list[modfiles.s_cnt++].s_name);
	return(NOERR);
}
