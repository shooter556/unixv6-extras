#

/*
 * Memory allocation routines
 */

#include "gtl.h"

insert(adr, cnt)
char	*adr, *cnt;
{
	register char	*hiadr;
	register struct seg	*mp;
	char	*tmp;

	hiadr = adr + cnt;
	for (mp = &seg[0]; mp < &seg[SEGS]; mp++) {
		if (mp->m_cnt == 0) {
			/* there is a hole sitting there waiting */
			mp->m_cnt = cnt;
			mp->m_adr = adr;
			return;
		}
		if (hiadr < mp->m_adr) {
			/* swap descriptors */
			tmp = mp->m_adr;
			mp->m_adr = adr;
			adr = tmp;
			tmp = mp->m_cnt;
			mp->m_cnt = cnt;
			cnt = tmp;
			hiadr = adr+cnt;
			continue;
		}
		tmp = mp->m_adr + mp->m_cnt;
		if (tmp >= adr) {
			if (adr < mp->m_adr)
				mp->m_adr = adr;
			if (hiadr<=tmp || mp==&seg[SEGS-1])
				mp->m_cnt = tmp - mp->m_adr;
			else
				/* now slide any overlapping entries down */
				mp->m_cnt = slide(hiadr, mp) - mp->m_adr;
			return;
		}
	}
	fputs("Insertion failure\n", stderr);
	exit(2);
	abort();
}

/*
 * Merge any entries if possible; and slide higher
 * entries down appropriately.
 */
slide(hi, ptr)
register char	*hi;
struct seg	*ptr;
{
	char	*hi1;
	register struct seg	*mp, *mp1;

	for (mp = ++ptr; mp < &seg[SEGS] && mp->m_cnt; mp++) {
		if (mp->m_adr > hi)
			break;

		/* does this segment get merged? */
		hi1 = mp->m_adr + mp->m_cnt;
		if (hi1 > hi)
			hi = hi1;
	}
	for (mp1 = ptr; mp < &seg[SEGS] && mp->m_cnt; mp++, mp1++) {
		/* move useful stuff down memory */
		mp1->m_cnt = mp->m_cnt;
		mp1->m_adr = mp->m_adr;
	}

	/* now clear out dropped fields */
	for (; mp1 < mp; mp1++) {
		mp1->m_cnt = 0;
		mp1->m_adr = 0;
	}
	return(hi);
}
