#

/*
 * printed output routines
 */

#include "slup.h"

struct ubuf	fout { 0 };
int	maxcols;	/* columns on print field */
int	tab0;		/* offset of column 0 */
int	thiscol;
int	prettymode;	/* set if trying for special format */

/*
 * switch to console/printer mode
 */
setmode(mode)
int	mode;
{
	register int	f;

	if (prntmode == mode)
		return(NOERR);

	fflush(&fout);
	if (mode == 0)
		/* console mode */
		f = 1;
	else {
		/* printer mode */
		if (lstfile <= 0) {
			if ((lstfile = creat(lstname, 0600)) < 0)
				return(ERRTEMP);
			title = NULL;
			lineposn = 0;
			fileno = 0;
			pageno = 0;
			(nowtime = ctime(now))[24] = '\0';
		}
		f = lstfile;
		if (mode == 1) {
			maxcols = 96;
			tab0 = 32 - 2;
		} else {
			maxcols = 104;
			tab0 = 24 - 2;
		}
	}
	FOUTSET(&fout, f);
	prntmode = mode;
	return(NOERR);
}

/*
 * set the page title
 */
settitle(s)
char	*s;
{
	title = s;
	lineposn = 0;
	fileno++;
	pageno = 0;
	return(NOERR);
}

/*
 * check for end of page / skip to next page
 */
kickline()
{
	if (--lineposn <= 0) {
		/* new page */
		putc('\f', &fout);
		pageno++;
		printf("S O U R C E   L I B R A R I A N   A N D   U P D A T E   P R O G R A M  ");
		printf(VERSION);
		printf("\t\t%s", nowtime);
		printf("\tPage %d-%d\n", fileno, pageno);

		switch (prntmode) {

		case 1:	/* extraction */
			printf("Extraction: file-name=%s", title);
			break;

		case 2:	/* update */
			printf("Update:");
			if (title != NULL)
				printf(" Modification-set name := %.14s", title);
			break;

		case 3:	/* yank */
			printf("Yank/Unyank:");
			if (title != NULL)
				printf(" File-name = %s", title);
			break;

		case 4:	/* documentation */
			printf("Documentation");
			break;
		}
		printf("\n");
		lineposn = LINESPP - 3;
	}
	printf("\n");
	return(NOERR);
}

/*
 * make sure that we have n lines remaining on page
 */
needln(n)
int	n;
{
	if (n >= lineposn)
		lineposn = 0;
}

/*
 * skip n blank lines
 */
skipln(n, ninc)
register int	n;
int	ninc;
{
	if ((n + ninc) >= lineposn)
		lineposn = 0;
	else
		do {
			lineposn--;
			printf("\n");
		} while (--n != 0);
}

/*
 * print an entire line handling continuation
 */
println(s)
register char	*s;
{
	register int	i;
	register char	c;

	prettymode++;
	for (thiscol = 0; (c = *s++) != '\0';) {
		if (c < 0) {
			for (c =& 0177; --c >= 0;)
				putchar(' ');
			continue;
		}
		if (c<'\040' || c=='\177')
			switch (c) {

			case '\t':
				putchar(c);
				break;

			case '\r':
				printf("\\r");
				break;

			case '\b':
				printf("\\b");
				break;

			case '\014':
				printf("\\f");
				break;

			default:
				printf("\\%o", c);
				break;
			}
		else
			putchar(c);
	}
	prettymode = 0;
}

/*
 * special magic putchar
 */
putchar(c)
register int	c;
{
	register int	i;

	if (prettymode && thiscol>maxcols) {
		prettymode = 0;
		kickline();
		i = tab0;
		do {
			if (i >= 8) {
				i =- 7;
				putc('\t', &fout);
			} else
				putc(' ', &fout);
		} while (--i != 0);
		printf("\\ ");
		thiscol = 0;
		prettymode++;
	}
	putc(c, &fout);
	if (c == '\t')
		thiscol = (thiscol & ~7) + 8;
	else
		thiscol++;
}
