#

/*
 * command line routines:-
 *		parsing
 *		find the input file
 */

#include "gtl.h"

/*
 * parse: analyse all of input arguments and save flags and filename
 */
parse(argc, argv)
int	argc;
char	*argv[];
{
	register char	*cp;
	register int	i;

	file = NULL;
	do {
		cp = *argv++;
		if (*cp == '-')
			/* switch handler */
			while (*++cp != '\0')
				switch(*cp) {

				case '6':
					load6 = 1;
					break;

				case '7':
					load7 = 1;
					break;

				case '8':
					load8 = 1;
					break;

				case 'd':
					odtflag++;
					break;

				case 'h':
					helpflag++;
					break;

				case 'l':
					listflag++;
					break;

				case 'r':
					rawflag++;
					break;

				default:
					fputs("Unknown switch - ", stderr);
					putc(*cp, stderr);
					fputs("\n", stderr);
					return(-1);
				}
		else {
			if (file != NULL) {
				fputs("Too many files\n", stderr);
				return(-1);
			}
			file = cp;
		}
	} while (--argc != 0);

	if (helpflag || listflag) {
		if ((helpflag && listflag) ||
		    (load6 || load7 || load8) ||
		    file!=NULL) {
			fputs("Bad argument combination\n", stderr);
			return(-1);
		}
		return(0);
	}
	i = load6 + load7 + load8;
	if (i > 1) {
		fputs("Only one loader please!\n", stderr);
		return(-1);
	}
	if (i == 0)
		DFLTLOAD++;

	if (file == NULL) {
		fputs("No file specified\n", stderr);
		return(-1);
	}
	return(0);
}

/*
 * Search for the file mentioned in the arguments.
 * The search sequence is:-
 *		file-name	directory
 *	1.	<file>		.
 *	2.	<file.gtl>	.
 *	3.	<file>		LIBRARY
 *	4.	<file.gtl>	LIBRARY
 *
 * note:	steps 2. and 4. are passed if <file> already
 *		has a suffix.
 *		steps 3. and 4. are skipped if <file> is a
 *		pathname (i.e. contains "/").
 */
getfile()
{
	char	work[100];
	register char	*cp;
	int	step;
	char	bigpath, qualif;

	bigpath = 0;
	qualif = 0;
	for (cp = file; *cp != '\0'; cp++)
		if (*cp == '/') {
			bigpath++;
			qualif = 0;
		} else if (*cp == '.')
			qualif++;

	for (step = 0; step < 3; step++) {
		if (step >= 2) {
			if (bigpath)
				continue;
			strcpy(work, LIBRARY);
		} else
			strcpy(work, "");

		strcat(work, file);

		if (step & 1) {
			if (qualif)
				continue;
			strcat(work, ".gtl");
		}
		if (freopen(work, "r", stdin) != NULL)
			return(0);
	}
	fputs("Can't find file\n", stderr);
	return(-1);
}
