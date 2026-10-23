#include	"batch.h"
/*
 *	status - give status of batch system.
 *
 *					daveh.	march '78
 */
int	fout;

main()
{
	register	active;

	if (actflag >= 0)
		active++;
	else
	{
		if (open(fullckp, 2) != 3)
			error("ckp open error\n");
		if (read(3, chkptp, chkptl) != chkptl)
			error("ckp read error\n");
	}
	fout = dup(1);
	printf("Status of Batch System at %s\n", ijtime());

	if (!active) printf("Batch not active\n\n");
	if (local) printf("Local system running\n\n");
	printf("Basetime:%16t%s", ctime(basetime));
	if (norec) printf("\nCyber output suspended by \"n\"\n\n");
	printf("Current work file:%30t%10s\n", conv(filen+1));
	printf("Jobs in queues:%30t%10d\n", njobs);
	printf("Cyber jobs in queues:%30t%10d\n", ncybjobs);
	if (active)
	{
		if (nocrd) printf("Card reader:%32tdisabled\n");
		if (noprt) printf("Line printer:%32tdisabled\n");
		if (nocyb) printf("Cyber jobs:%32tdisabled\n");
		if (nosub) printf("Submitting:%32tdisabled\n");
		if (noloc) printf("Local jobs:%32tdisabled\n");
		if (nomac) printf("Macro jobs:%32tdisabled\n");
		if (noftn) printf("Fortran jobs:%32tdisabled\n");
		if (noshl) printf("Shell jobs:%32tdisabled\n");
		if (scrdflag) printf("Job on card reader:%33tflushed\n");
		if (*cybmsg) printf("Last cyber message was:%30t%s\n", cybmsg);
		printf("Time limit:%30t%10u seconds\n", timelimit);
		printf("Card limit:%30t%10u cards\n", cardlimit);
		printf("Recv limit:%30t%10u sectors\n", recvlimit);
		printf("Cjob limit:%30t%10u cyber jobs\n", maxcjobs);
		printf("Sbmt limit:%30t%10u lines (cyber jobs)\n", sbmtlimit);
	}
	flush();
	exit(0);
}
error(s)
{
	prints("Status: ");
	prints(s);
	exit(1);
}
