#define	ENXIO	6

/*
 *	logon - grab export login account and log in.
 *
 *					daveh.	march '78
 */

char	*cmdsu	"/dev/cyb.cmdsu";
char	*eilog	".eilog";
char	buf[100];	/* should be big enough ... */

main()
{
	register int	cupfd;
	register int	logfd;
	register int	n;

	logfd = open(eilog, 0);
	if (logfd < 0)
		error("Can't open log file\n");

	if ((n = read(logfd, buf, sizeof buf)) <= 0)
		error("Can't read log file\n");

	while ((cupfd = open(cmdsu, 1)) < 0)
		if (cupfd != ENXIO)
			error("Can't open cyber line\n");
		else
			sleep(1);
	while ( write(cupfd, buf, n) != n ) sleep(1);
	exit(0);
}
error(s)
{
	prints(2, "logon: ");
	prints(2, s);
	exit(1);
}
