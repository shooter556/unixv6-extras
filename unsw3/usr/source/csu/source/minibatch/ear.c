/*
 *	ear - listen to the ravings of export
 *
 *	just continually write from "/dev/cyb.cmdsd"
 *	to standard output.
 */

char	*cmdsd	"/dev/cyb.cmdsd";

main()
{
	char	buf[1000];
	register	n, k;
	extern	fout;

	fout = 2;
	close(0);
	if ( open(cmdsd, 0) != 0 )
		error("Can't open cyber line\n");

	for (;;)
	{
		n = read(0, buf, sizeof buf);
		buf[n++] = '\n';
		for ( k = -1; k <= n; )
			buf[k =+ 50] = '\n';
		write(1, buf, n);
	}
}

error(s)
char	*s;
{
	printf("Ear: %s", s);
	flush();
	exit(1);
}
