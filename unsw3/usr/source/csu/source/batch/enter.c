#include	"batch.h"
/*
 * enter - set various limits in the system.
 *
 * link to it by appropriate name.
 *
 * format is en?? num
 *					daveh.	march '78
 */

main(argc, argv)
char	**argv;
{
	register	i;
	register char	*ap;

	if (!comok())
		error("Not allowed\n");

	if (actflag < 0)
		error("batch not active\n");

	if ( argc != 2 )
		error("usage: en?? limit (where ?? is tl,cl,jl,rl or sl)\n");

	ap = argv[0];
	while (*ap)
		if (*ap++ == '/')
			argv[0] = ap;

	i = atoi(argv[1]);	/* get number */

	switch ( (argv[0][3] << 8) | argv[0][2])
	{

	case 'tl':		/* time limit max */
		timelimit = i;
		break;

	case 'cl':		/* card limit on reader */
		cardlimit = i;
		break;

	case 'jl':		/* cyber job limit */
		if (i > 100)
			error("JOb limit must be <= 100\n");
		else
			maxcjobs = i;
		break;

	case 'rl':		/* receive limit (sectors) */
		recvlimit = i;
		break;

	case 'sl':		/* cyber submit limit */
		sbmtlimit = i;
		break;

	default:
		prints(argv[0]);
		prints(" ??\n");
	}

	exit(0);
}
error(s)
{
	prints("enter: ");
	prints(s);
	exit(1);
}
