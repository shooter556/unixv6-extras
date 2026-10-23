#

/*
 * More of the update routines
 */

#include "slup.h"

/*
 * Check that it is possible to enter
 * modification information to a file.
 * If so, then set up "updtid".
 */
updtok(fp, mp)
register struct filentry	*fp;
struct modentry	*mp;
{
	register int	i;
	register struct uwork	*wp;
	int	errrtn;
/***/	long	_u_l;

	if (fp == NULL)
		return(ERRNFIL);
	if (updtid == 0) {
		if (mp == NULL)
			return(ERRNMOD);
		if ((errrtn = getupdt(&ftmp1, fp, 1)) < 0)
			return(errrtn);
		if (findupdt(mp->m_name, uwork, works) != NULL)
			return(ERRMDUP);
		if (works == mxworks)
			if ((errrtn = growwork(mxworks + 10)) < 0)
				return(errrtn);
		wp = &uwork[updtid = works++];
		copyname(mp->m_name, wp->w_name);
		if ((errrtn = addmod(fp->f_name)) < 0)
			return(errrtn);
		wp->w_flags = 0;
		wp->w_lines = 0;
		wp->w_cmd = 0;
		wp->w_count = 0;
		if (!unewflag) {
			scanid = 0;
			uwork[0].w_flags =| UXEND;
		} else {
			scanid = 1;
			uwork[1].w_flags =| UXEND;
		}
/***		ulngi = fp->f_size - fp->f_mods*(sizeof *uentry);	*/
/***/		_u_l = fp->f_mods * (sizeof *uentry);
/***/		ulngi = fp->f_size - _u_l;

		REWIND(ufid);
		FOUTSET(&ftmp2, ufid);
		ulngo = 0;

		if (!unewflag || updtid!=1) {
			uappndf = 0;
			posnmode = 1;
			FOUTSET(&ufsav, workfid);
		} else {
			posnmode = -1;
			uappndf = 1;
			FOUTSET(&ufsav, ufid);
		}
		/* simulate "*before .1" */
		srchid = 0;
		srchlin = 1;
	}
	return(NOERR);
}

/*
 * given a tag check its validity and save its status
 */
ldeschk(tag, mode)
register struct tag	*tag;
int	mode;
{
	register struct uwork	*wp;

	if ((wp = findupdt(tag->t_name, uwork, works - 1)) == NULL)
		return(ERRMFND);
	if ((tag->t_line < wp->w_lines) || uappndf)
		return(ERRPAST);

	srchid = wp - uwork;
	if (unewflag && srchid==0 && updtid!=1)
		srchid++;
	srchlin = tag->t_line;
	posnmode = mode;
	return(NOERR);
}

/*
 * Seek a particular position on the input file
 * given:
 *	line specifier in:	tag
 *
 * flag:
 *	0	skip to end of line
 *	1	skip to beginning of line
 */
useek(function, testflag)
char	function;	/* character for echoing if echo mode */
int	testflag;	/* test for already deleted */
{
	register struct uwork	*wp;
	struct uwork	*wpx, *wdp;
	register int	c, i;
	char	wkb[150];	/* buffer for listings */
	char	*wkbp;

	if (uappndf)
		return(NOERR);
	if (unewflag && srchid==0 && updtid!=1)
		srchid++;
	wp = &uwork[scanid];
	wpx = &uwork[srchid];
	wdp = setdr(uwork, works);

	if (testflag)
		if ((wp->w_flags & UXYANKD) || wdp->w_cmd==UDEL)
			uerror(0, ERRUDEL);

	if ((c = upeekc) == '\0')
		goto l2;
	else {
		upeekc = '\0';
		goto l1;
	}
	do {
		ulngi--;
		if ((c = getc(&ftmp1)) != '\0') {
			/* text line */
	l1:
			if (posnmode>0 && wp==wpx && (srchlin - 1)==wp->w_lines) {
				upeekc = c;
				return(NOERR);
			}
			for (wkbp = wkb;;) {
				putc(c, &ftmp2);
				ulngo++;
				if (c == '\n') break;
				*wkbp++ = c;
				c = getc(&ftmp1);
				ulngi--;
			}
			*wkbp = '\0';
			wp->w_lines++;
			wp->w_count--;

			if (subopts&OPTUPRT && function!='\0' && !(wp->w_flags & UXYANKD) && wdp->w_cmd!=UDEL) {
				kickline();
				printf("%-14.14s", wp->w_name);
				printf(".%6u", wp->w_lines);
				printf(" %c\t", function);
				println(wkb);
			}
			if (wdp!=uwork && wdp->w_modfind==scanid && wdp->w_count==wp->w_lines) {
				wdp->w_cmd = 0;
				wdp->w_count = 0;
				wdp = setdr(uwork, works);
			}
	l2:
			if (posnmode==0 && wp==wpx && srchlin==wp->w_lines)
				return(NOERR);
		} else {
			/* update control data */
			putc('\0', &ftmp2);
			c = getc(&ftmp1);
			putc(c, &ftmp2);
			i = getc(&ftmp1);
			putc(i, &ftmp2);
			switch (i) {

			case UOWN0:
			case UOWN1:
				wp = &uwork[scanid = c];
				i = getc(&ftmp1);
				putc(i, &ftmp2);
				wp->w_cmd = 0;
				wp->w_count = getw(&ftmp1);
				putw(wp->w_count, &ftmp2);
				ulngi =- 5;
				ulngo =+ 6;
				break;

			case UOWN2:
				wp = &uwork[scanid = c];
				wp->w_cmd = 0;
				wp->w_flags =| UXEND;
				ulngi =- 2;
				ulngo =+ 3;
				break;

			case UDEL:
			case URESTOR:
				wdp = &uwork[c];
				wdp->w_cmd = i;
				i = getc(&ftmp1);
				putc(i, &ftmp2);
				wdp->w_modfind = i;
				i = getw(&ftmp1);
				putw(i, &ftmp2);
				wdp->w_count = i;
				if (wdp->w_flags & UXYANKD) {
					wdp->w_cmd = 0;
					wdp->w_count = 0;
				}
				wdp = setdr(uwork, works);
				ulngi =- 5;
				ulngo =+ 6;
				break;
			}
		}

		/* end of insert section ? */
		while (!(wp->w_flags & UXEND) && (wp->w_cmd!=0 || wp->w_count==0)) {
			wp--;
			scanid--;
		}
	} while (ulngi > 0);
	if (posnmode<0 || (posnmode>0 && (srchlin - 1)==wpx->w_lines)) {
		posnmode = -1;
		uappndf = 1;
		putc('\0', &ftmp2);
		putc(updtid, &ftmp2);
		putc(UOWN2, &ftmp2);
		ulngo =+ 3;
		fflush(&ftmp2);
		FOUTSET(&ufsav, ufid);
		return(NOERR);
	}
	return(ERR2FAR);
}

/*
 * If the file has been modified then copy it
 * back out into the scratch file.
 */
ucopy(fp)
register struct filentry	*fp;
{
	register int	c;
	register int	errrtn;

	if (fp==NULL || updtid==0)
		return(NOERR);
	if (upeekc != '\0') {
		putc(upeekc, &ftmp2);
		upeekc = '\0';
		ulngo++;
	}
	while (ulngi > 0) {
		c = getc(&ftmp1);
		putc(c, &ftmp2);
		ulngi--;
		ulngo++;
	}
	fflush(&ftmp2);
	if ((errrtn = putupdt(fp, ulngo, ufid)) < 0)
		return(errrtn);
	updtid = 0;
	return(NOERR);
}

/*
 * Move any buffered insert lines into main file
 */
uunsave()
{
	register int	errrtn;

	if (savlines != 0) {
		fflush(&ufsav);
		if (!uappndf) {
			putc('\0', &ftmp2);
			putc(updtid, &ftmp2);
			putc((posnmode > 0) ? UOWN1 : UOWN0, &ftmp2);
			putc(scanid, &ftmp2);
			putw(savlines, &ftmp2);
			fflush(&ftmp2);

			REWIND(workfid);
			copyf(workfid, ufid, ulngsav);
			ulngo =+ 6 + ulngsav;

			REWIND(workfid);
		} else
			ulngo =+ ulngsav;
		ulngsav = 0;
		savlines = 0;
	}
	return(NOERR);
}
