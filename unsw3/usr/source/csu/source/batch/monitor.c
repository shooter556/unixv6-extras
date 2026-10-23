#include	"batch.h"

/*
 *	this program is execed by the process which forked all the batch
 *	processes.  its purpose is to wait until all these processes die,
 *	print a message and die.
 *
 *					ianj.	march '76
 *					daveh.	march '78
 */
main()
{
	int	status[1];

	prints("\n*** batch now active ***\n");
	while ( waitx(status) != -1 );
	chkpt();
	prints("\n** batch has completed\n");
}
