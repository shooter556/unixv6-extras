#

/*
 * update deletions
 */

#include "slup.h"

/*
 * Delete the named updates from the named source modules.
 */
delupdts(fp, fnm)
register struct filentry	*fp;
char	*fnm;
{
	register int	i;
	int	id;
	register struct uwork	*wp;
	long	inleng;
	long	outleng;
	int	errrtn;
/***/	long	_u_l;

	if ((errrtn = getupdt(&ftmp1, fp, 0)) < 0)
		return(errrtn);
/***	inleng = fp->f_size - fp->f_mods*(sizeof *uentry);	*/
/***/	_u_l = fp->f_mods * (sizeof *uentry);
/***/	inleng = fp->f_size - _u_l;
	REWIND(workfid);
	FOUTSET(&ftmp2, workfid);

	if ((errrtn = modtouched()) < 0)
		ERROR();
	if (fp->f_flags&FXNEW && uwork[1].w_flags&UXRUN)
		ERROR(errrtn = ERRUDEL);

	id = 0;
	for (i = 0, wp = uwork; i < works; i++, wp++)
		if (!(wp->w_flags & UXRUN))
			wp->w_modfind = id++;

	outleng = 0;
	wp = uwork;
	while (--inleng >= 0) {
		if ((i = getc(&ftmp1)) == '\0') {
			id = getc(&ftmp1);
			if (uwork[id].w_flags & UXRUN) {
				/* flush command */
				i = getc(&ftmp1);
				switch (i) {

				case UOWN0:
				case UOWN1:
					wp = &uwork[id];
					getc(&ftmp1);
					wp->w_count = getw(&ftmp1);
					inleng =- 5;
					break;

				case UOWN2:
					wp = &uwork[id];
					wp->w_flags =| UXEND;
					inleng =- 2;
					break;

				case UDEL:
				case URESTOR:
					getc(&ftmp1);
					getw(&ftmp1);
					inleng =- 5;
					break;
				}
			} else {
				/* keep command */
				putc('\0', &ftmp2);
				putc(uwork[id].w_modfind, &ftmp2);
				i = getc(&ftmp1);
				putc(i, &ftmp2);
				switch (i) {

				case UOWN0:
				case UOWN1:
					wp = &uwork[id];
					i = getc(&ftmp1);
					if (uwork[i].w_flags & UXRUN)
						ERROR(errrtn = ERRUDEL);
					putc(uwork[i].w_modfind, &ftmp2);
					wp->w_count = getw(&ftmp1);
					putw(wp->w_count, &ftmp2);
					inleng =- 5;
					outleng =+ 5;
					break;

				case UOWN2:
					wp = &uwork[id];
					wp->w_flags =| UXEND;
					inleng =- 2;
					outleng =+ 2;
					break;

				case UDEL:
				case URESTOR:
					if (wp->w_flags & UXRUN)
						ERROR(errrtn = ERRUDEL);
					i = getc(&ftmp1);
					if (uwork[i].w_flags & UXRUN)
						ERROR(errrtn = ERRUDEL);
					putc(uwork[i].w_modfind, &ftmp2);
					i = getw(&ftmp1);
					putw(i, &ftmp2);
					inleng =- 5;
					outleng =+ 5;
					break;
				}
			}
		} else {
			if (wp->w_flags & UXRUN) {
				/* flush text line */
				while (i != '\n') {
					i = getc(&ftmp1);
					inleng--;
				}
			} else {
				/* keep line */
				while (i != '\n') {
					putc(i, &ftmp2);
					outleng++;
					i = getc(&ftmp1);
					inleng--;
				}
				putc(i, &ftmp2);
				outleng++;
			}
			wp->w_lines++;
			wp->w_count--;
		}
		while (!(wp->w_flags & UXEND) && wp->w_count==0) wp--;
	}
	fflush(&ftmp2);
	if ((errrtn = modprune(fp)) < 0)
		ERROR();
	return(putupdt(fp, outleng, workfid));

errlabl:
	free(uwork);
	works = 0;
	mxworks = 0;
	return(errrtn);
}

/*
 * Mark the modset entries that are to be deleted
 */
modtouched()
{
	register int	i;
	register struct uwork	*wp;
	register struct listitem	*lp;

	for (i = 0, wp = uwork; i < works; i++, wp++)
		wp->w_flags =& ~UXRUN;
	for (i = 0, lp = dmodlist.s_list; i < dmodlist.s_cnt; i++, lp++)
		if ((wp = findupdt(lp->s_name, uwork, works)) != NULL)
			wp->w_flags =| UXRUN;
	return(NOERR);
}

/*
 * Subroutine to prune out deleted modsets from
 * update table.
 */
modprune(fp)
struct filentry	*fp;
{
	register int	i;
	register struct uwork	*wpn, *wpo;
	struct modentry	*mp;
	int	errrtn;

	for (i = 0, wpn = wpo = uwork; i < works; i++, wpo++)
		if (wpo->w_flags & UXRUN) {
			mp = findmod(wpo->w_name);
			if ((errrtn = getmods(mp, 1)) < 0 ||
			    (errrtn = delmod(fp->f_name)) < 0 ||
			    (errrtn = putmods(mp)) < 0)
				return(errrtn);
		} else {
			copyname(wpo->w_name, wpn->w_name);
			wpn->w_flags = wpo->w_flags;
			wpn->w_lines = wpo->w_lines;
			wpn->w_cmd = wpo->w_cmd;
			wpn->w_modfind = wpo->w_modfind;
			wpn->w_count = wpo->w_count;
			wpn++;
		}
	works = wpn - uwork;
	return(NOERR);
}
