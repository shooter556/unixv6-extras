/*
 * One structure allocated per active
 * process. It contains all data needed
 * about the process while the
 * process may be swapped out.
 * Other per process data (user.h)
 * is swapped with the process.
 */
struct	proc
{
	char	p_stat;
	char	p_flag;
	char	p_pri;		/* priority, negative is high */
	char	p_sig;		/* signal number sent to this process */
	char	p_uid;		/* user id, used to direct tty signals */
	char	p_time;		/* resident time for scheduling */
	char	p_cpu;		/* cpu usage for scheduling */
	char	p_nice;		/* nice for scheduling */
	int	p_ttyp;		/* controlling tty */
	int	p_pid;		/* unique process id */
	int	p_ppid;		/* process id of parent */
	int	p_addr;		/* address of swappable image */
	int	p_size;		/* size of swappable image (*64 bytes) */
	int	p_wchan;	/* event process is awaiting */
	int	*p_textp;	/* pointer to text structure */
	unsigned p_rtl;		/* count of secs left of real time limit */
	unsigned p_stl;		/* count of secs left of sleep time */
} proc[NPROC];

/* stat codes */
#define	SSLEEP	1		/* sleeping on high priority */
#define	SWAIT	2		/* sleeping on low priority */
#define	SRUN	3		/* running */
#define	SIDL	4		/* intermediate state in process creation */
#define	SZOMB	5		/* intermediate state in process termination */
#define	SSTOP	6		/* process being traced */

/* flag codes */
#define	SLOAD	01		/* in core */
#define	SSYS	02		/* scheduling process */
#define	SLOCK	04		/* process cannot be swapped */
#define	SSWAP	010		/* process is being swapped out */
#define	STRC	020		/* process is being traced */
#define	SWTED	040		/* another tracing flag */

/*
 * Redefinition of the proc structure for use when
 * a process is in the zombie state.
 * Some fields of the normal proc structure are no
 * longer available, being otherwise used.
 * These are: p_uid, p_time, p_addr, p_size, p_wchan, p_textp.
 */
struct	pzomb {
	char	p_stat;
	char	p_flag;
	char	p_pri;
	char	p_sig;
	int	pz_ur0;
	char	p_cpu;
	char	p_nice;
	int	p_ttyp;
	int	p_pid;
	int	p_ppid;
	long	pz_utime;
	long	pz_stime;
	};

struct proc	*high_proc;	/* address of highest used entry in proc table */
