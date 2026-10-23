#

/*
 * actually send the file down the line
 */

#include "gtl.h"

char	init6[]	{ '\175', 'R', '\175', 'L' };
char	init8[]	{ 0014, 0033, 0033 };

char	sixmode;		/* output in 6-bit mode */

/*
 * dumpit: sends the program to the terminal.  handles the different
 * formats available.  if neccessary, sends the 8-bit loader and odt.
 */
dumpit()
{
	register char	*p;
	register int	i;
	register struct seg	*mp;

	extern	ld8, ld8l, ld8adr;
	extern	ld8adx;

	stty(fileno(stdout), tty6);

	if (load6 || load7) {
		/* use the builtin 6-bit loader */
		sixmode++;
		for (p = &init6[0]; p < &init6[sizeof init6];)
			putchar(*p++);

		/* fill to allow display file to exit. */
		for (i = 0; i < 3; i++)
			putbyte(0);

		if (load7) {

			/* must load in the 8-bit loader */
			send(ld8adr, &ld8, &ld8l);
			send(ld8adr, 0, 0);
			putbyte(0);	/* empty out half byte */
		}
	}

	if (load8) {

		/* must initialize 8-bit load */
		for (p = &init8[0]; p < &init8[sizeof init8];)
			putchar(*p++);

		/* fill to allow display file to exit */
		for (i = 0; i < 3; i++)
			putchar(0);
	}
	if (load7 || load8) {
		fflush(stdout);
		stty(fileno(stdout), tty8);
		sixmode = 0;
	}

	for (mp = &seg[0]; mp<&seg[SEGS] && mp->m_cnt!=0; mp++)
		send(mp->m_adr, &mem[mp->m_adr], mp->m_cnt);

	send(startadr, 0, 0);

	if (load6)
		/* still in 6-bit mode; empty buffer char. */
		putbyte(0);
	fflush(stdout);
}

send(adr, p, sz)
char	*adr;
register char	*p;
register int	sz;
{
	register int	i;

	checksum = 0;

	/* fill between records */
	for (i = 0; i < 3; i++)
		putbyte(0);
	putbyte(1);
	putbyte(0);
	putword(sz+6);
	putword(adr);

	for (i = 0; i < sz; i++)
		putbyte(*p++);

	putbyte(-checksum);
}

putword(val)
int	val;
{
	putbyte(val);
	putbyte(val>>8);
}

putbyte(val)
register int	val;
{
	if (sixmode)
		sixbit(val);
	else
		putchar(val);
	checksum =+ val;
}

sixbit(byte)
register int	byte;
{
	register int	buf;
	static int	state;
	static int	last;

	switch (++state) {

	case 1:
		buf = (byte >> 2) & 077;
		last = byte;
		break;

	case 2:
		buf = ((last << 4) & 060) | ((byte >> 4) & 017);
		last = byte;
		break;

	case 3:
		state = 0;
		buf = ((last << 2) & 074) | ((byte >> 6) & 003);
		if (!(buf & 040))
			buf =| 0100;
		putchar(buf);
		buf = byte & 077;
		break;
	}
	if (!(buf & 040))
		buf =| 0100;
	putchar(buf);
}
