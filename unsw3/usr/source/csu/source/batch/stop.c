#include "batch.h"
/*
 *	stop the batch system by getting pids & killing 'em
 */
main()
{
	int register	i;
	register struct proc	*p;

	if (!comok())
		error("Not allowed\n");

	if (actflag < 0 )
		error("Batch not active\n");

	pp(qlock);    /* obtain exclusive use of the queues */

	p = &procs[0]; i = 0;
	do
	{
		kill(procids[i++], SIGTERM);
	} while( (++p)->pfile );

	vv(qlock);	/*   in case some clot re-stops */
}
error(s)
{
	prints("Stop: ");
	prints(s);
	exit(1);
}
