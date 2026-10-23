#include "batch.h"

#define	sigon	signal(6, init); signal(7, cancel)
#define	sigoff	signal(6, 1); signal(7, 1)
#define error	{ qtp = print; prints(m7); chdir(workdir); goto err; }

/*
 *	this program controls all the print output that comes off the
 *	line.  it identifies all jobs and queues them for printing,
 *	or possibly for returning to user's directory.
 *
 *	filedescriptors -
 *		      0 - <- cyber
 *		      1	- work
 *		      2 - initiating terminal
 *		      3 - checkpoint file
 *						ian j.	march '76
 *						daveh.	march '78
 */

char m1[] "\nSpl: file create error\n";
char m2[] "\nSpl: job abcdefg/xyz requeued to alternate print queue\n";
#define M2A	17	/* points to end of job name above */
#define M2B	21	/* points to end of cyber suffix above */
char m3[] "\nSpl: Job abcdefg/xyz returned as '$cccccc'\n\n";
#define M3A	17
#define M3B	21
#define M3C	42
char m4[] "LP       0.000 KPG.  $     0.00\n";
char m5[] "o,ab00xyz,alt";
char m6[] "\n\n\n*** Job cancelled by 'e,lp' ***\n\n";
char m7[] "\nSpl: Job abcdefg/xyz cannot be returned; will print instead\n";
#define M7A	17
#define M7B	21

char	buff[1100] ;
int	sig;
int	qtp;

init()	/* reinitialize - job coming down is requeued to cyber */
{
	signal(6, init);
	sig = 6;
}
cancel()	/* reinitialize - job coming down is truncated */
{
	signal(7, cancel);
	sig = 7;
}

struct quelt *jnget()
{
/*
 * this esoteric code attempts to identify the job coming down the
 * line by examining the banner page.
 * it relies on the special format banner page used at unsw.
 */
	char	jname[7];
	register char	*c;
	int register	i;
	register struct quelt	*q;

	/*
	 * if we can't find the message "JOB SYSNAME = AB00XYZ"
	 * in the first buffer, assume garbage but print it anyway.
	 */

	for (c = buff; c < &buff[ (sizeof buff) - 21]; c++ )
		if ( c[0] == 'J' && c[4] == 'S' &&
		     c[7] == 'N' && c[12] == '=' )
		{
			/* gotcha  !!! */
			i = 7;
			do jname[i-1] = m5[i+1] = c[i+13]; while (--i );
			goto gotit;	/* urgh */
		}

	/*
	 * didn't find message.  accept as error job.
	 */
	prints("Spl: format error ?\n");
	i = 7;
	do jname[i-1] = "format?"[i-1]; while (--i);
	goto ferr;

	/*
	 * job name obtained.
	 * see if in atcyber queue.
	 * if so, grab it
	 */
gotit:	if ( (q = getcyb(jname)) != 0 )	/* found the job */
	{
		if (!q->cjobn[0] )	/* no kronos suffix, so fudge one */
		{
			q->cjobn[0] = jname[4];
			q->cjobn[1] = jname[5];
			q->cjobn[2] = jname[6];
		}

		return(q);
	}

	/*
	 * didn't find it, so assume it was
	 * a "dispose" job and construct an entry for it.
	 *
	 * it may also have been a "format error" job.
	 */
ferr:
	if ((q = qget(free)) == 0)
		exit(0);
	q->origin = ogcyb | tycyb;
	q->qtype = atcyber;
	ncybjobs++;
	for (i = 0; i < 7; i++)
		q->jobn[i] = q->cjobn[i-4] = jname[i];
	return(q);
}


main(argc, argv)
char	**argv;
{
	register	n, j;
	register char	*x;
	struct quelt	*q;
	int	sectors;	/* sectors read - decremented */
	int	cupfd;

	if (fstat(3, buff) < 0 || local) exit(0);

	sigoff;

	for (;;)
	{
		sig = 0; qtp = print;

		if ((j = crfnm()) < 0)
		{
			prints(m1);
			exit(-1);
		}

		while ((n = read(0, buff, sizeof buff)) <= 0); /* banner page */
		sectors = recvlimit - 1;
		sigon;

		/* dig out job name being received */
		q = jnget();
		q->nofile = j;
		chkpt();
		write(1, buff, n);

		while ((n = read(0, buff, sizeof buff)) >= 0 && sectors >= 0)	/* read rest of job */
			if (n > 0)
			{
				--sectors;
				while (write(1, buff, n) != n) sleep(5);
			}

		if (sectors < 0 )	/* limit exceeded */
		{
			x = &m2[M2A];	/* end of job name in string */
			j = 7;
			do *--x = q->jobn[j-1]; while (--j);

			x = &m2[M2B];	/* cyber extension */
			j = 3;
			do *--x = q->cjobn[j-1]; while (--j);

			prints(m2);
			q->origin =| altq;	/* for 'dq' to pick up */
			while ((cupfd=open("/dev/cyb.cmdsu", 1)) < 0)
				sleep(1);
			while (write(cupfd, "n", 1) != 1)
				sleep(1);
			while (write(cupfd, "u", 1) != 1)
				sleep(1);
			sleep(10);
			while (read(0, buff, sizeof buff) >= 0);
			sleep(10);	/* unload takes a fair while to process */
			while (write(cupfd, m5, sizeof m5) != sizeof m5)
				sleep(1);
			if (!norec)
				while (write(cupfd, "n,c", 3) != 3)
					sleep(1);
			sigoff;
			close(cupfd);
			sig = 0;
			nunlink(q->nofile);
			q->nofile = 0;
			qput(q, atcyber);
			close(1);
			continue;
		}
		sigoff;
		if (sig)
			while ((n = read(0, buff, sizeof buff)) >= 0)
				if (n > 0)
					while (write(1, buff, n) != n)
						sleep(5);
		write(1, m4, sizeof m4);
		if (sig) write(1, m6, sizeof m6);
		close(1);
		q->ndtime = dtime();
		if (q->origin & altd)
		{
			register char	*rc, *rce;
			register int	i;

			rc = &m3[M3A]; rce = &m7[M7A]; i = 7;
			do *--rc = *--rce = q-> jobn[i-1]; while (--i);
			rc = &m3[M3B]; rce = &m7[M7B]; i = 3;
			do *--rc = *--rce = q->cjobn[i-1]; while (--i);
			rc = &m3[M3C]; i = 6;
			do *--rc = fnm[i]; while (--i);
				/* all that does is preset the messages */

			qtp = free;

			if (getpw(q->tlimit, buff) != 0)
				error;

			rc = buff; i = 5;
			do
			{
				while (*rc++ != ':')
					;
			} while (--i);		/* get home directory */
			rce = rc;
			while (*rce++ != ':');
			rce[-1] = '\0';		/* isolate directory name */

			if (chdir(rc) < 0)	/* to user's dir. */
				error;

			gfnm(q->nofile);
			unlink(fnm);	/* just in case */
			if (link(ffnm, fnm) < 0)
			{
				/* copy file over */
				register	fi, fo, n;

				if ((fo = creat(fnm, 0600)) < 0)
					error;
				if ((fi = open(ffnm, 0)) < 0)
				{
					unlink(fnm);
					error;
				}
				while ((n = read(fi, buff, 512)) > 0)
					write(fo, buff, n);
				close(fi); close(fo);
			}

			chown(fnm, q->tlimit);
			prints(m3);
			mail(m3, sizeof m3 -1, q->tlimit);
			chdir(workdir);		/* back home again */
			ncybjobs--;
			nunlink(q->nofile);
			q->prtime = q->extime = dtime();
		}
	err:	qput(q, qtp);
	}
}
getpw(uid, buf)
char	buf[];
{
	int	pbuf[259];
	int	pwf;
	register	n, c;
	register char	*bp;

	if ((pwf = open("/etc/passwd", 0)) < 0)
		return(1);
	pbuf[0] = pwf;
	pbuf[1] = pbuf[2] = 0;		/* preset buffer */

	for (;;)
	{
		bp = buf;
		while ((c = getc(pbuf)) != '\n')
		{
			if (c <= 0)
			{
				close(pwf);
				return(1);
			}
			*bp++ = c;
		}

		*bp++ = '\0';
		bp = buf; n = 3;
		while (--n)
			while ((c = *bp++) != ':')
				if (c == '\n')
				{
					close(pwf);
					return(1);
				}
		while ((c = *bp++) != ':')
		{
			if (c < '0' || c > '9')
				continue;
			n = n*10 + c - '0';
		}

		if (n == uid)
		{
			close(pwf);
			return(0);
		}
	}
	close(pwf);
	return(1);
}
mail(s, n, u)
char	*s;
{
	register	fid, crt;

	crt = 0;

	if ((fid = open(".mail", 1)) < 0)
	{
		crt++;
		if ((fid = creat(".mail", 0600)) < 0)
			return;		/* bugger it ... */
	}
	if (crt)
		chown(".mail", u);
	else
		seek(fid, 0, 2);	/* to eof */
	write(fid, s, n);
	close(fid);
	return;
}
