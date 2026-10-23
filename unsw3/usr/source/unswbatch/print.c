#include "batch.h"
char	tfile[]	 "lpd/x/dfxXXXXXX";
char	lpd[]	"/etc/lpd";
int	ok	1;	/* zero to stop print */
stopit()
{
	signal(14,stopit);
	ok = 0;
}
main(argc)
{
	extern char *mktemp(),*gfnm();
	register int f;
	register struct quelt *q;
	char buf[36];
	int lpdpid;

	if(fstat(3,cbuf) < 0) exit();
	signal(14, stopit);
	chdir("/tmp");
	while( ok )
	{
		if( (q=qget(print)) == 0) exit();
		q->prtime = dtime(); /* note starting time */
		if(!fork())
		{
			f = creat(mktemp(&tfile[6]), 0600); /* df file */
			write(f, "I\0\0\0\0", 40);	/* set uid for prstop */
			if( !(q->origin & tycyb) )
			{
	/*put in code for banner page*/
				write(f, "b", 1);	/*batch origin*/
				write(f, q->jobn, 39);	/* lpd record length */
				write(f,"p",40);	/* push */
			}
			else
			{
				ncybjobs++;
				write(f,"*",1);
				write(f,q->jobn,39);
			}
			gfnm(q->nofile[0]);
			write(f, "f", 1);
			write(f, ffnm, 39);
			write(f, "u", 1);
			write(f, ffnm, 39);
			close(f);
			link(&tfile[6], tfile);
			unlink(&tfile[6]);
/* OLD LPD
			if(stat("lpd/lock",buf) < 0)
			{
				execl(lpd,lpd+5,0);
			}
				 OLD LPD */
			if( ((f = open("lpd/lock", 0)) < 0) ||
				(read(f, &lpdpid, 2) != 2) ||
					( kill(lpdpid, 4) < 0) )
			{
				prints("print: lpd not active\n");
			}
			exit();
		}
		else wait(buf);
		q->extime = dtime();
		write(1,q->jobn,statrln);
		qput(q,free);
/*		if( qhdr[print].qnum == 0) sleep(180); ***** no longer necessary - ggr */
	}
}
