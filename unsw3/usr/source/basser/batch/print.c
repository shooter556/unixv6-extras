#include "batch.h"
char	tfile[]	 "bpd/x/dfxXXXXXX";
char	lpd[]	"/etc/bpd";	/* BASSER daemon is differrent */
char	lock[]	"bpd/lock";
int	ok	1;	/* zero to stop print */
stopit()
{
	signal(13,stopit);
	ok = 0;
}
main(argc)
{
	extern char *mktemp(),*gfnm();
	register int f;
	register struct quelt *q;
	char buf[36];

	if(fstat(3,ccbuf) < 0) exit();
	signal(13, stopit);
	chdir("/spool");
	while( ok )
	{
		if( (q=qget(print)) == 0) exit();
		q->prtime = dtime(); /* note starting time */
		if(!fork())
		{
			f = creat(mktemp(&tfile[6]), 0600); /* df file */
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
			if(stat(lock,buf) < 0)
			{
				execl(lpd,lpd+5,0);
			}
			exit();
		}
		else wait(buf);
		q->extime = dtime();
		write(1,q->jobn,statrln);
		qput(q,free);
		if( qhdr[print].qnum == 0) sleep(180);
	}
}
