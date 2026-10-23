#define	EVENP	000200
#define	XTABS	000002
#define	B2400	((11 << 8) + 11)

char tty[]	"/dev/tty?";
char printers[]	"76";
char defaultp[]	"7";
char	*ps;
int ttybuf[3]	{ B2400, '#@', EVENP+XTABS };
char prs[]	"Using printer ?\n";
char pu[]	"Printer  busy\n";

main(c, v)
register char *v[];
{
	register char *x,*y;
	int retryflag = 0;
	extern null();

	close(1);
	ps = defaultp;
	if(v[1][0] == '-')
		{
		x = v[1];
		x++;
		if( x[0] == 'w' )
			{
			retryflag++;
			x++;
			}
		if(x[0] != 0)
			ps = x;
		v++;
		c--;
		}
openit:
	x = ps;
	while( *x ) {
		if(notin(*x, printers))
			*x = defaultp[0];
		tty[8] = *x++;
		if( open(tty, 1) == 1) goto opened;
	}
	if( retryflag ) { sleep(10); goto openit; }
	if( ps[1] ) pu[7] = 's';
	prints(2, pu);
	exit(1);
opened:
	prs[14] = *--x;
	prints(2, prs);
	stty(1, ttybuf);
	v[c] = 0;
	v[0][0] = *x;
	v[0][1] = '\0';
	execv("/usr/bin/speed", v);
	prints(2, "Cannot exec speed\n");
}


notin(c, s)
char	c, *s;
{
	while(*s) if(*s++ == c) return(0);
	return(1);
}
