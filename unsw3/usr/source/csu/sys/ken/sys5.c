#
/*
 * More system calls
 */

#include "../param.h"
#include "../user.h"
#include "../reg.h"
#include "../proc.h"
#include "../text.h"
#include "../inode.h"

enqueue()
{
	register int	x,n;

	spl6();
	n = u.u_ar0[R0];
	while ((x = fuword(n)) <= 0)
		sleep(n | 1, PPV);
	suword(n, --x);
	spl0();
}

dequeue()
{
	register int	x,n;

	spl6();
	n = u.u_ar0[R0];
	x = fuword(n) + 1;
	suword(n, x);
	if (x == 1)
		wakeup(n | 1);
	spl0();
}

/*
 *	Return with PS set to supplied value.
 *	Old PS is pushed on stack.
 *	Only the super user may set certain bits.
 */
setpsw()
{
	register m, s;

	m = (u.u_uid != 0) ? 0177740 : 0140340;
	s = u.u_ar0[RPS];
	u.u_ar0[RPS] = (s & m) | (u.u_arg[0] & ~m);
	grow(u.u_ar0[R6] =- 2);
	suword(u.u_ar0[R6], s);
}

/*
 * Set CPU time limit
 */
tlimit()
{
	register t;

	spl6();
	u.u_tix = HZ;
	t = u.u_cpusec;
	u.u_cpusec = u.u_ar0[R0];
	spl0();
	u.u_ar0[R0] = t;	/* return remaining time */
}

/*
 * Real-time limit
 */
clktim()
{
	register unsigned x;

	spl1();
	x = u.u_procp->p_rtl;
	u.u_procp->p_rtl = u.u_ar0[R0];
	spl0();
	u.u_ar0[R0] = x;
}

struct	tabdescr {
	char	*tb_base;		/* base of region */
	unsigned	tb_size;	/* size of region */
	unsigned	tb_items;	/* no. of elements in region */
	} tabdescr[] {
		{	proc,	sizeof proc,	NPROC	},
		{	text,	sizeof text,	NTEXT	},
		{	inode,	sizeof inode,	NINODE	},
	};

/*
 * Copy various system tables into user buffers
 */
gettable()
{
	register struct	tabdescr	*tbp;
	register char	*p;

	if (u.u_ar0[R0] >= (sizeof tabdescr / sizeof tabdescr[0]))
		goto bad;
	tbp = &tabdescr[u.u_ar0[R0]];
	p = u.u_arg[0];
	if (p != NULL) {
		if (p&01 || p>=(p + u.u_arg[1]) || u.u_arg[1]<tbp->tb_size)
			goto bad;
		if (copyout(tbp->tb_base, p, tbp->tb_size) < 0)
			goto bad;
	}
	u.u_ar0[R0] = tbp->tb_items;
	u.u_ar0[R1] = tbp->tb_base;
	return;

bad:
	u.u_error = EFAULT;
}
