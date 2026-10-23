#

/*
 *	slp - set printer/plotter modes
 *
 *	slp [+] [0-9] [flags]
 *
 *	+ - use std output for device
 *	digit - /dev/lp?
 *	default - /dev/lp
 */

#define	TRNS	0004
#define	SKIP	0010
#define	THROW	0020
#define	PACK	0040
#define	PLTTR	0100
#define	FEED	0200
#define	NOEJECT	0400

struct {
	char	*string;
	int	set, reset;
} modes[] {
	"eject",	0, NOEJECT,
	"-eject",	NOEJECT, 0,
	"feed",		FEED, 0,
	"-feed",	0, FEED,
	"plot",		PLTTR, 0,
	"-plot",	0, PLTTR | PACK,
	"print",	0, PLTTR | PACK,
	"pack",		PACK | PLTTR, 0,
	"-pack",	0, PACK,
	"skip",		SKIP, 0,
	"cdc",		SKIP, 0,
	"-skip",	0, SKIP,
	"-cdc",		0, SKIP,
	"trns",		TRNS, 0,
	"half",		TRNS, 0,
	"-trns",	0, TRNS,
	"full",		0, TRNS,
	"throw",	THROW, 0,
	"-throw",	0, THROW,
	0
};

char	*arg;
int	fd	0;
int	mode[4];
char	*lp	"/dev/lp\0";	/* note spare character */

main(argc, argv)
char *argv[];
{
	char	c;
	register int	i;
	extern fout;

	fout = 2;

	if (argc > 1) {
		c = argv[1][0];
		if (c == '+') {
			fd = 1;
			argc--;
			argv++;
		} else if (c<='9' && c>='0') {
			lp[7] = c;
			argc--;
			argv++;
		}
	}
	if (!fd && (fd = open(lp, 1))<0) {
		printf("Device busy or non-existent\n");
		return(1);
	}
	if (gtty(fd, mode) < 0) {
		printf("Gtty failed\n");
		return(1);
	}
	if (argc == 1) {
		prmodes();
		return(0);
	}

	while (--argc > 0) {
		arg = *++argv;

		if (eq("width")) {
			mode[2] = atoi(*++argv);
			argc--;
		}
		if (eq("length")) {
			mode[1] = atoi(*++argv);
			argc--;
		}

		for (i = 0; modes[i].string != 0; i++)
			if (eq(modes[i].string)) {
				mode[0] =& ~modes[i].reset;
				mode[0] =| modes[i].set;
			}

		if (arg != 0)
			printf("Unknown mode: %s\n", arg);
	}

	if (stty(fd, mode) < 0) {
		printf("Stty failed\n");
		return(1);
	}

	return(0);
}

eq(string)
char *string;
{
	register int	i;
	register char	*s;

	if (arg == 0)
		return(0);

	s = string;
	i = 0;
	do {
		if (arg[i] != s[i])
			return(0);
	} while (arg[i++] != '\0');

	arg = 0;
	return(1);
}

prmodes()
{
	register int	m;

	m = mode[0];

	if (!(m & PLTTR))
		printf("Width=%d cols   Length=%d lines\n", mode[2], mode[1]);

	if (m & TRNS)
		printf("half ");
	if (m & SKIP)
		printf("cdc ");
	if (m & THROW)
		printf("throw ");
	if (m & PLTTR)
		printf("plot ");
	if (m & PACK)
		printf("pack ");
	if (m & FEED)
		printf("feed ");
	if (m & NOEJECT)
		printf("-eject ");

	printf("\n");
}
