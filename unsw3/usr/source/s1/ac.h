/*
 *	data and structure definitions
 */

#include	<wtmp.h>

/*
#define	DIAGNOSTIC_REDIRECT		/* allows flag 'x' to redirect diagnostics to file "diagf" */
/*
#define	DEBUG				/* allows flag 'y' to turn on debugging */
/*
#define	GARBAGE_COLLECTION		/* allows for garbage collection in "ualloc" if "sys brk" fails */
/*
#define	TIME_IN_DAYS			/* allows conversion of time printout to days if >300 hours */


/*
 *	FLAGS -- program parameters
 */

struct	flags
{
	char	d;			/* details by day (default month) */
	char	f;			/* use form-feeds */
	char	h;			/* give statistics over 24 hours */
	char	n;			/* no warnings */
	char	o;			/* assume input produced by "cnvtwtmp" */
	char	p;			/* details of people */
	char	s;			/* details of system (default) */
	char	t;			/* details of ttys */
	char	w;			/* specify alternate wtmp file */
#	ifdef	DIAGNOSTIC_REDIRECT
	char	x;			/* diagnostic redirection requested */
#	endif
#	ifdef	DEBUG
	char	y;			/* DEBUG */
#	endif
	char	z;			/* (initialised flag) */
}
	flg;

#define	DATE_CHANGE	0200		/* bit set in flag if large -ve date change has occurred */

/*
 *	sort orders for people
 */

int	bycpu();			/* flag 'C' */
int	bylog();			/* flag 'L' */
int	byname();			/* flag 'N' */
int	bylogons();			/* flag 'S' */
int	byuid();			/* flag 'U' */

int	(*porder)();			/*  pointer to chosen order */



/*
 *	times
 */

#define	YEAR		31557600L
#define	DAY		86400L
#define	HOUR		3600

long	year;				/* end of current year */
long	month;				/* end of current month */
long	midnight;			/* end of current day */
long	hour;				/* end of hour */
long	lasthour;			/* end of last hour */
long	timenow;			/* current time */
long	lastprint;			/* time of last print out */


/*
 *	U_ENT -- the user structure -- initialised in getuser()
 */

#define	HASHSIZ		251
#define	uhash(A)	((A)%HASHSIZ)

struct	u_ent
{
	struct u_ent	*ue_next;	/* next structure in chain */
	unsigned	ue_uid;
	char		ue_flags;
	char		ue_logins;	/* count of current logins */
	unsigned	ue_multilogins;	/* count of multiple logins */
	unsigned	ue_logons;	/* count of all logons */
	char		*ue_namep;
	long		ue_asynctime;
	long		ue_cputime;
	long		ue_logtime;
}
	*uhasht[HASHSIZ];		/* the hash table */

/*	ue_flags	*/
#define	LOGIN		01		/* person is currently logged in */
#define	UPDATE		02		/* person has done something in last period */
#define	NEWHR		04		/* data has been collected for this hour */

int	nu_ents;			/* count of user structures */


/*
 *	TTY -- the tty structure
 */

#define	NTTYS		0177

struct	tty
{
	struct u_ent	*t_userp;	/* non-zero if user logged in */
	unsigned	t_flags;
	unsigned	t_logons;	/* count of all logons */
	long		t_logintime;	/* time user logged in */
	long		t_logtime;	/* total logged */
}
	ttys[NTTYS+1];			/* the ttys */

#define	BADTTY		(&ttys[NTTYS])
#define	LAST_TTY	(&ttys[NTTYS+1])
#define	gtty(A)		(&ttys[((A)->w_ttyid&0177)])	/* expand to check tty */


/*
 *	CLOCK -- hourly clock structure
 */

struct	clock
{
	unsigned	c_ttys;		/* ttys used during this hour */
	unsigned	c_logons;	/* logons during this hour */
	unsigned	c_logoffs;	/* logoffs during this hour */
	long		c_logtime;	/* time logged during this hour */
	long		c_cputime;	/* cpu time used this hour */
	long		c_async;	/* async time used this hour */
}
	hours[24],			/* the clock */
	*hourp;				/* the hour hand */

#define	CHISTCOLS	30		/* number of columns to be used by histogram */


/*
 *	SYS_ENT -- system totals
 */

struct	sys_ent
{
	long	st_boottime, st_uptime;
	long	st_syscpu, st_usercpu;
	long	st_async;
	int	st_boots;
	int	st_crashes;
}
	sys;


/*
 *	miscellany
 */

#define	PTIMSIZ		8		/* string size produced by ptime() */

char	*name;				/* program invoked name */

#define	max(A,B)	(((A)>(B))?A:B)
