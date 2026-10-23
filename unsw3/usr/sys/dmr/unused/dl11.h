/*
 * data used in coordinating the KL driver with other drivers on the same controller
 */

#define	FIRSTDL 2	/* first DL11 involved */
#define	NDL	2	/* no. of DL11s involved */

char	dl11[NDL];	/* coordinating status */

#define TTY	1	/* value of dl11 if tty */
#define	BLK	2	/* value of dl11 if block device */
#define	LPR	4	/* value of dl11 if line printer */
#define COM	010	/* value of dl11 if communications protocol being used */

/*
 *	now assign the devices
 */
#define	VS	0	/* selects element of dl11 & dl11v */
#define	VSADDR	0175610	/* interrupt vector at 310 (dl11-e) */

#define	BKL	0	/* selects element of dl11 */
#define	BKLADDR	0175610	/* interrupt vector at 310 (dl11-e) */

#define BP	1	/* selects element of dl11 */
#define BPADDR	0175620	/* interrupt vector at 320 (dl11-d) */

struct {	/* structure for accessing DL registers */
	int	dlrs;	/* DL reading status register ( RCSR ) */
	int	dlrb;	/* "	"	buffer ( RBUF ) */
	char	dlws;	/* "  writing status register ( XCSR ) */
	char	dlfill;
	char	dlwb;	/* "	"	buffer ( XBUF ) */
};
#define RDENB	01	/* status register bits */
#define IENABLE 0100
#define DONE	0200

#define E_OVRUN		040000		/* rbuf bits */
#define E_FRAME		020000	
#define E_PARITY	010000

/*
 *	locations defined in l.s
 */
struct  {
	int	rcall[2];
	int	(*rintad)();
	int	wcall[2];
	int	(*wintad)();
}  dl11v[NDL];

/*
 *	structure used for accessing memory during block transfers
 */
struct	abs
{
  char	*addr[2];	/* pointers to physical memory */
  int	wcount;		/* allows memory access while -ve */
};

#define set	=|
#define reset	=& ~
