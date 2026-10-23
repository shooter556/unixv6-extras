#

/*
 * Switches and argument processing
 */

#include "slup.h"

/*
 * Scan entire argument list scanning for switches.
 * Any non-switches are left piled up at bottom of list.
 */
switches(argc, argv, xargc, xargv)
int	argc;
char	*argv[];
int	*xargc;
char	**xargv[];
{
	register char	*cp;
	char	**newv;
	int	newc;
	int	anf, someopts, badopts;
	int	argsreq, argsbad;
	int	errrtn;

	*xargv = argv;
	newv = argv;
	func = 0;
	options = 0;
	subopts = 0;
	verbose = 0;
	dfltescape = '*';
	badopts = 0;
	argsreq = 0;
	argsbad = 0;
	for (newc = 0; --argc >= 0; argv++)
		if (*(cp = *argv) != '-') {
			*newv++ = cp;
			newc++;
		} else {
			anf = 0;
			someopts = 0;
			switch (*++cp) {

			case 'r':	/* r[v] */
				someopts = OPTREPL;
			case 'i':	/* i[v] */
				anf = INSERT;
				if (cp[1] == 'v') {
					cp++;
					verbose = 1;
				}
				argsreq++;
				badopts =| OPTGRPS | OPTXTR | OPTPRNT;
				break;

			case 'c':	/* c[+/-][v] */
				anf = COMMON;
				if (cp[1] == '+')
					someopts = OPTCOMM;
				else if (cp[1] != '-')
					cp--;
				cp++;
				if (cp[1] == 'v') {
					cp++;
					verbose = 1;
				}
				badopts =| OPTGRPS | OPTXTR | OPTPRNT;
				break;

			case 'd':	/* d[v], du[v].. */
				argsreq++;
				if (cp[1] != 'u')
					anf = DELETE;
				else {
					cp++;
					anf = DELUPDT;
					if (cp[1] == 'a') {
						cp++;
						someopts = OPTDUAL;
					}
				}
				badopts =| OPTXTR | OPTPRNT;
				if (cp[1] == 'v') {
					cp++;
					verbose = 1;
				}
				if (anf==DELUPDT && !(someopts & OPTDUAL)) {
					/* modsets */
					if ((errrtn = getlist(&cp[1], &dmodlist)) < 0)
						return(errrtn);
					cp = "z";
				}
				break;

			case 'e':	/* e? - (escape character) */
				if (cp[1] == '\0')
					dfltescape = '*';
				else {
					cp++;
					dfltescape = *cp;
				}
				break;

			case 'g':	/* g/.. gm/.. gr/.. gi/.. gd/.. */
				switch (*++cp) {

				default:	/* g/.. */	/* group selection */
					someopts = OPTGRPS;
					if ((errrtn = getlist(cp, &grplist)) < 0)
						return(errrtn);
					break;

				case 'm':	/* gm/.. */	/* make groups */
					if ((errrtn = getlist(&cp[1], &mgrplist)) < 0)
						return(errrtn);
					break;

				case 'r':	/* gr/.. */	/* remove groups */
					if ((errrtn = getlist(&cp[1], &rgrplist)) < 0)
						return(errrtn);
					break;

				case 'i':	/* gi/.. */	/* insert files */
					anf = INSGRP;
					argsreq++;
					if ((errrtn = getlist(&cp[1], &igrplist)) < 0)
						return(errrtn);
					break;

				case 'd':	/* gd/.. */	/* drop files */
					anf = DELGRP;
					argsreq++;
					if ((errrtn = getlist(&cp[1], &dgrplist)) < 0)
						return(errrtn);
					break;
				}
				cp = "z";
				break;

			case 'l':	/* l[a][g][m][x][d][v] */
				someopts = OPTLIST;
				while (cp[1] != '\0') {
					switch (*++cp) {

					case 'a':	/* all */
						subopts =| OPTLGRP|OPTLMOD|OPTLXRF|OPTLDOC|OPTLBIG;
						continue;

					case 'd':	/* modset documentation */
						subopts =| OPTLDOC;
						continue;

					case 'g':	/* groups */
						subopts =| OPTLGRP;
						continue;

					case 'm':	/* modsets */
						subopts =| OPTLMOD;
						continue;

					case 'v':	/* big */
						subopts =| OPTLBIG;
						continue;

					case 'x':	/* modset cross-reference */
						subopts =| OPTLXRF;
						continue;
					}
					cp--;
					break;
				}
				break;

			case 'o':	/* o file */
				if (argc <= 0)
					return(terror("No file name"));
				someopts = OPTOFIL;
				badopts =| OPTOFIL;
				lstname = *++argv;
				argc--;
				break;

			case 'p':	/* p[a][l][d][y][c][s][v] */
				while (cp[1] != '\0') {
					switch (*++cp) {

					case 'a':	/* all */
						subopts =| OPTPCOM | OPTPDEL | OPTPYNK | OPTPDIR;
						continue;

					case 'c':	/* common files */
						subopts =| OPTPCOM;
						continue;

					case 'd':	/* deletions */
						subopts =| OPTPDEL;
						continue;

					case 'l':	/* directives */
						subopts =| OPTPDIR;
						continue;

					case 's':	/* no compile stuff */
						subopts =| OPTXNCL;
						continue;

					case 'y':	/* yanked stuff */
						subopts =| OPTPYNK;
						continue;

					case '0':	/* basic only */
						someopts =| OPTPRNT;
						continue;

					case 'v':	/* verbose */
						verbose = 1;
						continue;
					}
					cp--;
					break;
				}
				if (someopts==0 && !(subopts & (OPTPCOM | OPTPDEL | OPTPYNK | OPTPDIR)))
					subopts =| OPTPDEL;
				someopts =| OPTPRNT;
				badopts =| OPTPRNT;
				break;

			case 'x':	/* x[s][v], xo[v], xu[v][/..].. */
				switch (*++cp) {

				default:	/* x[s][v] */
					someopts = OPTXTR;
					cp--;
					while (cp[1] != '\0') {
						switch (*++cp) {

						case 's':
							/* extract current */
							subopts =| OPTXNCL;
							continue;

						case 'C':
							/* extract C mode */
							subopts =| OPTXCC;
							continue;

						case 'v':
							verbose = 1;
							continue;
						}
						cp--;
						break;
					}
					break;

				case 'o':	/* xo[v] */
					anf = EXORIG;
					badopts =| OPTXTR | OPTPRNT;
					if (cp[1] == 'v') {
						cp++;
						verbose = 1;
					}
					break;

				case 'u':	/* xu[v][/...].. */
					anf = EXMODS;
					badopts =| OPTXTR | OPTPRNT;
					if (cp[1] == 'v') {
						cp++;
						verbose = 1;
					}
					/* modsets */
					if ((errrtn = getlist(&cp[1], &xmodlist)) < 0)
						return(errrtn);
					cp = "z";
					break;
				}
				break;

			case 't':	/* t[p][v] */
				someopts = OPTXTR | OPTTEST;
				goto l1;

			case 'u':	/* u[p][v] */
				badopts =| OPTTEST;
			l1:
				anf = UPDATE;
				if (cp[1] == 'p') {
					cp++;
					subopts =| OPTUPRT;
				}
				if (cp[1] == 'v') {
					cp++;
					verbose = 1;
				}
				badopts =| OPTGRPS;
				break;

			case 'n':	/* n */
				someopts = OPTNEWL;
				if (cp[1] == 'v') {
					cp++;
					verbose = 1;
				}
				badopts =| OPTGRPS | OPTXTR | OPTPRNT;
				break;

			case 'v':	/* v */
				verbose = 1;
				break;

			default:
				return(terror("Illegal switch - %s", *argv));
			}

			if (cp[1] != '\0')
				return(terror("Unknown subswitch - %s", *argv));

			if (func != 0) {
				if (anf != 0)
					return(terror("Only one major function - %s", *argv));
			} else
				func = anf;
			if (options & badopts)
				return(terror("Incompatible switches - %s", *argv));
			if (argsbad && argsreq)
				return(terror("EH? - %s", *argv));
			options =| someopts;
			badopts =& ~someopts;
		}

	*xargc = newc;

	if (argsbad && newc!=0)
		return(terror("No files allowed with these switches"));
	else if (argsreq && newc==0)
			return(terror("Files are required"));

	switch (func) {

	case DELUPDT:
		if (dmodlist.s_cnt == 0)
			return(ERRIMPL);
		break;

	case INSGRP:
		if (igrplist.s_cnt == 0)
			return(ERRIMPL);
		break;

	case DELGRP:
		if (dgrplist.s_cnt == 0)
			return(ERRIMPL);
		break;

	default:
		break;
	}

	return(NOERR);
}
