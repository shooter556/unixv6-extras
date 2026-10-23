#include	"odb7.h"
#define	ESC	033

rdline()
	{ lp=line;
	while ((*lp=getchar()) != EOT   &&
		!match(*lp,"/\\\r\n^<%=>;:*") &&
		*lp != ESC &&
		!(match(*lp,"bptg") && scflg) )
			testch();
	lp = line;
	}

testch()
	{
	switch(*lp)
		{
		default :
			if ( ++lp > line+128 )
					error();
			return;
		case '#':
			if (lp>line) --lp;
			return;
		case RUB :
			printf("####  ");
			lp=line;
			}
}


match(c,st)
char *c,*st;
	{
	register char *s;
	s = st;
	while ( *s )
		if ( *s++ == c )
			return(1);
	return(0);
}

rch()
	{ return((ch = *lp++)); }

prcont(v)
	{
	register val;
	register char *c;
	val = v;
	switch(cmode)
		{
		case 'i' :
			putins(val, 0);
			break;
		case 'o' :
			putoct(val);
			break;
		case 'd' :
			if (val<0)
				{
				putlp("-");
				val = -val;
				if ( val<0 )
					{
					putlp("32768 ");
					return;
					}
				}
			putdec(val);
			break;
		case 'c' :
			c = &v;
			*lbp++ = *c++;
			if (bytwrd!=1)
				*lbp++ = *c;
		}
	putlp(" ");
}

symbolic()
	{
	 register d,*sptr;
	sptr = &symbol[10];
	putlp(symbol);
	if ( (d= *sptr-dot) == 0 )
			return;
	if ( d>0 )
		putlp("-");
	else
		{
		putlp("+");
		d = -d;
		}
	putdec(d);
	}

getnfil()
	{
	write(1,"    >",5);
	getlin(0);
	if (line[0] == '\n')
			return;
	*lp = '\0';
	if ( (cfdes = creat(line,0666)) == -1 )
			{
			write(1, "\n?", 2);
			error();
	}
}

getlin(k)
	{
	lp = line;
	setty(1);
	while ((*lp=getchar()) != '\n')
			if (*lp++ == '\0')
				{
				if (pid)
					ptrace(8, pid, 0, 0);
				exit();
			}
	if (k)
		lp = line;
}

putlp(st)
char *st;
	{
	register char *s;
	s = st;
	while (*s)
		*lbp++ = *s++;
}


putdec(n)
	{
	int d;
	if ( d = n/10 )
		putdec(d);
	*lbp++ = (n%10) + '0';
}


putoct(o)
	{
	register char *c;
	c = lbp;
	poct(o);
	if ( o != 0 )
		{
		while (c++ < lbp)
			*(c-1) = *c;
		lbp--;
		}
}

poct(o)
	{
	if (o)
		poct((o>>3) & 017777);
	*lbp++ = (o&07) + '0';
}


linput()
	{
	if ( lbp>linbuf )
		write(cfdes,linbuf,lbp-linbuf);
	lbp = linbuf;
}
