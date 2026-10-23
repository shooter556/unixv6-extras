#include	"batch.h"
/*
 *	enable - enable/disable certain batch functions.
 *
 *				daveh.	march '78
 */

char	*a;

main(argc, argv)
char	**argv;
{
	register	endis, i;

	a = argv[0];
	while (*a)
		if (*a++ == '/')
			argv[0] = a;
	a = argv[0];

	if (!comok())
		error("Not allowed\n");

	if (actflag < 0)
		error("batch not active\n");
	if (argc < 2 )
		error("usage: enable/disable [all] [cyber] [submit] [local] [shell] [fortran] [macro]\n");

	endis = argv[0][0] == 'd';

	for (i = 1; i < argc; i++ )
		switch ( (argv[i][1] << 8) | argv[i][0])
		{

		case 'al':		/* everything ... */
			nocyb = nosub = noloc = noshl = nomac = noftn = endis;
			break;

		case 'cy':		/* cyber reading */
			nocyb = endis;
			break;

		case 'su':		/* submitting */
			nosub = endis;
			break;

		case 'lo':		/* local jobs */
			noloc = endis;
			break;

		case 'sh':		/* shell */
			noshl = endis;
			break;

		case 'ma':		/* macro */
			nomac = endis;
			break;

		case 'fo':
		case 'ft':		/* fortran */
			noftn = endis;
			break;

		default:
			prints(argv[i]);
			prints(" ??\n");
			break;
		}
	exit(0);
}
error(s)
{
	prints(a);
	prints(": ");
	prints(s);
	exit(1);
}
