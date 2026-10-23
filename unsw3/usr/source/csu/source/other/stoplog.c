#include	<param.h>
#include	<proc.h>

#define	GETPROC	0

/*
 *	stoplog - stop the errorlogger process.
 *
 *	look thru process table & assume process sleeping on
 *	"errlog+2" is the errorlogger ...
 *					daveh. 22 mar 78
 */

struct
{
	char	name[8];
	int	type, value;
} nl[]
{
	"_errlog", 0, 0,
	"\0\0\0\0\0\0\0\0", 0, 0
};

main()
{
	register struct proc	*p;
	register	addr, i;

	nlist("/unix", nl);
	if (nl[0].type == -1)
		error("Error logger device not present in system\n");

	addr = nl[0].value + 2;

	for (i = 0; i < 10; i++)	/* try 10 times for device - it may be busy */
	{
		gettable(GETPROC, proc, sizeof proc);

		for (p = &proc[0]; p < &proc[NPROC]; p++)
			if (p->p_stat == SWAIT && p->p_wchan == addr)	/* got it */
				if (kill(p->p_pid, SIGTERM) < 0)
					error("Can't signal error logger\n");
				else
					exit(0);

	}
	/* didn't find it */

	error("Can't locate error logger\n");
}

error(s)
{
	prints(2, "Stoplog: ");
	prints(2, s);
	exit(1);
}
