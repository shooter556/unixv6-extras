#include	"batch.h"
/*
 *	tell - sends its args to cyber.
 *		also performs some local processing
 *
 *					daveh.	march '78
 */

#define	BUFL	18

char	*name	"x: ";

main(argc, argv)
char	**argv;
{
	register char	*argp;
	register	i;
	register struct quelt	*q;
	int	cupfd;

	/* retain last component of pathname */

	argp = argv[0];
	while (*argp)
		if (*argp++ == '/')
			argv[0] = argp;

	name[0] = argv[0][0];

	if (!comok())
		error("Not allowed\n");

	if (local)
		error("Local running\n");

	if (actflag < 0)
		error("Batch not active\n");

	if (open(fullckp, 2) != 3)
		error("Can't open ckp file\n");

/*
 * rip thru arg list, compacting it into one arg.
 *
 * i assume args are stored consecutively ...
 */
	if (i = argc - 1)
		do
			argv[i][-1] = ',';
		while (--i);

/* check size of message */

	argp = argv[0];
	i = 0;
	while ( *argp++ ) i++;
	if ( i > BUFL )
		error("Message too long\n");

/* check special processing */

	switch ( argv[0][0] )
	{

	case 'b':		/* bye */
		bsw++;
		break;

	case 'd':		/* drop job from cyber q */
		if (argc != 2)
			error("arg count\n");
		if ( (q=getcyb(argv[1])) != 0 )
			qput(q, free);
		else
			prints("D: warning - job not in BATCH queue\n");
		break;

	case 'e':		/* eradicate job sending/rec */
		if (argc != 2)
			error("arg count\n");
		switch ( (argv[1][1] << 8) | argv[1][0] )
		{

		case 'cr':
			kill(jtdpid, 7);
			break;

		case 'lp':
			kill(splpid, 7);
			break;
		}
		break;

	case 'l':		/* log on */
		if ( bsw )
		{
			kill(jtdpid, 6);
			kill(splpid, 6);
			bsw = 0;
			break;
		}

		if (argc == 1)	/* 'l' by itself */
		{
			/* since we are already logged on, do a reset */

			argv[0][0] = 'b';
			argv[0][1] = '\0';	/* send a 'bye' */
			i = 1;
		}
		break;

	case 'n':		/* enable/disable output */
		if (argc == 1)
			norec++;
		else
			if (argc == 2 && argv[1][0] == 'c')
				norec = 0;
		break;

	case 'o':		/* redirect job */
		if (argc != 3)
			error("arg count\n");
		if ((q = getcyb(argv[1])) != 0)
		{
			if (((argv[2][1] << 8) | argv[2][0]) == 'al')	/* alt q */
			{
				q->origin =| altq;
				qput(q, atcyber);
			}
			else		/* send elsewhere - we don't want to know about it */
				qput(q, free);
		}
		else
			prints("O: warning - job not in BATCH queue\n");
		break;

	case 'u':		/* unload job */
		kill(splpid, 6);
		break;
	}

/* send command */
	signal(2, 0);	signal(3, 0);	signal(1, 0);

	while ( (cupfd = open("/dev/cyb.cmdsu", 1)) < 0) sleep(1);
	while ( write(cupfd, argv[0], i) != i )
	{
		prints("Last command not yet sent - retrying in 5 seconds\n");
		sleep(5);
	}

	exit(0);
}

error(s)
{
	prints(name);
	prints(s);
	exit(1);
}
