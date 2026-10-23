#
/*
 */

#include "../defines.h"
#include "../param.h"
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include "../systm.h"
#include "../file.h"
#include "../user.h"
#include "../inode.h"
#include "../reg.h"

/*
 * Max allowable buffering per pipe.
 * This is also the max size of the
 * file created to implement the pipe.
 * If this size is bigger than 4096,
 * pipes will be implemented in LARG
 * files, which is probably not good.
 */
#define	PIPSIZ	4096

/*
 * The sys-pipe entry.
 * Allocate an inode on the root device.
 * Allocate 2 file structures.
 * Put it all together with flags.
 */
pipe()
{
	register *ip, *rf, *wf;
	int r;

	ip = ialloc(rootdev);
	if(ip == NULL)
		return;
	rf = falloc();
	if(rf == NULL) {
		iput(ip);
		return;
	}
	r = u.u_ar0[R0];
	wf = falloc();
	if(wf == NULL) {
		rf->f_count = 0;
		u.u_ofile[r] = NULL;
		iput(ip);
		return;
	}
	u.u_ar0[R1] = u.u_ar0[R0];
	u.u_ar0[R0] = r;
	wf->f_flag = FWRITE|FPIPE;
	wf->f_inode = ip;
	rf->f_flag = FREAD|FPIPE;
	rf->f_inode = ip;
	ip->i_count = 2;
	ip->i_flag = IACC|IUPD;
	ip->i_mode = IALLOC;
}

/*
 * Read call directed to a pipe.
 */
readp(fp)
int *fp;
{
	register *rp, *ip;

	rp = fp;
	ip = rp->f_inode;

loop:
	/*
	 * Very conservative locking.
	 */

	plock(ip);

	/*
	 * If nothing in the pipe wait.
	 */

	if( ip->i_size1 == 0 ) {			/* fix025 */
		/*
		 * If there are not both reader and
		 * writer active, return without
		 * satisfying read.
		 */
		prele(ip);
		if(ip->i_count < 2)
			return;
		ip->i_mode =| IREAD;
		sleep(ip+2, PPIPE);
		goto loop;
	}

	/*
	 * Read and return
	 */

	u.u_offset = rp->f_offset.loint.unsignd; /* fix000 */
	readi(ip);
	rp->f_offset.loint = u.u_offset.loint; /* fix000 */
	/*
	 * If reader has caught up with writer,			fix025
	 * reset offset and size to zero.			fix025
	 */
	if( rp->f_offset.loint == ip->i_size1 )			/* fix025 */
	{
		rp->f_offset.loint = 0;				/* fix025 */
		ip->i_size1 = 0;				/* fix025 */
		if( ip->i_mode & IWRITE )			/* fix025 */
		{
			ip->i_mode =& ~IWRITE;			/* fix025 */
			wakeup(ip+1);				/* fix025 */
		}
	}
	prele(ip);
}

/*
 * Write call directed to a pipe.
 */
writep(fp)
{
	register *rp, *ip, c;

	rp = fp;
	ip = rp->f_inode;
	c = u.u_count;

loop:

	/*
	 * If all done, return.
	 */

	plock(ip);
	if(c == 0) {
		prele(ip);
		u.u_count = 0;
		return;
	}

	/*
	 * If there are not both read and
	 * write sides of the pipe active,
	 * return error and signal too.
	 */

	if(ip->i_count < 2) {
		prele(ip);
		u.u_error = EPIPE;
		psignal(u.u_procp, SIGPIPE);
		return;
	}

	/*
	 * If the pipe is full,
	 * wait for reads to deplete
	 * and truncate it.
	 */

	if(ip->i_size1 >= PIPSIZ) {		/* fix025 */
		ip->i_mode =| IWRITE;
		prele(ip);
		sleep(ip+1, PPIPE);
		goto loop;
	}

	/*
	 * Write what is possible and
	 * loop back.
	 * If writing less than PIPSIZ, it always goes.		fix025
	 * One can therefore get a file > PIPSIZ if write	fix025
	 * sizes do not divide PIPSIZ.				fix025
	 */

	u.u_offset = ip->i_size1.unsignd; /* fix000 */
	u.u_count = min(c, PIPSIZ);	/* fix025 */
	c =- u.u_count;
	writei(ip);
	prele(ip);
	if(ip->i_mode&IREAD) {
		ip->i_mode =& ~IREAD;
		wakeup(ip+2);
	}
	goto loop;
}

/*
 * Lock a pipe.
 * If its already locked,
 * set the WANT bit and sleep.
 */
plock(ip)
int *ip;
{
	register *rp;

	rp = ip;
	while(rp->i_flag&ILOCK) {
		rp->i_flag =| IWANT;
		sleep(rp, PPIPE);
	}
	rp->i_flag =| ILOCK;
}

/*
 * Unlock a pipe.
 * If WANT bit is on,
 * wakeup.
 * This routine is also used
 * to unlock inodes in general.
 */
prele(ip)
int *ip;
{
	register *rp;

	rp = ip;
	rp->i_flag =& ~ILOCK;
	if(rp->i_flag&IWANT) {
		rp->i_flag =& ~IWANT;
		wakeup(rp);
	}
}
