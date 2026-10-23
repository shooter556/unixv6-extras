#

/*
 * downline program loader for the GT40 display computer.
 *
 * it is capable of loading either "a.out" or "absolute loader"
 * format program files.
 */

#include "gtl.h"

int	tty6[4] { 006415, 0, 000000, 0 };
int	tty8[4] { 006415, 0, 000040, 0 };

/*
 * note: to use the 8-bit loading modes (-7 & -8), then the
 *	super special modified unsw tty driver is required.
 */


main(argc, argv)
int	argc;
char	*argv[];
{
	extern	dead();
	register	tmp;

	if (argc <= 1) {
		sndhelp();
		return(0);
	}
	if (parse(--argc, ++argv) < 0)
		return(1);
	if (helpflag) {
		bighelp();
		return(0);
	}
	if (listflag) {
		fputs("gt40 library:-\n", stderr);
		execl("/bin/ls", "ls", LIBRARY, 0);
		puts("Can't exec \"ls\"");
		return(1);
	}
	if (odtflag)
		if (loadodt() < 0)
			return(1);
	if (getfile() < 0)
		return(1);
	if (loadfile() < 0)
		return(1);
	if (gtty(fileno(stdout), oldtty) < 0)
		nottty++;
	if (valid() < 0)
		return(1);
	if (pauseflag)
		sleep(3);
	signal(1, dead);
	signal(2, dead);
	signal(3, dead);
	signal(14, dead);
	setbuf(stdout, calloc(1, BUFSIZ));
	dumpit();
	stty(fileno(stdout), oldtty);	/* restore */
	fputs("\nAll done!\n", stderr);
	return(0);
}

/*
 * here if someone has hit the 'del' key when
 * the program was outputting.  shall clean up the mess?
 */
dead()
{
	if (!nottty) {
		stty(fileno(stdout), oldtty);
		fputs("\nZotted!\n", stderr);
	}
	exit(9);
}

sndhelp()
{
	puts("flags are:-");
	puts(" -6, -7, -8 : select loader");
	puts(" -d : load odt");
	puts(" -h : help listing");
	puts(" -l : list gt40 library");
}

bighelp()
{
	puts("\tUse of the downline loader for the GT40.\n");
	puts("Usage: \"gtl [-switches] [file]\"");
	puts("Loads the specified file into the");
	puts("current terminal after first examining the format");
	puts("of the file for validity.");
	puts("valid switches are :-");
	puts("\t-\tthe null switch is equivalent to \"-l\".");
	puts("\t-d\tload a debugger into the gt40 along with the");
	puts("\t\tuser's program.");
	puts("\t-h\tproduce this listing (note: no file allowed)");
	puts("\t-l\tlist the contents of the gt40 library.");
	puts("\t\t(again no file allowed)");
	puts("\t-6\tuse the builtin 6-bit ascii loader supplied");
	puts("\t\twith the GT40 machine.");
	puts("\t-7\tusing the 6-bit loader load an 8-bit loader.");
	puts("\t\tthis in turn is used to load the user program.");
	puts("\t-8\tload the user's program using the UNSW built");
	puts("\t\t8-bit loader.");
	puts("\tnote:\tthe 3 switches -6, -7 & -8 are regarded as");
	puts("\t\tmutually exclusive.");
	puts("\t-r\tsuppress the normal checking for memory conflicts");
}
