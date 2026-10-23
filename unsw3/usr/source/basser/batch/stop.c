#include "batch.h"
/*
 */

main()
{	int register i; register struct proc *p;
	extern zap();

	signal(0, zap); signal(1, zap); signal(2, zap);
	pp(qlock);	/* obtain exclusive use of the queues */
	sync();		/* ensure i/o complete !! */
	sleep(3);	/* ensure i/o complete !! */
	if(actflag = 0) prints("batch not active\n");
	else {
		p= &procs[0]; i=0;
		do {
			kill(procids[i++],BATCHTERMINATE);
		} while( (++p)->pfile );
		}
	vv(qlock);	/* daveh	in case some clot re-stops */
}
zap()
{
	signal(0,zap); signal(1,zap); signal(2,zap);
	prints("\ndon't try to abort 'stop' !!!\n");
	return;
}
