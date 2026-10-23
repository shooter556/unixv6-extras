#

/*
 * Main routines
 */

#include "slup.h"

/*
 * List the content of library.
 */
list(argc, argv)
int	argc;
char	*argv[];
{
	register struct filentry	*fp;
	char	*s;
	register int	i, j;
	char	namebuf[NAMLENG];

	if (subopts & OPTLBIG) {
		printf("Directory of file - %s\n", libname);
		printf(" Creation date: %s\n", ansitime(lheader.l_crdate));
		printf(" Last modification: %s\n", ansitime(lheader.l_modate));
		printf(" %d. directory entries\n", nfiles);
		for (i = j = 0; i < 16; i++)
			if (lheader.l_groups[i][0] != '\0')
				j++;
		if (j != 0)
			printf(" %d. groups\n", j);
	}

	if (subopts & OPTLGRP)
		listgrps();
	if (subopts & (OPTLMOD | OPTLXRF | OPTLDOC))
		if ((i = listmods()) < 0)
			return(i);
	if (argc==0 &&
	    subopts & (OPTLMOD | OPTLXRF | OPTLDOC | OPTLGRP) &&
	    !(subopts & OPTLBIG))
		return(NOERR);

	if (subopts & (OPTLMOD | OPTLXRF | OPTLDOC | OPTLGRP))
		printf("\n");

	if (argc == 0) {
		/* list all */
		for (i = 0, fp = filentry; i < nfiles; i++, fp++)
			if (!(options & OPTGRPS) || grpmask&fp->f_groups)
				error(list0(fp));
	} else
		do {
			s = endpath(*argv);
			if ((i = getname(s, namebuf)) >= 0)
				i = ((fp = findfile(namebuf)) == NULL) ? ERRFFND : list0(fp);
			error(i);
			argv++;
		} while (--argc > 0);
	return(NOERR);
}

/*
 * List one entry in source library
 */
list0(fp)
register struct filentry	*fp;
{
	register struct uentry	*up;
	register int	i;
	int	f;
	char	*p;

	if (!(subopts & OPTLBIG))
		printf("%.14s\n", fp->f_name);
	else {
		printf("\n%-14.14s", fp->f_name);
		printf("\tsize: %D.,", fp->f_size);
		printf("  date: %s\n", ansitime(fp->f_date));

		if (fp->f_flags != 0) {
			if (fp->f_flags & FXCOMMN)
				printf("\t(common file)");
			if (fp->f_flags & FXNEW)
				printf("\t(modifications only)");
			printf("\n");
		}

		/* update history */
		if (fp->f_mods != 0) {
			printf("    Modifications:\n");
			i = fp->f_mods * (sizeof *up);
			if ((p = alloc(i)) == -1)
				return(ERRMEM);
			f = (fp->f_flags & FXUPDAT) ? scrfid : libfile;
			lseek(f, fp->f_base, 0);
			if (read(f, p, i) != i) return(ERRIO);
			for (i = 0, up = p; i < fp->f_mods; i++, up++) {
				printf("\t%c", up->u_flags & UXYANKD ? '-' : '+');
				printf(" %-14.14s", up->u_name);
				printf("  %s\n", ansitime(findmod(up->u_name)->m_date));
			}
			free(p);
		}

		/* containing groups? */
		if (fp->f_groups != 0) {
			printf("    Groups:\n");
			for (i = 0; i < 16; i++)
				if (fp->f_groups & (1 << i))
					printf("\t%.14s\n", lheader.l_groups[i]);
		}
	}
	return(NOERR);
}

/*
 * convert the time to fixed format string
 */
ansitime(atime)
long	atime;
{
	register int	*t;
	static char	timbuf[18];
	register char	*cp;

	t = localtime(atime);
	cp = timbuf;
	*cp++ = t[5] / 10 + '0';
	*cp++ = t[5] % 10 + '0';
	*cp++ = '/';
	t[4]++;
	*cp++ = t[4] / 10 + '0';
	*cp++ = t[4] % 10 + '0';
	*cp++ = '/';
	*cp++ = t[3] / 10 + '0';
	*cp++ = t[3] % 10 + '0';
	*cp++ = ' ';
	*cp++ = t[2] / 10 + '0';
	*cp++ = t[2] % 10 + '0';
	*cp++ = ':';
	*cp++ = t[1] / 10 + '0';
	*cp++ = t[1] % 10 + '0';
	*cp++ = ':';
	*cp++ = t[0] / 10 + '0';
	*cp++ = t[0] % 10 + '0';
	*cp++ = '\0';

	return(timbuf);
}

/*
 * list all group names
 */
listgrps()
{
	register int	i, j;
	register struct listitem	*lp;

	if (subopts & OPTLBIG)
		printf("\n");
	j = 0;
	for (i = 0, lp = lheader.l_groups; i < 16; i++, lp++)
		if (lp->s_name[0] != '\0') {
			if (j++ == 0)
				printf("Groups:");
			printf("\t%.14s\n", lp->s_name);
		}
	if (j == 0)
		printf("- no groups -\n");
}

/*
 * List all modset names and optionally list a
 * cross-reference of modified files and any
 * associated documentation.
 */
listmods()
{
	register int	i;
	register struct modentry	*mp;
	int	errrtn;

	if (subopts & (OPTLBIG | OPTLGRP)) printf("\n");
	if (nmods == 0) {
		printf("- No Modifications made -\n");
		return(NOERR);
	}
	if (!(subopts & (OPTLXRF | OPTLDOC)))
		printf("Modifications:\n");
	else {
		printf("Modification ");
		if (subopts & OPTLXRF) {
			printf("Cross-Reference");
			if (subopts & OPTLDOC)
				printf("/");
		}
		if (subopts & OPTLDOC)
			printf("Documentation");
		printf("\n");
	}
	for (i = 0, mp = modentry; i < nmods; i++, mp++) {
		printf("\n%-14.14s", mp->m_name);
		printf("\t%s\n", ansitime(mp->m_date));
		if ((errrtn = getmods(mp, 0)) < 0);
		if (subopts & OPTLXRF)
			listxrf();
		if (subopts & OPTLDOC)
			listdoc(mp, 1);
		free(modfiles.s_list);
		modfiles.s_cnt = 0;
		modfiles.s_mxcnt = 0;
	}
	return(NOERR);
}

/*
 * List names of modified sub-files
 */
listxrf()
{
	register int	i;
	register struct listitem	*lp;

	for (i = 1, lp = modfiles.s_list; i <= modfiles.s_cnt; i++, lp++)
		if ((i % 4)==0 || i==modfiles.s_cnt)
			printf("\t%.14s\n", lp->s_name);
		else
			printf("\t%-14.14s", lp->s_name);
	return(NOERR);
}

/*
 * list documentation of a modset
 */
listdoc(mp, tabflag)
struct modentry	*mp;
int	tabflag;
{
	register int	c;
	register long	l;

	c = mp->m_flags & MUUPDAT ? scrfid : libfile;
	FINSET(&ftmp1, c);
	for (l = docleng; l > 0;) {
		if (tabflag)
			printf("\t");
		printf("*/");
		while ((c = getc(&ftmp1)) != '\n') {
			l--;
			if (c & 0200) {
				for (c =& 0177; c > 0; c--)
					printf(" ");
			} else
				printf("%c", c);
		}
		l--;
		printf("\n");
	}
	return(NOERR);
}
