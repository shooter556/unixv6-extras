/*
 *	tell - sends its args to cyber.
 */

#define	BUFL	23

int	fout;

char	*cmdsu	"/dev/cyb.cmdsu";

main(argc, argv)
char	**argv;
{
	int	fd;
	register char	*argp;
	register	i;

	fout = 2;
	if ( argc == 0 )
		error("No args\n");

/* retain last component of pathname */

	argp = argv[0];
	while ( *argp )
		if ( *argp++ == '/' )
			argv[0] = argp;

/*
 * rip thru arg list, compacting it into one arg.
 *
 * i assume args are stored consecutively ...
 */
	for ( i = 1; i < argc; i++ )
		argv[i][-1] = ',';

/* check size of message */

	argp = argv[0];
	i = 0;
	while ( *argp++ ) i++;
	if ( i > BUFL )
		error("Message too long\n");

/* send command */

	while ( (fd = open(cmdsu, 1)) < 0 )
	{
		printf("line busy - retrying in 5 seconds\n");
		flush();
		sleep(5);
	}

/* send message */

	while ( write(fd, argv[0], i) != i )
	{
		printf("Last command not yet sent - retrying in 5 seconds\n");
		flush();
		sleep(5);
	}

	exit(0);
}

error(s)
{
	printf("tell: %s", s);
	flush();
	exit(1);
}
