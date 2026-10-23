#
#include "../defines.h"
#include "../param.h"
#include "../file.h"
#ifdef	ONCE
#include "../conf.h"
#include "../filsys.h"
#include "../tty.h"
#include "../buf.h"
#endif
#include "../user.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"
#include "../seg.h"

#define	CLOCK1	0177546
#define	CLOCK2	0172540
/*
 * Icode is the octal bootstrap
 * program executed in user mode
 * to bring up the system.
 */
int	icode[]
{
	0104413,	/* sys exec; init; initp */
	0000014,
	0000010,
	0000777,	/* br . */
	0000021,	/* initp: init+5; 0 
			   use init as name -> looks nicer on ps */
	0000000,
	0062457,	/* init: </etc/init\0> */
	0061564,
	0064457,
	0064556,
	0000164,
};

#ifdef	BIG_UNIX
extern	char	eto[], eu[];
#endif

#ifdef PROFILE
extern char	pshift[],pmask[],pfreq[],etext[];
int	profstart;
int	pnbytes;
long	pusertix;
#endif PROFILE

/*
 * Initialization code.
 * Called from m40.s or m45.s as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	find which clock is configured
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap
 *
 * panic: no clock -- neither clock responds
 * loop at loc 6 in user mode -- /etc/init
 *	cannot be executed.
 */
main()
{
	extern schar;
	register i, *p;

	/*
	 * zero and free all of core
	 */

#ifndef	BIG_UNIX
	i = *ka6 + USIZE;
#else	BIG_UNIX
	i = eu + USIZE;
#endif	BIG_UNIX
#ifdef	_1170
	p = MEMCNTRL->integ;
	MEMCNTRL->integ =| 3;		/* parity traps here can be ugly */
#endif	_1170
	UISD->r[0] = 077406;
	for(;;) {
		UISA->r[0] = i;
#ifndef	BIG_UNIX
		if(suiword(0,0) < 0)	/* fix027 */
			break;
		clearseg(i);
#else	BIG_UNIX
		if ( fuibyte(0) < 0 )
			break;
#endif	BIG_UNIX
		maxmem++;
	/*	mfree(coremap, 1, i);	fix027 */
		i++;
	}
#ifdef	_1170
	MEMCNTRL->integ = p;
#endif	_1170
#ifndef	BIG_UNIX
	mfree( coremap , maxmem , *ka6 + USIZE );	/* fix027 */
#else	BIG_UNIX
	mfree( coremap , maxmem , eu + USIZE );
#endif	BIG_UNIX
#ifdef	UNIBUS_MAP & ! MAPPED_BUFFERS
/*	if(cputype == 70)	*/
	for(i=0; i<62; i=+2) {
		UBMAP->r[i] = i<<12;
		UBMAP->r[i+1] = 0;
	}
#ifdef	MALLOC_UMAP
	/* unibus-map 1->8 (really 0->7) point at kernal buffer pool etc */
	mfree( umap , 23 , 9 ); /* free unibus-map 9->32 */
#endif	MALLOC_UMAP
#endif	UNIBUS_MAP & ! MAPPED_BUFFERS
#ifdef	MAPPED_BUFFERS
	maxmem =- NBUF*8;	/* this space is grabbed for buffer pool */
#endif	MAPPED_BUFFERS
	printf("       mem = %lKb\n\n", maxmem/16);				/* fix011 */
/*	printf("RESTRICTED RIGHTS\n\n");				fix012 */
/*	printf("Use, duplication or disclosure is subject to\n");	fix012 */
/*	printf("restrictions stated in Contract with Western\n");	fix012 */
/*	printf("Electric Company, Inc.\n");				fix012 */

	maxmem = min(maxmem, MAXMEM);
	mfree(swapmap, nswap, swplo);

	/*
	 * determine clock
	 */

	UISA->r[7] = ka6[1]; /* io segment */
	UISD->r[7] = 077406;
#ifndef	PROG_CLOCK
	lks = CLOCK1;
	if(fuiword(lks) == -1) {
#endif
		lks = CLOCK2;
		if(fuiword(lks) == -1)
			panic("no clock");
#ifndef	PROG_CLOCK
	}
#endif

#ifdef PROFILE
	if(lks.integ == CLOCK2)
		printf("clock in use -- cannot profile\n");
	else
	{	int	j;	int	*p2;
		struct {
			int	kwpcsr,kwpcbuffer,kwpcounter;
		};
#define CLOCKINIT 0111
#define SISA0 0772240
#define SISD0 0772200
		pnbytes = (((etext << pshift) & ~pmask) + 3) & ~03;
		pusertix = 0;	/* redundant really */
		j = (pnbytes + 63) >> 6;
		if((i = malloc(coremap,j)) == NULL)
			panic("no core for profile\n");
		profstart = i;
		p = SISA0;
		p2 = SISD0;
		do {
			*p++ = i;
			*p2++ = 077406;
			i =+ 0200;
			j =- 0200;
		} while(j > 0);
		*--p2 = (j + 0177) << 8 | 06;	/* appropriate size, R/W */
		/* lets get going.. */
		CLOCK2->kwpcbuffer = pfreq;
		spl7();
		printf("profiling about to go\n");
		printf("data area start : %o\n",&profstart);
		printf("interrupt routine start : %o\n",0104->integ);
		spl0();
		CLOCK2->kwpcsr = CLOCKINIT;
	}
#endif PROFILE
#ifdef	STACK_LIMIT
	/*
	 *	if stack-limit register exists
	 *	use it !!
	 */

	if( fuiword(SLR) != -1)
	{	i = &u ;
		SLR->integ = i + sizeof u;
	}

#endif

#ifdef	MEM_PAR_INTR & _1140
	/*
	 *	if memory-parity interrupts exist - use them.
	 */
	p = PARMEMCSR;
	while ( fuiword( p ) != -1 )
		*p++ = 1;	/* enable interrupts */
#endif	MEM_PAR_INTR & _1140

	/*
	 * set up system process
	 */

	proc[0].p_addr = *ka6;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag =| SLOAD|SSYS;
	u.u_procp = &proc[0];

	/*
	 * set up 'known' i-nodes
	 */

	*lks = 0115;
	cinit();
	binit();
	iinit();
	rootdir = iget(rootdev, ROOTINO);
	rootdir->i_flag =& ~ILOCK;
	u.u_cdir = iget(rootdev, ROOTINO);
	u.u_cdir->i_flag =& ~ILOCK;

	/*
	 * make init process
	 * enter scheduling loop
	 * with system process
	 */

	if(newproc()) {
		expand(USIZE+1);
		estabur(0, 1, 0, 0);
#ifndef	_1170 | _1145
		copyout(icode, 0, sizeof icode);
#endif
#ifdef	_1170 | _1145
		copyout(icode, 0, sizeof icode, SEG_USD);
#endif
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
	sched();
}

#ifndef	ONCE
#include "../estabur.h"
#endif

#ifdef	ONCE
#include "../iinit.h"

#include "../binit.h"

#include "../cinit.h"

#endif
