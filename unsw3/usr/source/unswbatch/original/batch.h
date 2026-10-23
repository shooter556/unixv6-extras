#
/*	this file describes all sorts of beaut things that the unreal system
 *	wants to know about
 *
 *
 *					ian j. march  '76
 */
#define	nqs	200	/* no. of queue elts */
#define	qeltlen	30	/* length of each elt */
struct quelt  {
/*must*be*1*/struct quelt *qlink;	/* link to other q elements. */
/*must*be*2*/char jobn[7];		/* jobname from job card. */
/*must*be*3*/char cjobn[3];		/* jobname from cyber. */
		char *rdtime;		/* read-time.	min */
		char *bgtime;		/* send-time-*-start local exec.  min */
		char *ndtime;		/* reci-time-*-end local exec.	  min */
		char *prtime;		/* start print-time.		  min */
		char *extime;		/* system complete time.	  min */
		char  origin;		/* where it came from.		      */
#define	statrln	22	/* 'jobn' thru 'origin'	for stats */
		char  qtype;		/* what	queue it is on.		      */
		char *npfile;		/* file	# for program.		      */
		char *ndfile;		/* file	# for data.		      */
		char *nofile[1];		/* file	#'s for	output.		      */
		};
/************ i	calculate the length to	be 30 if it changes then
		dq.s & text1.s must be altered	 *************/
/*
 *	origins
 */
#define	ogcrd  01	/* read from card	*/
#define	ogterm 02	/* initiated from terminal */
#define	ogcyb  04	/* disposed from the cyber */
#define	tyloc  010	/* local job */
#define	tycyb  020	/* cyber job */
#define	tyerr  040	/* error job */
/*
 *	queue  type
 */
#define	free	  0
#define	forcyber  1
#define	print	  2
#define	atcyber	  3
#define	midi	  4	      /* maths only */
#define	macro	  5
#define	ftn	  6
#define	cc	  7
#define	pascal	8
#define	list	9
#define	file	10
#define	login	11
/*
 *
 */
struct proc {
	char *pfile;
	char *pdesc;
	char pfd1;
	char pqtype;
	int  *pfd2;
	};
struct	      {
	char lowbyte;
	char highbyte;
	};
/*
 *
 */
	int
		actflag,	/*   if	++actflag non-zero then	batch active */
		local,		/* if non-zero then "local" started */
		nocyb,		/* if non-zero then no cyber jobs */
		errno,
		listenf,	      /*	0==> list all cyber crap
					1==> don't list	it		*/
		mypid,	      /*   id of current process */
		monid,	      /*   monitor process id */
		lpdid,	      /*   lpd id. zero	=> not active */
		chkpt(),	      /*   function to take check points on '3'	*/
		chkptp[],		/* address of checkpoint area */
		chkptl,			/* length of checkpoint	area */
		crfnm(),	      /*   function to create a	file,returns file # */
		*filen,		 /*  used when allocating file numbers */
		qput(),	      /*   place entry on appropriate queue */
		procids[],	      /*   ids of active processes	*/
#define	printpid procids[0]
#define	jtdpid	 procids[1]
#define	splpid	 procids[2]
#define	earpid	 procids[3]
#define	crdpid	 procids[4]
#define	macpid	 procids[5]
#define	midpid	 procids[6]
#define	ftnpid	 procids[7]
#define	cccpid	 procids[8]
#define	paspid	procids[9]
#define	lispid	procids[10]
#define	filpid	procids[11]
#define	logpid	procids[12]
		nspj,	      /*   init	to max jobs in system */
		njobs,	       /*   number of jobs not in queues */
		ncybjobs,		/*  max	number of jobs in queues associated
					with cyber.		  */
		nblks,	      /*   init	to max blocks on disk used */
		pp() ,	      /*   grab	resource */
		vv(),	      /*   free	resource */
		dtime(),	      /*   returns time	since basetime in minutes */
		qlock[],	      /*   used	to control queue access	*/
		jupfd,
		statfd,
		jdnfd,
		cupfd,
		cdnfd,
		crdfd;
		char ffnm[]	, /* start of file name */
		fnm[]		, /* file name in work directory	*/
		fnme[] ; /* possible extension (null terminated) */
		long basetime;		/* seconds since 1970 */
		long time();
extern struct proc  procs[];
extern struct quelt queue[],queuend[],*qget();
extern struct {
	struct quelt *qptr;
	int qnum;
} qhdr[];

/*
 *
 */
/* flags set by	the 'cancel' command */
int scrdflag ; /* ==1 -> flush current job on reader*/
