/*
 *	Structure to reference a process after it
 *	is dead. This must conform exactly to
 *	'proc.h', except for the things explicitly used.
 */


struct	pzomb
{
	char	pz_stat;
	char	pz_flag;
	char	pz_pri;		/* priority, negative is high */
	char	pz_sig;		/* signal number sent to this process */
#ifndef	AUSAM16
	int	pz_ur0;		/* returned status */
#endif	AUSAM16
#ifdef	AUSAM16
	char	pz_time;
#endif	AUSAM16
	char	pz_cpu;		/* cpu usage for scheduling */
#ifndef	AUSAM16
	char	pz_nice;	/* nice for scheduling */
#endif	AUSAM16
#ifdef	AUSAM16
	int	pz_ur0;		/* returned status */
	unsigned pz_uid;	/* user id, used to direct tty signals */
#endif	AUSAM16
	int	pz_ttyp;	/* controlling tty */
	int	pz_pid;		/* unique process id */
	int	pz_ppid;	/* process id of parent */
	long	pz_utime;					   /* fix000 */
	long	pz_stime;	/* times returned from process */  /* fix000 */
#ifdef	NEW_SLEEP
	unsigned pz_stl;		/* count of secs left of sleep time */
#endif
#ifdef	TIME_LIMITS
	unsigned pz_rtl;		/* count of secs left of real time limit */
#endif
#ifdef	IGNORE_SIGNALS
	unsigned pz_ignsig;	/* boolean array of 1st 16 signals if ignored */
#endif	IGNORE_SIGNALS
#ifdef	AUSAML
	struct lnode *pz_lnode;	/* pointer to limit structure */
#endif	AUSAML
};
