#define QESCP 0177676
#define QEOR  0177727
#define QEOI  0177726

struct { int integ;};
char card[81];
int f;

main(c,v)
char *v[];
{
	register n,m;
	register char *c;

	char buf[1024];

	card[80] = '\n';
	if( (c<2) || ( (f = open(v[1], 0)) < 0) )
	{
		write(2, "File open error\n", 16);
		exit();
	}
	while( read(f, buf, 1024) > 0)
	{
		n = (buf->integ) / 80;
		c = &buf[2];
		do
		{
			m = 0;
			if( *c == QESCP)
			{
				switch(*++c)
				{
			    case QEOR:	card[m++] = (*++c == '*') ? 006 : 022;
					while( m < 80)
						card[m++] = ' ';
					c =+ 79;
					break;
			    case QEOI:	continue;
				}
			}
			else do
				card[m++] = *c++;
			while(m < 80);
			write(1, card, 81);
		}
		while( --n);
	}
}
