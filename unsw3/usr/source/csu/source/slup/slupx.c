#

/*
 * More extraction routines
 */

#include "slup.h"

struct filentry	*newfp;	/* to handle calls */
int	xdepth;		/* calling depth */
char	xescape;		/* escape character used in extractions */

/*
 * extract an individual source module producing:
 *	1. source file
 *	2. listing (possibly removing inactive lines)
 * note that each of these may be optionally suppressed
 */
extract(fp, fnm)
struct filentry	*fp;
char	*fnm;
{
	register int	errrtn;

	if (options & OPTXTR)
		if (fcreat(fnm, &ftmp2) < 0)
			return(ERRNEW);
	if (options & OPTPRNT) {
		if ((errrtn = setmode(1)) < 0)
			return(errrtn);
		settitle(fnm);
		kickline();	/* start it up */
		printf("Module: %.14s", fp->f_name);
		kickline();
		printf(" options:");
		if (subopts & OPTPDEL)
			printf(" d");
		if (subopts & OPTPYNK)
			printf(" y");
		if (subopts & OPTPDIR)
			printf(" l");
		kickline();
	}
	xdepth = 0;
	linenum = 0;
	xescape = dfltescape;

	errrtn = xsrce1(fp);

	mxworks = 0;
	works = 0;
	if (options & OPTXTR) {
		fflush(&ftmp2);
		close(ftmp2.io_fid);
		if (errrtn < 0)
			unlink(fnm);
	}
	if (options & OPTPRNT)
		setmode(0);
	return(error(errrtn));
}

/*
 * recursive routine that scans a particular
 * file shooting it out.
 */
xsrce1(fp)
struct filentry	*fp;
{
	char	c, *s;
	int	id, inactiv;
	int	fid;
	register int	i;
	register struct uwork	*wp, *wdp;
	int	thisid;
	struct uwork	*wkbase;
	int	wkcnt, wkmaxc;
	long	inleng;
	long	locn;
	int	lflag;
	int	errrtn;
/***/	long	_u_l;

	if (xdepth == NCALLS)
		return(error(ERRCALL));

	if ((errrtn = getupdt(&ftmp1, fp, 0)) < 0)
		return(errrtn);
	fid = (&ftmp1)->io_fid;
	locn = fp->f_base + fp->f_size;
	wkbase = uwork;
	wkcnt = works;
	wkmaxc = mxworks;


	lflag = options&OPTPRNT && xdepth==0;

	if (lflag && fp->f_flags!=0) {
		kickline();
		if (fp->f_flags & FXCOMMN)
			printf("\t(common file)");
		if (fp->f_flags & FXNEW)
			printf("\t(modifications only)");
		kickline();
	}

	if (lflag && fp->f_mods!=0) {
		kickline();
		printf("\tModifications:");
	}
	for (i = 0, wp = wkbase; i < wkcnt; i++, wp++)
		if (lflag && wp!=wkbase) {
			kickline();
			printf("\t\t%c", wp->w_flags&UXYANKD ? '-' : '+');
			printf(" %-14.14s  %s", wp->w_name, ansitime(findmod(wp->w_name)->m_date));
		}
	if (lflag && fp->f_groups!=0) {
		kickline();
		printf("\tGroups:");
		for (i = 0; i < 16; i++)
			if (fp->f_groups & (1 << i)) {
				kickline();
				printf("\t\t%.14s", lheader.l_groups[i]);
			}
	}
	if (lflag)
		kickline();

	wp = wkbase;
	thisid = 0;
	if (fp->f_flags & FXNEW) {
		wp++;
		thisid++;
	}
	wdp = wp;

/*	for (inleng = fp->f_size - fp->f_mods*(sizeof *uentry); --inleng >= 0;) {	*/
/***/	_u_l = fp->f_mods * (sizeof *uentry);
/***/	for (inleng = fp->f_size - _u_l; --inleng >= 0;) {
		if ((c = getc(&ftmp1)) == '\0') {
			id = getc(&ftmp1);
			inactiv = wkbase[id].w_flags & UXYANKD;
			lflag = subopts&OPTPDIR && xdepth==0;
			if (lflag) {
				kickline();
				printf("\t%-14.14s.******", wkbase[id].w_name);
				printf(" %c\t", inactiv ? '-' : '+');
			}
			c = getc(&ftmp1);
			switch (c) {

			case UOWN0:
			case UOWN1:
				wp = &wkbase[id];
				wp->w_cmd = 0;
				wp->w_modfind = getc(&ftmp1);
				wp->w_count = getw(&ftmp1);
				inleng =- 5;
				if (lflag) {
					printf(c==UOWN0 ? "*after" : "*before");
					printf(" %.14s", wkbase[wp->w_modfind].w_name);
					printf(".%u", wkbase[wp->w_modfind].w_lines + (c != UOWN0));
				}
				if ((wkbase[thisid].w_flags & UXYANKD) ||
				    (setdr(&wkbase[thisid], id - thisid)->w_cmd == UDEL))
					error(ERRUDEL);
				thisid = id;
				break;

			case UOWN2:
				wp = &wkbase[thisid = id];
				wp->w_cmd = 0;
				wp->w_modfind = 0;
				wp->w_flags =| UXEND;
				inleng =- 2;
				if (lflag)
					printf("*append");
				break;

			case UDEL:
			case URESTOR:
				wdp = &wkbase[id];
				wdp->w_cmd = c;
				wdp->w_modfind = getc(&ftmp1);
				wdp->w_count = getw(&ftmp1);

				if (lflag) {
					printf(c==UDEL ? "*delete" : "*restore");
					printf(" %.14s.%u", wp->w_name, wp->w_lines + 1);
					if (wdp->w_modfind != thisid ||
					    wdp->w_count != (wp->w_lines + 1)) {
						printf(",%.14s", wkbase[wdp->w_modfind].w_name);
						printf(".%u", wdp->w_count);
					}
				}

				if ((wp->w_flags & UXYANKD) ||
				    (setdr(wp, wdp - wp)->w_cmd == UDEL))
					error(ERRUDEL);

				if (inactiv) {
					wdp->w_cmd = 0;
					wdp->w_count = 0;
				}
				wdp = setdr(wkbase, wkcnt);
				inleng =- 5;
				break;
			}
		} else {
			for (alinep = aline; c != '\n';) {
				if (c < 0)
					for (c =& 0177; c > 0; c--)
						*alinep++ = ' ';
				else
					*alinep++ = c;
				c = getc(&ftmp1);
				inleng--;
			}
			*alinep = '\0';
			wp->w_lines++;
			wp->w_count--;

			i = (subopts&OPTXNCL || wp->w_flags&UXYANKD || wdp->w_cmd==UDEL) ? UTEXT : xdirective();

			lflag = '\0';
			if (options & OPTPRNT)
				if (xdepth == 0) {
					if (wdp->w_cmd!=UDEL && !(wp->w_flags & UXYANKD)) {
						lflag = thisid==0 ? ' ' : '+';
					} else if ((subopts&OPTPDEL && wdp->w_cmd==UDEL) ||
						   (subopts&OPTPYNK && wp->w_flags&UXYANKD))
							lflag = '-';
				} else {
					if (subopts&OPTPCOM &&
					    !(wp->w_flags & UXYANKD) &&
					    wdp->w_cmd!=UDEL)
						lflag = 'c';
				}

			inactiv = wp->w_flags&UXYANKD || wdp->w_cmd==UDEL;

			if (i==UTEXT && !inactiv)
				linenum++;

			if (lflag != '\0') {
				kickline();
				if (i!=UTEXT || lflag=='-')
					printf("\t");
				else
					printf("%6u\t", linenum);
				if (lflag != 'c')
					printf("%-14.14s.%6u", wp->w_name, wp->w_lines);
				else
					printf("\t\t\t     ");
				printf(" %c\t", lflag);
				println(aline);
			}
			switch (i) {

			case UTEXT:
				if (!inactiv && options&OPTXTR) {
					for (alinep = aline; *alinep != '\0'; alinep++)
						putc(*alinep, &ftmp2);
					putc('\n', &ftmp2);
				}
				break;

			case CALL:
				/* do call */
				xdepth++;
				errrtn = xsrce1(newfp);
				xdepth--;
				FINSET(&ftmp1, fid);
				lseek(fid, locn - inleng, 0);
				break;

			case CPREFIX:
				break;

			default:
				error(i);
				break;
			}

			/* test for end of delete/restore range */
			if (wdp!=wkbase &&
			    wdp->w_modfind==thisid &&
			    wdp->w_count==wp->w_lines) {

				/*
				 * Is end of delete block itself deleted?
				 */
				if (setdr(wp, wdp - wp)->w_cmd == UDEL)
					error(ERRUDEL);

				wdp->w_cmd = 0;
				wdp->w_count = 0;
				wdp = setdr(wkbase, wkcnt);
			}
		}

		/* end of insert ?? */
		while (!(wp->w_flags & UXEND) && (wp->w_cmd!=0 || wp->w_count==0)) {
			wp--;
			thisid--;
		}
	}
errlabl:
	free(wkbase);
	return(NOERR);
}

/*
 * Find the oldest still active delete/restore descriptor.
 */
setdr(wkbase, wkcnt)
register struct uwork	*wkbase;
int	wkcnt;
{
	register struct uwork	*wdp;

	for (wdp = &wkbase[wkcnt - 1]; wdp != wkbase; wdp--)
		if (wdp->w_cmd != 0) {
			if (wkbase[wdp->w_modfind].w_lines < wdp->w_count)
				break;
			wdp->w_cmd = 0;
			wdp->w_count = 0;
		}
	return(wdp);
}

/*
 * compile time directives
 */
struct cmds	xcmds[] {
	CPREFIX,"pc",	"prefixc",
	CALL,	"c",	"call",
	-1
	};

/*
 * look for compile time directives
 */
xdirective()
{
	register int	i;
	register char	*s;
	int	errrtn;
	char	wkb[150];
	char	*wkp;
	char	namebuf[NAMLENG];

	if (aline[0] != xescape)
		return(UTEXT);
	copys(aline, wkb);
	wkp = &wkb[1];
	s = getstring(&wkp);
	if (s == NULL)
		return(ERRDIRC);
	if ((i = match(xcmds, s)) < 0)
		return(i);
	s = getstring(&wkp);
	if (getstring(&wkp) != NULL)
		return(ERR2MNY);

	switch (i) {

	case CPREFIX:	/* [character] */
		if (s == NULL)
			xescape = dfltescape;
		else if (s[1] == '\0')
			xescape = s[0];
		else
			return(ERRPREF);
		break;

	case CALL:	/* filename */
		if (s == NULL)
			return(ERR2FEW);
		if ((errrtn = getname(endpath(s), namebuf)) < 0)
			return(errrtn);
		if ((newfp = findfile(namebuf)) == NULL)
			return(ERRFFND);
		break;
	}
	return(i);
}
