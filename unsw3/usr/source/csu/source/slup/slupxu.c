#

/*
 * Handle -xu
 */

#include "slup.h"

/*
 * Handle all modsets to be extracted from all files
 */
exupdts(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	int	j;
	register struct filentry	*fp;
	register struct modentry	*mp;
	int	errrtn;

	setexmods();
	if ((errrtn = setexfiles(argc, argv)) < 0)
		return(errrtn);;
	modoutn = (options & OPTOFIL) ? lstname : setsuf(libname, "mod");
	if ((modfout = creat(modoutn, 0600)) < 0)
		return(ERRNEW);

	for (j = 0, mp = modentry; j < nmods; j++, mp++) {
		if (!(mp->m_flags & MXRUN))
			continue;
		if (verbose)
			printf("xu - /%.14s/\n", mp->m_name);
		fflush(&fout);
		FOUTSET(&fout, modfout);
		printf("**\n");
		printf("********************************************************************************\n");
		printf("**\n");
		printf("*ident %.14s\n", mp->m_name);
		if ((errrtn = getmods(mp, 0)) < 0)
			return(errrtn);
		if ((errrtn = listdoc(mp, 0)) < 0)
			return(errrtn);
		for (i = 0; i < mp->m_refs; i++) {
			fp = findfile(modfiles.s_list[i].s_name);
			if (!(fp->f_flags & FXRUN))
				continue;
			printf("**\n");
			printf("********************************************************************************\n");
			printf("**\n");
			printf("*%s %.14s\n", fp->f_flags&FXNEW ? "newdeck" : "deck", fp->f_name);
			if ((errrtn = exup0(fp, mp)) < 0)
				return(errrtn);
		}
		free(modfiles.s_list);
		modfiles.s_cnt = 0;
		modfiles.s_mxcnt = 0;
		fflush(&fout);
		FOUTSET(&fout, 1);
	}
	return(NOERR);
}

/*
 * Extract an individual modset from an individual file.
 */
exup0(fp, mp)
struct filentry	*fp;
struct modentry	*mp;
{
	register int	i;
	register struct uwork	*wp;
	struct uwork	*wpx;
	int	id;
	int	msetbuf;
	int	cntbuf;
	int	errrtn;
	long	inleng;
	extern char	xescape;

	if ((errrtn = getupdt(&ftmp1, fp, 0)) < 0)
		return(errrtn);

	xescape = dfltescape;
	inleng = fp->f_size - fp->f_mods*(sizeof *uentry);

	wp = uwork;
	if (fp->f_flags & FXNEW)
		wp++;

	wpx = findupdt(mp->m_name, uwork, works);
	while (--inleng >= 0) {
		if ((i = getc(&ftmp1)) == '\0') {
			id = getc(&ftmp1);
			i = getc(&ftmp1);
			switch (i) {

			case UOWN0:
			case UOWN1:
				wp = &uwork[id];
				msetbuf = getc(&ftmp1);
				wp->w_count = getw(&ftmp1);
				inleng =- 5;
				if (wp == wpx) {
					printf(i==UOWN0 ? "*after" : "*before");
					printf(" %.14s", uwork[msetbuf].w_name);
					printf(".%u\n", uwork[msetbuf].w_lines + (i != UOWN0));
				}
				break;

			case UOWN2:
				wp = &uwork[id];
				wp->w_flags =| UXEND;
				inleng =- 2;
				if (wp == wpx)
					printf("*append\n");
				break;

			case UDEL:
			case URESTOR:
				msetbuf = getc(&ftmp1);
				cntbuf = getw(&ftmp1);
				inleng =- 5;

				if (&uwork[id] == wpx) {
					printf(i==UDEL ? "*delete" : "*restore");
					printf(" %.14s", wp->w_name);
					printf(".%u", wp->w_lines + 1);
					if (&uwork[msetbuf] != wp ||
					    cntbuf != (wp->w_lines + 1)) {
						printf(",%.14s", uwork[msetbuf].w_name);
						printf(".%u", cntbuf);
					}
					printf("\n");
				}
				break;
			}
		} else {
			for (alinep = aline; i != '\n';) {
				if (i & 0200)
					/* blank uncompression */
					for (i =& 0177; i > 0; i--)
						*alinep++ = ' ';
				else
					*alinep++ = i;
				i = getc(&ftmp1);
				inleng--;
			}
			*alinep = '\0';
			wp->w_lines++;
			if (wp == wpx) {
				if (aline[0]==dfltescape || aline[0]=='\\')
					putchar('\\');
				printf("%s\n", aline);
			}
			wp->w_count--;
		}
		while (!(wp->w_flags & UXEND) && wp->w_count==0)
			wp--;
	}
	if (wpx->w_flags & UXYANKD)
		printf("*yank %.14s\n", wpx->w_name);
	free(uwork);
	mxworks = 0;
	works = 0;
	return(NOERR);
}

/*
 * Discover which modsets are to be extracted
 */
setexmods()
{
	register int	i;
	register struct modentry	*mp;
	register struct listitem	*lp;

	if (xmodlist.s_cnt == 0)
		/* extract ALL modsets */
		for (i = 0, mp = modentry; i < nmods; i++, mp++)
			mp->m_flags =| MXRUN;
	else {
		/* extract only explicit modsets */
		for (i = 0, mp = modentry; i < nmods; i++, mp++)
			mp->m_flags =& ~MXRUN;
		for (i = 0, lp = xmodlist.s_list; i < xmodlist.s_cnt; i++, lp++)
			if ((mp = findmod(lp->s_name)) != NULL)
				mp->m_flags =| MXRUN;
			else
				return(ERRMFND);
	}
	return(NOERR);
}

/*
 * Discover which files are to extracted from
 */
setexfiles(argc, argv)
int	argc;
char	*argv[];
{
	register int	i, errrtn;
	register struct filentry	*fp;
	char	namebuf[NAMLENG];

	if (argc == 0)
		/* extract from ALL files */
		for (i = 0, fp = filentry; i < nfiles; i++, fp++)
			fp->f_flags =| FXRUN;
	else {
		/* extract only from explicit files */
		for (i = 0, fp = filentry; i < nfiles; i++, fp++)
			fp->f_flags =& ~FXRUN;
		for (i = 0; i < argc; i++) {
			if ((errrtn = getname(endpath(argv[i]), namebuf)) < 0) ERROR();
			if ((fp = findfile(namebuf)) == NULL) ERROR(errrtn = ERRFFND);
			fp->f_flags =| FXRUN;
		}
	}
	return(NOERR);

errlabl:
	return(errrtn);
}
