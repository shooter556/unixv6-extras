#

/*
 * Insertion/replacement routines
 */

#include "slup.h"

/*
 * Handle all source files
 */
insert(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	register struct filentry	*fp;
	char	namebuf[NAMLENG];
	int	errrtn, errflag;

	descrip = options & OPTREPL ? "r" : "i";
	errflag = 0;
	for (i = 0; i < argc; i++) {
		setprompt(argv[i]);
		if (verbose)
			doprompt();
		if ((errrtn = getname(endpath(argv[i]), namebuf)) < 0)
			ERROR();
		if ((fp = findfile(namebuf)) == NULL)
			fp = newfile(namebuf);
		else {
			if (!(options & OPTREPL))
				ERROR(errrtn = ERRDONE);
			if ((errrtn = delrefs(fp)) < 0)
				ERROR();
		}
		if ((errrtn = ins0(fp, argv[i])) < 0) {
errlabl:
			error(errrtn);
			errflag++;
		}
	}
	return(errflag ? ERR : NOERR);
}

/*
 * Copy and compact an individual source file
 */
ins0(fp, fnm)
register struct filentry	*fp;
char	*fnm;
{
	register int	c, compress;
	unsigned	lnsize, linenum;
	long	outleng;
	int	errrtn;

	errrtn = 0;
	if (fopen(fnm, &ftmp1) < 0)
		return(ERROLD);
	REWIND(workfid);
	FOUTSET(&ftmp2, workfid);

	outleng = 0;
	lnsize = 0;
	compress = 0;
	linenum = 1;
	while ((c = getc(&ftmp1)) >= 0) {
		if (c > '\176')
			ERROR(errrtn = ERRCODE);
		lnsize++;
		if (c < ' ')
			switch (c) {

			case '\0':
			case '\r':
				lnsize--;
				continue;

			case '\n':
				if (lnsize >= sizeof aline)
					ERROR(errrtn = ERRLINE);
				linenum++;
				lnsize = 0;
				break;

			default:
				break;
			}
		else
			if (c == ' ') {
				if (++compress == 0177) {
					putc(0200 | 0177, &ftmp2);
					outleng++;
					compress = 0;
				}
				continue;
			}
		if (compress != 0) {
			putc(0200 | compress, &ftmp2);
			outleng++;
			compress = 0;
		}
		putc(c, &ftmp2);
		outleng++;
	}
/***/	if (lnsize != 0)
		ERROR(errrtn = 0);
	fflush(&ftmp2);
	errrtn = putupdt(fp, outleng, workfid);
errlabl:
	close(ftmp1.io_fid);
	return(nerror(errrtn));
}
