/*
 * Configuration conditional information
 */
#ifdef	CPU34
#define		BACKUP
#endif	CPU34
#ifdef	CPU40
#define		BACKUP
#endif	CPU40
#ifdef	CPU45
#define		SEP_ID
#define		STACKLIMIT
#define		DISPLAY
#define		SPL
#endif	CPU45
#ifdef	CPU60
#define		FPU
#endif	CPU60
#ifdef	CPU70
#define		FPU
#define		SPL
#define		STACKLIMIT
#define		UNIBUSMAP
#define		SEP_ID
#define		CPUERR
#define		DISPLAY
#endif	CPU70

/*
 * tunable variables
 */

#define	NINODE	100		/* number of in core inodes */
#define	NFILE	100		/* number of in core file structures */
#define	NMOUNT	8		/* number of mountable file systems */
#define	NEXEC	3		/* number of simultaneous exec's */
#define	MAXMEM	(64*32)		/* max core per process - first # is Kw */
#define	SSIZE	20		/* initial stack size (*64 bytes) */
#define	SINCR	20		/* increment of stack (*64 bytes) */
#define	NOFILE	15		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	100		/* size of core allocation area */
#define	SMAPSIZ	100		/* size of swap allocation area */
#define	NCALL	20		/* max simultaneous time callouts */
#define	NPROC	50		/* max number of processes */
#define	NTEXT	40		/* max number of pure texts */
#define	NCLIST	200		/* max total clist size */
#define	HZ	50		/* Ticks/second of the clock */
#define	NTFREE	2		/* no. of free blocks in timout freelist */
#define	NRBUF	10		/* size of raw buffer pool */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	-100
#define	PINOD	-90
#define	PRIBIO	-50
#define	PREDIR	-1
#define	PDELAY	-1
#define	PPIPE	1
#define	PWAIT	40
#define	PPV	40
#define	PSLEP	90
#define	PUSER	100

/*
 * signals
 * dont change
 */

#define	NSIG	20
#define		SIGHUP	1	/* hangup */
#define		SIGINT	2	/* interrupt (rubout) */
#define		SIGQIT	3	/* quit (FS) */
#define		SIGINS	4	/* illegal instruction */
#define		SIGTRC	5	/* trace or breakpoint */
#define		SIGIOT	6	/* iot */
#define		SIGEMT	7	/* emt */
#define		SIGFPT	8	/* floating exception */
#define		SIGKIL	9	/* kill */
#define		SIGBUS	10	/* bus error */
#define		SIGSEG	11	/* segmentation violation */
#define		SIGSYS	12	/* sys */
#define		SIGPIPE	13	/* end of pipe */
#define		SIGTERM	14	/* terminate */
#define		SIGCLOK	15	/* real time limit */
#define		SIGTLMT	16	/* time limit - see sys4.c */

/*
 * fundamental constants
 * cannot be changed
 */

#define	USIZE	16		/* size of user block (*64) */
#define	NULL	0
#define	NODEV	(-1)
#define	ROOTINO	1		/* i number of all roots */
#define	DIRSIZ	14		/* max characters per directory */

/*
 * structure to access an
 * integer in bytes
 */
struct
{
	char	lobyte;
	char	hibyte;
};

/*
 * structure to access a
 * long in integers
 */
struct {
	int	hiint;
	int	loint;
	};

/*
 * structure to remove sign when
 * converting from int to long
 */
struct {
	unsigned int	unsign;
	};

/*
 * structure to access an integer
 */
struct
{
	int	integ;
};

/*
 * Certain processor registers
 */
#define PS	0177776
#define KL	0177560
#define SW	0177570
#ifdef	STACKLIMIT
#define	SLR	0177774	/* stack limit register */
#endif	STACKLIMIT
#ifdef	CPUERR
#define	CPUEREG	0177772	/* cpu error register */
#endif	CPUERR
