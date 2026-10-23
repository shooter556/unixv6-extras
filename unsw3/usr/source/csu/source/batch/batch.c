#include  "batch.h"

/*
 *	this code will be execed by the "batch" command to start local
 *	batch facilities and remote job submission.
 *
 *						ian j.  march '76
 *						daveh.	march '78
 */
char	rbuff[50]	0;
int	null[1]		0;
int	fbuff[9]	0;

main(argc,argv)
char	**argv;
{
	register struct proc	*p;
	int	i;
	register struct quelt	*pq, *vq;
	int	xt;
	char	*ap;
	int	cold;
	extern	fout;

	fout = 2;
	ap = argv[0];
	while (*ap)		/* retain last pathname */
		if (*ap++ == '/')
			argv[0] = ap;

	if ( ++actflag )
	{
		actflag--;
		error("already active\n", null);
	}

	comtty = ttyn(2);	/* get name of command terminal */

	if (chdir(workdir) < 0)
		error("working directory unattainable\n",null);

	if ( argv[0][0] == 'l' ) local++;	/* local */

	bopen(chkpf, 2);		/* should be '3' */

	if ( !local )
	{
		jupfd = bopen("/dev/cyb.jobsu", 1);
		jdnfd = bopen("/dev/cyb.jobsd", 0);
		cdnfd = bopen("/dev/cyb.cmdsd", 0);
	}
	statfd = bopen(".rawstatistics", 1);

	seek(statfd, 0, 2);	/* seek to end of the file */
	/*
	 *  what are the system parms ?
	 */
	for (i = 1; i < argc; i++ )
		switch ( (argv[i][1] << 8) | argv[i][0])
		{

		case 'co':		/* cold start */
			cold++;
			break;

		case 'tl':		/* time limit */
			timelimit = atoi(&argv[i][2]);
			break;

		case 'cl':		/* card limit */
			cardlimit = atoi(&argv[i][2]);
			break;

		case 'jl':		/* job limit */
			maxcjobs = atoi(&argv[i][2]);
			if (maxcjobs > 100)
				error("job limit > 100\n", null);
			break;

		case 'rl':		/* receive limit */
			recvlimit = atoi(&argv[i][2]);
			break;

		case 'sl':		/* cyber submit lines */
			sbmtlimit = atoi(&argv[i][2]);
			break;

		case 'cy':		/* no cyber reading */
			nocyb++;
			break;

		case 'lo':		/* no local jobs */
			noloc++;
			break;

		case 'su':		/* no submitting */
			nosub++;
			break;

		case 'nr':		/* no receive ( "n" ) */
			norec++;
			break;

		case 'nc':		/* no card reader */
			nocrd++;
			break;

		case 'np':		/* no line printer */
			noprt++;
			break;

		case 'ma':		/* no macro */
			nomac++;
			break;

		case 'fo':
		case 'ft':		/* no fortran */
			noftn++;
			break;

		case 'sh':		/* no shell */
			noshl++;
			break;

		default:
			prints(argv[i]);
			prints(" ??\n");
			exit(1);
		}

	if (!nocrd)
		crdfd = bopen("/dev/cr.batch", 0);

	seek(3, 0, 0);

	if ( read(3, chkptp, chkptl) != chkptl || njobs <= 0 || cold )
		goto coldgold;

	if ( !fork() )
	{
		execl("/bin/dq", "dq", 0);
		exit();
	}
	waitx(&xt);

	if ( xt &0200 ) prints("queues are screwed ...\n");

	prints("re-initialize the batch system ? ");

	read(0, rbuff, sizeof rbuff);

	if ( rbuff[0] == 'c' && rbuff[1] == 'o' &&
           rbuff[2] == 'l' && rbuff[3] == 'd' &&
           rbuff[4] == '\n' )
	{
  coldgold:
/* cold */
		prints("directory being cleared & queues reinitialized\n");
		if ( (i=open(".", 0))<0 ) error("can't access batch directory\n", null);
		seek(i, 32, 0);  /* skip . .. */
		while (read(i, fbuff, 16) == 16)
		{
			if (fbuff[0] && fbuff[1].lowbyte=='$' )
				if (unlink(&fbuff[1]) < 0)
				{
					prints("batch: ");
					prints(&fbuff[1]);
					prints(" can't remove\n");
				}
		}
		close(i);
		basetime = time();
		write(statfd, null, 2); write(statfd, &basetime, 4);
		qinit();
		filen = 0; njobs = 0; ncybjobs = 0;
	}
	else
	{

/* warm */	prints("queues being restored\n");
		/* reconstitute the queues */
		for (pq = queue; pq <= queuend; pq++)
			if ( (xt = pq->qlink) & 1 )	/* active job */
				switch ( xt = (pq->qtype) )
				{

				case free:	/* read/receive/submit */
					write(2, pq->jobn, 7);
					prints(" - was being read/received/submitted\n");
				freeit:
					if (pq->ndfile)
					{
						nunlink(pq->ndfile);
						pq->ndfile = 0;
					}
					if (pq->npfile)
					{
						nunlink(pq->npfile);
						pq->npfile = 0;
					}
					xt = free;	njobs--;
				putit0:
					if (pq->nofile)
					{
						nunlink(pq->nofile);
						pq->nofile = 0;
					}
				putit:
					qhdr[xt].qnum++;
					pq->qlink = qhdr[xt].qptr;
					qhdr[xt].qptr = pq;
					break;

				case forcyber:	/* was sending to cyber */
					write(2, pq->jobn, 7);
					prints(" - was being transmitted\n");
					goto putit;

				case print:		/* was spooling */
					write(2, pq->jobn, 7);
					prints(" - was spooling\n");
					goto putit;

				case atcyber:		/* was receiving */
					write(2, pq->jobn, 7);
					prints(" - was being received\n");
					goto putit0;

				default:
					write(2, pq->jobn, 7);
					prints(" - was executing\n");
					goto putit;
				}
	}
	chkpt();

	if (!local)
		if (!fork())
			execl(".logon", ".logon", 0);
		else
			waitx(&xt);

	if ( fork() ) exit(0);
	/*
	 *    activate standard processes
	 */
	p = &procs[0];
	do
	{
		while ( (i = fork()) < 0 )
			sleep(1);
		/* fork finally forked */
		if ( i > 0 )
		{
			procids[ p - &procs[0] ] = i;
			continue;
		}
		/* child */
		close(p->pfd1);   dup(*p->pfd2);   close(!p->pfd1);
		for (i = 4; i < 15; i++)
			close(i);
		execl(p->pfile, p->pfile, 0);
		error(p->pfile, " exec failed\n");
	} while ( (++p)->pfile);
	/*
	 *    now let the monitor watch these tasks
	 */
	for (i = 4; i < 15; i++)
		close(i);
	close(0); close(1);
	execl(".monitor", local ? "* local *" : "* batch *", 0);
	error("can't exec monitor\n", null);
}
bopen(fn,md)
char	*fn;
{
	int register	i;

	if ((i = open(fn, md))<0)
		error(fn, ": can't open\n");

	return(i);
}
error(a, b)
{
	prints("batch: ");  prints(a);  prints(b);
	exit(-1);
}

qinit()
{
	register struct quelt	*q;
	register int	j, *p;
	int	i;

	q = queue;

	for ( i = 0; i < nqs; i++ )
	{
		p = q;
		j = qeltlen >> 1;

		do *p++ = 0; while (--j);
		q->qlink = p;
		q++;
	}

	queue[nqs-1].qlink = 0;

	for ( j = 0; j < nqt; j++ )
	{
		qhdr[j].qnum = 0;
		qhdr[j].qptr = 0;
	}

	qhdr[free].qnum = nqs;
	qhdr[free].qptr = queue;
	return;
}
