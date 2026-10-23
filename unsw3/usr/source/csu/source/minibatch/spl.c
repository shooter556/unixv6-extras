/*
 *	spl - receive jobs from cyber.
 *	just dump straight to line printer.
 */

char	*jobsd	"/dev/cyb.jobsd";
char	*lp	"/dev/lp";

main()
{
	char	buf[1100];
	register	n;
	extern	fout;

	fout = 2;  close(0);  close(1);

	if ( open(jobsd, 0) != 0 )
		error("Can't open cyber line\n");

	for (;;)
	{
		while ( (n=read(0, buf, sizeof buf)) <= 0 )
			;

		while ( open(lp,1) != 1 ) sleep(1);

		write(1, buf, n);
		while ( (n=read(0, buf, sizeof buf)) >= 0 )
			if (n>0) write(1, buf, n);
		write(1, "\f", 1);
		close(1);
	}
}

error(s)
char	*s;
{
	printf("spl: %s", s);
	flush();
	exit(1);
}
