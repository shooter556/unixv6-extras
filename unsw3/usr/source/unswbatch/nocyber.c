#
/*	nocyber - enable/disable cyber job spooling
 *
 *	if called with first letter 'n', then no spooling.
 *
 */

#include	"batch.h"

main( argc, argv ) char **argv;
{
	if( actflag ){ prints("batch not active\n"); exit(1); }

	nocyb = argv[0][0]=='n';
	exit(0);
}
