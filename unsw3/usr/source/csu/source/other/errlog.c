#

/*
 *	system error logger
 *
 *	- reads from system error emitting device
 *	  and writes to error log file.
 *
 *	- will buffer output & continue reading
 *	  if write returns error, then sleeps
 *	  until error goes away.
 *
 *	- will complain  if errors too frequent.
 */

#define	NOTNICE		-90	/* run at extremely high priority */
#define	DATESTAMP	5	/* don't date stamp more often than this */
#define	MAXERR		10	/* max. allowable errors in DATESTAMP period */
#define	SIGTERM		14

int	ou;
char	inbuf[80];
char	oubuf[512];
char	*oubufp		oubuf;

char	errlogf[]	"/usr/adm/errlog";
#define	errlogm		"Cannot open /usr/adm/errlog\n",28

char	errdevf[]	"/dev/errlog";
#define	errdevm		"Cannot open /dev/errlog\n",24

#define	seekm		"Error - seek\n",13

int	errcount;
int	totalerrs;
#define	panicm		"\nerrlogger - too many errors!\n",30


main(argc, argv)
char *argv[];
{
	register in, n;
	long this, last, time();
	int finish();

	close(0);
	close(1);
	if ((in = open(errdevf, 0)) < 0) {
		write(2, errdevm);
		return(1);
	}
	if ((ou = open(errlogf, 1)) < 0) {
		write(2, errlogm);
		return(1);
	}
	if (seek(ou, 0, 2) < 0) {
		write(2, seekm);
		return(1);
	}

	nice(NOTNICE);
	chdir("/");

	if (fork())
		return(0);

	signal(1, 1);
	signal(2, 1);
	signal(3, 1);
	signal(SIGTERM, finish);
	write(2, "error logger started\n", 21);
	write(ou, "******* error logger started: ", 30);
	write(ou, ctime(time()), 25);
	last = 0;

	while ((n = read(in, inbuf, sizeof inbuf)) >= 0)
		if (n != 0) {
			errcount++;
			totalerrs++;
			this = time();
			if ((this - last) > DATESTAMP) {
				copyout(ctime(this), 25);
				last = this;
				if (errcount > MAXERR) {
					write(2, panicm);
					sleep(DATESTAMP);
				}
				errcount = 0;
			}

			copyout(inbuf, n);
		}

	finish();
}

finish()
{
	write(ou, "******* error logger completed: ", 32);
	write(ou, ctime(time()), 25);
	write(2, "error logger completed\n", 23);
	if (totalerrs) {
		writen(totalerrs);
		write(2, " errors logged\n", 15);
	}
	exit(0);
}

writen(n)
register n;
{
	register r;
	int c;

	if ((r = n/10) != 0)
		writen(r);
	c = n%10 + '0';
	write(2, &c, 1);
}

copyout(cp, n)
register char *cp;
register n;
{
	register char *bp = oubufp;
	int z;

	while (bp < &oubuf[sizeof oubuf] && n--)
		*bp++ = *cp++;

	oubufp = oubuf;
	z = bp - oubuf;

	while (write(ou, oubuf, z) != z)
		if ((sizeof oubuf - z) < sizeof inbuf)
			sleep(1);
		else {
			oubufp = bp;
			break;
		}

	if (n > 0)
		copyout(cp, n);
}
