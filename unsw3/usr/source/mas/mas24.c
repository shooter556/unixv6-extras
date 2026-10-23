#


#define	r_start	0
#define	r_type	1
#define	r_count	2
#define	r_addr	4
#define	r_data	8

char	record[80];
char	*recp;
int	csum;
int	bytec;

extern	obfile;
extern	npc;





outbyte(b)
{
	register n;
	register char *r;

	if (bytec == 33)
	{
		outrec();
		newrec();
	}
	r = recp;
	n = b & 0377;
	csum =+ n;
	*r++ = bcd1(n);
	*r++ = bcd2(n);
	bytec++;
	recp = r;
	npc++;
}


outword(w)
{
	outbyte(w >> 8);
	outbyte(w);
}


newrec()
{
	register char *r;
	register n;

	r = record;
	*r++ = 'S';
	*r++ = '1';
	r = &record[r_addr];
	n = npc >> 8;
	n =& 0377;
	csum = n;
	*r++ = bcd1(n);
	*r++ = bcd2(n);
	n = npc & 0377;
	csum =+ n;
	*r++ = bcd1(n);
	*r++ = bcd2(n);
	recp = r;
	bytec = 3;
}




outrec()
{
	register n;
	register char *r;

	n = csum + bytec;
	n =~ n;
	n =& 0377;
	r = recp;
	*r++ = bcd1(n);
	*r++ = bcd2(n);
	*r = '\n';
	r = &record[r_count];
	n = bytec;
	*r++ = bcd1(n);
	*r++ = bcd2(n);
	n =<< 1;
	write(obfile, record, n+5);
}


  						/* mod000 */
/*
endrec()
{
	register n;
	register char *r;

	r = record;
	*r++ = 'S';
	*r++ = '9';
	*r = '\n';
	write(obfile, record, 3);
	close(obfile);
}
*/



bcd1(n)
{
	register j;

	j = n >> 4;
	return(j<10 ? j+'0' : j+'A'-10);
}


bcd2(n)
{
	register j;

	j = n & 017;
	return(j<10 ? j+'0' : j+'A'-10);
}
