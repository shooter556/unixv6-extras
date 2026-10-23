#
/*
 *	this file describes all sorts of beaut things that the batch system
 *	wants to know about.
 *					ian j. march  '76
 *					daveh. march  '78
 */

#define	nqt	8	/* no. of queues */
#define	nqs	100	/* no. of queue elts */
#define	qeltlen	32	/* length of each elt */

typedef
	unsigned	unsin;

struct quelt  {
/* must be 1 */	struct quelt	*qlink;	/* link to other q elements. */
/* must be 2 */	char	jobn[7];	/* jobname from job card. */
/* must be 3 */	char	cjobn[3];	/* jobname from cyber. */
		unsin	rdtime;		/* read-time.	min */
		unsin	bgtime;		/* send-time-*-start local exec.  min */
		unsin	ndtime;		/* reci-time-*-end local exec.	  min */
		unsin	prtime;		/* start print-time.		  min */
		unsin	extime;		/* system complete time.	  min */
		char	origin;		/* where it came from. */
#define	statrln	22	/* 'jobn' thru 'origin'	for stats */
		char	qtype;		/* what	queue it is on. */
		unsin	tlimit;		/* time limit (secs) for local job */
		unsin	npfile;		/* file	# for program. */
		unsin	ndfile;		/* file	# for data. */
		unsin	nofile;		/* file # for output */
	};
/************ i	calculate the length to	be 32 if it changes then
		dq.s, qlook.c & text1.s must be altered	 *************/
/*
 *	origins & flags
 */
#define	ogcrd	01	/* read from card */
#define	ogterm	02	/* initiated from terminal */
#define	ogcyb	04	/* disposed from the cyber */
#define	tyloc	010	/* local job */
#define	tycyb	020	/* cyber job */
#define	tyerr	040	/* error job */
#define	altq	0100	/* placed in alt print q by 'o' */
#define	altd	0200		/* alternate destination */
/*
 *	queue  type
 */
#define	free		0		/* must be zero */
#define	forcyber	1
#define	print		2
#define	atcyber		3
#define	macro		4
#define	ftn		5
#define	list		6		/* list a deck of cards */
#define	shell		7		/* deck contains shell commands */

struct proc
{
	char	*pfile;
	char	*pdesc;
	char	pfd1,
		pqtype;
	int	*pfd2;
};
struct
{
	char	lowbyte;
	char	highbyte;
};

struct
{
	int	integ;
};

	int
		bsw,		/* set by 'b' command */
		actflag,	/* if ++actflag non-zero then batch active */
		local,		/* if non-zero then "local" started */
		errno,
		chkpt(),	/* function to take check points on '3'*/
		chkptp[],	/* address of checkpoint area */
		chkptl,		/* length of checkpoint area */
		crfnm(),	/* function to create a file,returns file # */
		gfnm(),		/* return name of file in 'fnm' */
		comok(),	/* validate batch command */
		qput(),		/* place entry on appropriate queue */
		procids[],	/* ids of active processes */
#define	prtpid	procids[0]
#define	jtdpid	procids[1]
#define	splpid	procids[2]
#define	earpid	procids[3]
#define	crdpid	procids[4]

#define	lqstart	5	/* local processes start here */
#define	macpid	procids[5]
#define	ftnpid	procids[6]
#define	lstpid	procids[7]
#define	shlpid	procids[8]
#define	lqend	8	/* and stop here */

		lprocs[3],	/* pids of user jobs */
#define	lmacpid	lprocs[0]
#define	lftnpid	lprocs[1]
#define	lshlpid	lprocs[2]
		njobs,		/* no. of jobs in queues */
		ncybjobs,	/* no. of cyber jobs in all queues */
		maxcjobs,	/* max. no. of cyber jobs allowed */
		pp(),		/* grab resource */
		vv(),		/* free resource */
		dtime(),	/* returns time since basetime in minutes */
		qlock[],	/* used to control queue access */
		reserved[8],	/* reserved for future use */
		tmjtl,		/* set on 'too many jobs - try later' */
		jupfd,		/* fd for jobs up */
		statfd,		/* fd for stats file */
		jdnfd,		/* fd for jobs down */
		cdnfd,		/* fd for commands down */
		crdfd;		/* fd for card reader */

	unsin	filen,		/* next file sequence no. */
		cardlimit,	/* no. cards limit per job to read */
		timelimit,	/* max time limit (secs) for local job */
		recvlimit,	/* no. buffers to receive from cyber */
		sbmtlimit;	/* no. lines in cyber submit job */

	char	fnm[],		/* first section of file name */
		ffnm[],		/* full directory prefix */
		chkpf[],	/* checkpoint file name */
		fullckp[],	/* full pathname for it */
		workdir[];	/* work directory */

	char	comtty;		/* name of command terminal */

	long	basetime,	/* seconds since 1970 */
		time();

extern struct proc  procs[];

extern struct quelt queue[], queuend[], *qget();

extern struct qhdr
{
	struct quelt	*qptr;
	int		qnum;
} qhdr[];

char	cybmsg[50];	/* broadcast message from cyber */

/* certain flags set by certain commands */

	int
		scrdflag,	/* cancel,cr */
		nocyb,		/* if non-zero then no cyber jobs */
		nosub,		/*  "	  "	"    no submitting */
		norec,		/*  "	  "     "    no receive */
		noloc,		/*		     no local jobs */
		nomac,		/* no macro */
		noftn,
		noshl,
		nocrd,		/* don't start crd */
		noprt;		/* same for print */

#define	SIGTLMT	16	/* cpu time limit */
#define	SIGTERM	14	/* system (or batch) shutdown */
#define	sigkil	9
