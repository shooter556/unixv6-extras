#

/*
 * Access update directory at beginning of subfile
 */

#include "slup.h"

/*
 * Format of subfile:
 *
 *	f_mods * "uentry" items
 *	text.
 */

/*
 * Read an update directory into memory and
 * convert it to useable format.
 */
getupdt(iop, fp, addflag)
struct ubuf	*iop;
struct filentry	*fp;
int	addflag;
{
	register int	i;
	register struct uentry	*up;
	register struct uwork	*wp;
	char	*p1, *p2;
	int	f;

	f = (fp->f_flags & FXUPDAT) ? scrfid : libfile;
	lseek(f, fp->f_base, 0);

	works = fp->f_mods;
	mxworks = works + 1;
	if (addflag)
		mxworks =+ 10;
	if ((p2 = alloc(mxworks * (sizeof *wp))) == -1)
		return(ERRMEM);
	uwork = p2;
	wp = p2;
	clrname(wp->w_name);
	wp->w_flags = UXEND;
	if (works != 0) {
		i = works * (sizeof *up);
		if ((p1 = alloc(i)) == -1) {
			free(p2);
			return(ERRMEM);
		}
		if (read(f, p1, i) != i) {
			free(p2);
			free(p1);
			return(ERRIO);
		}
		for (i = 0, up = p1, wp = uwork + 1; i < works; i++, up++, wp++) {
			copyname(up->u_name, wp->w_name);
			wp->w_flags = up->u_flags & UXYANKD;
		}
		free(p1);
		if (fp->f_flags & FXNEW)
			uwork[1].w_flags =| UXEND;
	}
	works++;
	for (i = 0, wp = uwork; i < works; i++, wp++) {
		wp->w_count = 0;
		wp->w_cmd = 0;
		wp->w_lines = 0;
	}
	FINSET(iop, f);
	return(NOERR);
}

/*
 * Convert a working area into update directory format
 * and write it out to the file
 */
putupdt(fp, leng, infile)
struct filentry	*fp;
long	leng;
int	infile;
{
	register int	i;
	register struct uwork	*wp;
	register struct uentry	*up;
	char	*p;
	int	l;

	if (fp->f_flags & FXUPDAT)
		if ((i = bfree(fp->f_base, fp->f_size)) < 0)
			return(i);
	if ((l = works) != 0)
		l--;
	fp->f_mods = l;
	l =* (sizeof *up);
	fp->f_size = leng + l;
	if ((i = balloc(fp->f_size)) < 0)
		return(i);
	fp->f_base = blkbase;
	fp->f_flags =| FXUPDAT;
	if (l != 0) {
		if ((p = alloc(l)) == -1) {
			i = ERRMEM;
			goto l1;
		}
		for (i = 1, wp = uwork + 1, up = p; i < works; i++, wp++, up++) {
			copyname(wp->w_name, up->u_name);
			up->u_flags = wp->w_flags;
		}
		if (write(scrfid, p, l) != l)
			ERROR(i = ERRIO);
	}
	REWIND(infile);
	i = copyf(infile, scrfid, leng);
	libmods = 1;
errlabl:
	if (l != 0)
		free(p);
l1:
	if (mxworks != 0) {
		free(uwork);
		works = 0;
		mxworks = 0;
	}
	return(i);
}

/*
 * delete references to a module from modset directory entries
 */
delrefs(fp)
register struct filentry	*fp;
{
	register int	i;
	register struct uentry	*up;
	int	f;
	char	*p;
	struct modentry	*mp;
	int	errrtn;

	if (fp->f_mods == 0)
		return(NOERR);
	i = fp->f_mods * (sizeof *up);
	if ((p = alloc(i)) == -1)
		return(ERRMEM);
	f = (fp->f_flags & FXUPDAT) ? scrfid : libfile;
	lseek(f, fp->f_base, 0);
	if (read(f, p, i) != i) {
		free(p);
		return(ERRIO);
	}
	for (i = 0, up = p; i < fp->f_mods; i++, up++) {
		mp = findmod(up->u_name);
		if ((errrtn = getmods(mp, 1)) < 0)
			return(errrtn);
		if ((errrtn = delmod(fp->f_name)) < 0)
			return(errrtn);
		if ((errrtn = putmods(mp)) < 0)
			return(errrtn);
	}
	free(p);
	return(NOERR);
}
