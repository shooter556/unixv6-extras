/*
 * Random set of variables
 * used by more than one
 * routine.
 */
char	canonb[CANBSIZ];	/* buffer for erase and kill (#@) */
int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */
int	*rootdir;		/* pointer to inode of root directory */
int	cputype;		/* type of cpu =40, 45, or 70 */
int	execnt;			/* number of processes in exec */
int	lbolt;			/* time of day in 60th not in time */
long	time;			/* time in sec from 1970 */
/*
 * The timout structure is a redefinition of
 * a clist block used to store information
 * describing the routine to be called in
 * a specified time, and the argument to
 * be passed to it by the clock interrupt.
 * Used, for example, to time tab delays
 * on teletypes.
 */
struct	timout {
	struct timout	*t_next;
	int	t_arg;		/* argument to routine */
	int	(*t_func)();	/* routine */
	int	t_time;
	} timouts;

#define	TFREELIST	timouts.t_func.t_next	/* the timeout reserve freelist */
#define	TFREECOUNT	timouts.t_time	/* number of blocks in reserve freelist */
#define	TOVERLOADCOUNT	timouts.t_arg	/* number of times the freelists were empty */
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
int	mpid;			/* generic for unique process id's */
char	runin;			/* scheduling flag */
char	runout;			/* scheduling flag */
char	runrun;			/* scheduling flag */
char	curpri;			/* more scheduling */
int	maxmem;			/* actual max memory per process */
int	*lks;			/* pointer to clock device */
int	rootdev;		/* dev of root see conf.c */
int	swapdev;		/* dev of swap see conf.c */
int	swplo;			/* block number of swap space */
int	nswap;			/* size of swap space */
int	updlock;		/* lock for sync */
int	rablock;		/* block to be read ahead */
char	regloc[];		/* locs. of saved user registers (trap.c) */
