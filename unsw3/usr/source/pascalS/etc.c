#include "nglob.h"
#include "iglob.h"
printd(dp, wp)
int dp, wp;
{
/*
 *	prints the decimal number 'dp' in field width 'wp'.
 *	if it won't fit, it is allowed extra space.
 */
	register d, w;
	register char *s;

	d = dp; w = wp;
	s = &string[21]; *s-- = '\0';
	if(d < 0)
		d = -d;
	do {
		*s-- = (d%10) + '0'; w--;
	} while(d =/ 10);
	if(dp < 0) {
		*s-- = '-'; w--;
	}
	while(w-- > 0)
		*s-- = ' ';
	while(*++s)
		putchar(*s);
}
printl(dp, wp)
long	dp;
int	wp;
{
/*
 *	prints the decimal number 'dp' in field width 'wp'.
 *	if it won't fit, it is allowed extra space.
 */
	register w;
	register char *s;
	long	d;

	d = dp; w = wp;
	s = &string[21]; *s-- = '\0';
	if(d < 0)
		d = -d;
	do {
		*s-- = (d%10) + '0'; w--;
	} while(d =/ 10);
	if(dp < 0) {
		*s-- = '-'; w--;
	}
	while(w-- > 0)
		*s-- = ' ';
	while(*++s)
		putchar(*s);
}
long labs(lint)
long	lint;
{
	return(lint<0 ? -lint : lint);
}
prints(sp, wp)
char *sp;
int wp;
{
/*
 *	prints the string pointed to by 'sp' in width 'wp'.
 *	if it won't fit, it is allowed extra space.
 */
	register n;
	register char *s;

	s = sp; n = 0;
	while(*s++)
		n++;
	n = wp - n;
	while(n-- > 0)
		putchar(' ');
	s = sp;
	while(*s)
		putchar(*s++);
}
printtab()
{
/*
 *	dumps the internal tables of the compiler to enable
 *	debugging. the best aid available.
 *	use it!!!! call your test routine 'test0'.
 */

	register struct tab *p;
	register struct btab *bp;
	register struct atab *ap;
	int i;
	struct order *o;

	prints("\n identifiers   link  obj",0);
	prints("  typ  ref  nrm  lev  adr\n",0);
	for(i=btab[1].b_last+1;i<=t;i++) {
		p = &tab[i];
		printd(i,2);
		prints(p->t_name,11);
		printd(p->t_link,5);
		printd(p->t_obj,5);
		printd(p->t_typ,5);
		printd(p->t_ref,5);
		printd(p->t_nrm,5);
		printd(p->t_lev,5);
		printd(p->t_adr,5);
		putchar(NL);
	}
	prints("\n blocks  last  lastpar psize vsize\n",0);
	for(i=1;i<=b;i++) {
		bp = &btab[i];
		printd(i,5);
		printd(bp->b_last,7);
		printd(bp->b_lastpar,7);
		printd(bp->b_psize,7);
		printd(bp->b_vsize,7);
		putchar(NL);
	}
	prints("\n arrays xtyp etyp eref",0);
	prints("  low high elsz size\n",0);
	for(i=1;i<=a;i++) {
		ap = &atab[i];
		printd(i,5);
		printd(ap->a_inxtyp,6);
		printd(ap->a_eltyp,5);
		printd(ap->a_elref,5);
		printd(ap->a_low,5);
		printd(ap->a_high,5);
		printd(ap->a_elsize,5);
		printd(ap->a_size,5);
		putchar(NL);
	}
	prints("\n code :\n",0);
	for(i=0;i<=(lc-1);i++) {
		if(i%5 == 0)  {
			putchar(NL); printd(i,5);
		}
		o = &code[i];
		printd(o->o_f,5);
		if(o->o_f < 31)
			if(o->o_f < 4) {
				printd(o->o_x,2);
				printd(o->o_y,5);
			} else
				printd(o->o_y,7);
		else
			prints("       ",0);
		putchar(',');
	}
	putchar(NL);
}
pwrite(wtyp,witem,wd,dp,wrmode)
int wtyp;
struct stack_r *witem;
int wd, dp;
char wrmode;
{
	register int xwd, xdp;
	register struct stack_r *xwitem;

	xwitem = witem;
	xwd = wd;

	switch(wtyp) {
	case BOOLS:	prints(xwitem->s_b ? "true" : "false", xwd);
			break;
	case CHARS:	if(xwd > 1)
				prints(" ",xwd - 1);
			putchar(xwitem->s_i.loword);
			break;
	case INTS:	printl(xwitem->s_i, xwd);
			break;
	case REALS:	xdp = dp;
			if(wrmode == 'e') { /* b+d.ddde+dd */
				xdp = xwd -  7;
				if(xdp < 2)
					xdp = 2;
			}
			ftoa(xwitem->s_r, string, xdp, wrmode);
			prints(string, xwd);
			break;
	}
}
