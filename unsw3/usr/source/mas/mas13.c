#

#include	"mas0.h"

char	inbuf[];
char	*fins;			/* mod011 */
int	tbuf[5], errc, lc;		/* mod011 */
extern	struct aform aout;

extern	char symbuf[], tsymbuf[32];		/* mod011 */
extern	struct segment segopt;
extern	char getnonbl();
extern	char getch();
extern	pc, pc_def;
extern	struct evalx r;
extern	lab;
extern	char tch;



pseudop(opc)
{
	register n;
	register char c;
	register int *tp;
	int *ip;
	char *tcp;

	switch (opc)
	{
    case o_SEG:	if (segopt.segcount == 0) segerr();
		segopt.segcount++;
		return;

    case o_EQU:	if (! lab) syntax();
		getexpr();
		evalexpr();
		if (r.r_type != ABS)	/* und, exp or est */
			outaform(a_EQU+lab);
		equsym(lab);
		return;

    case o_ORG:	if (lab) syntax();
		getexpr();
		evalexpr();
		if (r.r_type == UND) relerr();
		pass1a();
		newif(i_REL);
		evalexpr();
		if (r.r_type != ABS) syserr();
		pc = r.r_val;
		pc_def = ABS;
		return;

    case o_FCB:	pc =+ getlis();
		return;

    case o_FDB:	pc =+ getlis() << 1;
		return;

    case o_FCC:	if ((c = getnonbl()) != SLASH) syntax();
		while ((c = getc(inbuf)) != SLASH)		/* mod010 */
		{
			if (c == BACKSL && nonspec(getc(inbuf))) pc++;	/* mod010 */
			if (c == NL) syntax();
			if (c == EOF) eoferr();
			pc++;
		}
		return;

    case o_RMB:
    case o_ZMB:	getexpr();
		evalexpr();
		if ((n = r.r_type) == UND) relerr();
		if (n != ABS)
		{
			pass1a();
			evalexpr();
		}
		if (r.r_type != ABS) syserr();
		if ((n = r.r_val) < 0) relerr();
		pc =+ n;
		return;

    case o_ICL:	if (*(tp = tbuf)) iclerr();		/* mod011 */
		c = getnonbl();
		tcp = tsymbuf;
		while (c != SP && c != TAB && c != NL)
		{
			*tcp++ = c;
			if (tcp >= &tsymbuf[sizeof tsymbuf]) iclerr();
			c = getc(inbuf);
		}
		*tcp = '\0';
		tch = c; getcomm();
		ip = inbuf;
		seek(*ip, &inbuf[6]-ip[1]-ip[2], 1);	/* beginning of current block */
		*tp++ = *ip++; *tp++ = *ip++; *tp++ = *ip++;
		*tp++ = lc; *tp++ = fins;
		if ((fopen(fins = tsymbuf, inbuf)) == -1) iclerr();
		lc = errc = 0;
		reset();
	}
}



nonspec(c)
register char c;
{
	if    (c == SLASH ||
		c == BACKSL ||
		c == 'n' ||
		c == 't' ||
		c == 's' ||
		c == '0' ||
		c == 'r' ||		/* mod004 */
		c == 'b')		/* mod004 */
			return(0);
	return(1);
}
