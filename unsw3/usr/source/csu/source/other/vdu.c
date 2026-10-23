#

/*
 * General startup for terminal dependent programs
 */

#include "/usr/sys/param.h"
#include "/usr/sys/tty.h"

char	partab[1];	/* for tty.h */

main(argc, argv)
int	argc;
char	*argv[];
{
	char	workb[200];
	register char	*p, *p1, *p2;
	int	oldtty[4];

	if (ttyn(1) == 'x') {
		write(2, "Terminals only\n", 15);
		return(1);
	}
	gtty(1, oldtty);
	p = workb;
	for (p1 = "/lib/vdu/"; (*p = *p1++) != '\0'; p++);
	p2 = p;
	for (p1 = argv[0]; (*p = *p1++) != '\0';)
		if (*p!='/' || *p1=='\0')
			p++;
		else
			p = p2;
	*p++ = '.';

	if (((oldtty[0] >> 8) & 0377) == 13)
		p1 = "gt40";
	else if (oldtty[2] & TDATA)
		p1 = (oldtty[2] & LCASE) ? "tdata" : "tdatax";
	else
		p1 = "vt05";
	while ((*p++ = *p1++) != '\0');

	argv[argc] = 0;
	execv(workb, argv);

	write(2, "Not available for this terminal\n", 32);
	return(1);
}
