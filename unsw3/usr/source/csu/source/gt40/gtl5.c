#

/*
 * validity checking routine
 */

#include "gtl.h"

struct	{
	int	integ;
	};

/*
 * check validity of memory image i have.
 * possible troubles:
 *	1. Start address outside program.
 *	2. If "-d" option, program may overlay odt.
 *		note:	"d" test must be before loader tests
 *		as it adds to memory description.
 *		Also "-7" itself makes arbitrary additions.
 *	3. If "-6" option, program may overlay nasty magics.
 *	4. If "-7" option, program may overlay the 8-bit loader.
 *	5. If "-8", program may overlay the loader's stack.
 */
valid()
{
	register int	cnt;
	register char	*t;
	register struct seg	*mp;
	int	flag;
	extern char	ld8l, ld8siz;

	/* check position of start address */
	if (startadr & 01) {
		fputs("Warning: non-auto start.\n", stderr);
		pauseflag++;
		/*
		 * Note: the builtin 6-bit loader has a
		 * halt instruction at 0166722.
		 * This is to prevent the funny return code
		 * produced by the builtin loader.
		 */
		if (load6)
			startadr = 0166722;
	} else {
		/*
		 * make sure that start address is inside program
		 */
		flag = 0;
		for (mp = &seg[0]; mp<&seg[SEGS] && mp->m_cnt!=0; mp++)
			if (startadr>=mp->m_adr && startadr<(mp->m_adr+mp->m_cnt)) {
				flag++;
				break;
			}
		if (!flag) {
			fputs("Start address not in program.\n", stderr);
			return(-1);
		}
	}

	if (odtflag) {
		t = odthdr.odt_beg + odthdr.odt_siz;
		for (mp = &seg[0]; mp<&seg[SEGS] && mp->m_cnt!=0; mp++)
			if (mp->m_adr<t && (mp->m_adr+mp->m_cnt > odthdr.odt_beg)) {
				fputs("Memory conflict with odt\n", stderr);
				return(-1);
			}
		insert(odthdr.odt_beg, odthdr.odt_siz);

		/* now swap start addresses */

		(&mem[odthdr.odt_pc])->integ = startadr;
		startadr = odthdr.odt_start;
	}

	if (rawflag)
		return(0);	/* skip check of memory coincidences */

	flag = 0;	/* be ready to catch errors */

	if (load6) {
		/*
		 * there is an area in memory that the 6-bit loader
		 * may not be used to load.  this area is
		 * 015700-015777.  here i make sure that the
		 * program doesn't try to use this space.
		 */

		for (mp = &seg[0]; mp<&seg[SEGS] && mp->m_cnt!=0; mp++)
			if (mp->m_adr<=015777 && (mp->m_adr+mp->m_cnt > 015700)) {
				flag++;
				break;
			}
	} else if (load7) {
		/*
		 * an area in memory is required for the bootstrap.
		 * this must be large enough to load the bootstrap
		 * but not occupy the area that the 6-bit loader
		 * doesn't like.
		 */

		t = &ld8l;	/* lowest location for top of loader */
		for (mp = &seg[0]; mp<&seg[SEGS] && mp->m_cnt!=0; mp++) {
			if (t <= mp->m_adr)
				/*
				 * either no more segments to test
				 * or loader fits under this one
				 */
				break;
			t = (&ld8siz + mp->m_adr + mp->m_cnt + 1) & ~1;
			if (t > MEMSIZE) {
				/*
				 * loaders don't tend to work
				 * when loaded outside of memory
				 */
				fputs("No room for bootstrap\n", stderr);
				return(-1);
			}
			if (t < 015777+&ld8l && t > 015700)
				/*
				 * can't put the bootstrap on
				 * top of the unsafe area
				 */
				t = (015777 + &ld8l + 1) & ~1;
			else if (t < &ld8l)
				/*
				 * require some initialisation
				 * space
				 */
				t = &ld8l;
		}
		ld8fix(t - &ld8l);
	} else if (load8) {
		/* are there any illegal areas of memory with
		   8-bit loader */
	}

	if (flag) {
		fputs("Unloadable memory\n", stderr);
		return(-1);
	}
	return(0);
}
