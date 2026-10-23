#

/*
 * Error message routines
 */

#include "slup.h"

/*
 * printer error message outputting routine
 */
lerror(fmt, arg)
char	*fmt, *arg;
{
	register int	i;
	int	shift;

	errflag = 1;
	if (prntmode != 0) {
		kickline();
		shift = prntmode==1 ? 32 : 24;
		for (i = 0; i < shift; i++)
			putchar('>');
		printf(fmt, arg);
	}
	return(ERR);
}

/*
 * terminal error message routine
 */
terror(fmt, arg)
char	*fmt, *arg;
{
	register int	oldfout;

	errflag = 1;
	fflush(&fout);
	oldfout = fout.io_fid;
	FOUTSET(&fout, 1);
	doprompt();
	printf(fmt, arg);
	printf("\n");
	fflush(&fout);
	FOUTSET(&fout, oldfout);
	return(ERR);
}

char	*errmsgs[] {
	"I/O error",
	"Can't open old file",
	"Can't create new file",
	"Can't create temporary file !!",
	"Insufficient memory",
	"Too many arguments",
	"Insufficient arguments",
	"Input line too long",
	"Illegal character in input text",
	"Name too long",
	"First character of name not alphabetic",
	"Illegal character in name",
	"reference to modset being deleted",
	"Unknown modification directive",
	"Non-digit encountered",
	"No file specified for modification!",
	"No modset name specified",
	"Specified line outside module",
	"Already passed specified position",
	"Only single character prefixes allowed",
	"Reference to yanked modset",
	"File not in library",
	"Unique filename required",
	"Modset not in file",
	"Duplicate modification set name",
	"Unknown group",
	"Duplicate group name",
	"Module already in library",
	"Too many nested calls",
	};

/*
 * specific error messages
 */
error(errnum, arg)
int	errnum;
char	*arg;
{
	register char	*s;

	if (errnum >= 0)
		return(errnum);
	errflag = 1;
	if (errnum>=ERR || errnum<=ERRIMPL) {
		if (errnum != ERRIMPL)
			return(errnum);
		s = "Not yet implemented";
	} else
		s = errmsgs[ERR - errnum - 1];
	lerror(s);
	terror(s);
	return(ERR);
}

/*
 * set file-name for prompting
 */
setprompt(s)
char	*s;
{
	promptdone = 0;
	filename = s;
}

/*
 * print description and file-name
 */
doprompt()
{
	register int	oldfout;

	if (!promptdone) {
		promptdone++;
		fflush(&fout);
		oldfout = fout.io_fid;
		FOUTSET(&fout, 1);
		if (filename != thismod)
			printf("%s - %s:\n", descrip, filename);
		else
			printf("%s - \"%s\":\n", descrip, filename);
		fflush(&fout);
		FOUTSET(&fout, oldfout);
	}
}

/*
 * error message with line number
 */
nerror(errnum, arg)
int	errnum;
char	*arg;
{
	register int	oldfout;

	if (errnum>=0 || errnum==ERR)
		return(errnum);
	fflush(&fout);
	oldfout = fout.io_fid;
	FOUTSET(&fout, 1);
	printf("%u: ", linenum);
	fflush(&fout);
	FOUTSET(&fout, oldfout);
	return(error(errnum, arg));
}

/*
 * update mode error message:
 *	line number
 *	list command etc.
 */
uerror(flag, errnum, arg)
int	flag;	/* allow echo */
int	errnum;
char	*arg;
{
	extern char	cmdline[150];

	if (flag && (subopts & OPTUPRT)) {
		kickline();
		printf("\t\t????????");
		println(cmdline);
	}
	nerror(errnum);
}
