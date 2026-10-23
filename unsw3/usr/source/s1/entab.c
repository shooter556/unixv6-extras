#
#define	EOF	-1
#define	EOLN	'\n'
#define	CR	'\r'
#define	FF	'\f'
#define	TAB	'\t'
#define	BS	'\b'

char	ibuf[518],obuf[518];

main(ac, v)
char *v[];
{

	register c,icol,ocol;

	if( ac > 3 )
	{
		prints( 2, "Usage: entab [ infile [ outfile ] ]\n");
		return(1);
	}
	if(ac > 1)
	{
		if( fopen( v[1], ibuf) < 0)
		{
			prints( 2, v[1]);
			prints( 2, ": cannot open\n");
			return(1);
		}
	}
	if(ac > 2)
	{
		if( fcreat( v[2], obuf) < 0)
		{
			prints(2, v[2]);
			prints(2, ": cannot create\n");
			return(1);
		}
	}
	else
		obuf[0] = 1;
	/* here beginneth the program */

	icol = ocol = 0;
	while( (c = getc(ibuf)) != EOF )
	{
		switch(c)
		{
	    case EOLN:
	    case CR:
	    case FF:
			putc(c, obuf);
			icol = ocol = 0;
			break;
	    case ' ':
			icol++;
			break;
	    case TAB:
			icol = (icol + 8) & ~07;
			break;
	    default:
			while( ((ocol + 8) & ~07) <= icol && ocol+1 != icol)
			{
				putc( TAB, obuf);
				ocol = (ocol + 8) & ~07;
			}
			while( ocol < icol)
			{
				putc( ' ', obuf);
				ocol++;
			}
			putc( c, obuf);
			if( c == BS)
			{
				icol--;
				ocol--;
			}
			else
			{
				icol++;
				ocol++;
			}
		}
	}
	fflush(obuf);
	return(0);
}
