#include	"batch.h"

/*
 *	cancel - cancel card reader, line printer & local user jobs.
 *
 *					daveh.  march '78
 */

main(argc, argv)
char	**argv;
{
	register int	i;
	int	zwait;

	if ( !comok())
		error("Not allowed\n");

	if (actflag < 0)
		error("batch not active\n");

	if (argc < 2)
		error("Usage: cancel [cr] [lp] [macro] [ftn] [shell]\n");

	for (i = 1; i < argc; i++ )
	{
		switch ( (argv[i][1] << 8) | argv[i][0] )
		{

		case 'cr':
			scrdflag = 1;
			break;

		case 'lp':
			if ( !fork() )
			{
				execl("/bin/prstop", "prstop", 0);
				execl("/usr/bin/prstop", "prstop", 0);
				error("Can't execute 'prstop'\n");
			}
			waitx(&zwait);
			break;

		case 'ma':
			if (lmacpid)
				kill(lmacpid, sigkil);
			else
				prints("No macro process\n");
			break;

		case 'ft':
		case 'fo':
			if (lftnpid)
				kill(lftnpid, sigkil);
			else
				prints("No ftn process\n");
			break;

		case 'sh':
			if (lshlpid)
				kill(lshlpid, sigkil);
			else
				prints("No shell process\n");
			break;

		default:
			prints(argv[i]);
			prints(" ??\n");
			break;
		}
	}
	exit(0);
}

error(s)
{
	prints("cancel: "); prints(s);
	exit(1);
}
