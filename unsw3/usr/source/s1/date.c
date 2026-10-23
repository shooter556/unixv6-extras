#
/*
 *	date [[[[[yy]mm]dd]hh]mm]
 *
 *	Piers Lauder	Mar '78
 */

#include	<local-system>

#ifdef	AUSAM

long	time();
long	timbuf;
int	dmsize[];
char	*ijtime();
char	*cbp;

#include	<wtmp.h>
struct	dtmp	dtmp;



main(argc, argv)
  int argc;
  char **argv;
{
	register int wf;

	if(argc > 1) {
		cbp = argv[1];
		timbuf = time();
		dtmp.d_oldtime = timbuf;
		dtmp.d_type = D_TYPE;

		if(gtime()) {
			write(1, "bad conversion\n", 15);
			exit(-1);
		}

		if(stime(timbuf) < 0) {
			write(1, "no permission\n", 14);
		} else if ((wf = open("/usr/adm/wtmp", 1)) >= 0) {
			dtmp.d_newtime = time();
			seek(wf, 0, 2);
			write(wf, &dtmp, sizeof dtmp);
		}
	}

	cbp = ijtime();
	write(1, cbp, 20);
	write(1, "Sydney Time", 11);
	write(1, cbp+19, 6);
}

gtime()
{
	register int i;
	register int y;
	register int *tvec;
	int t, d, h, m;
	extern int *localtime();

	tvec = localtime( timbuf );

	if ( (y = gpair()) < 0 )	goto bad;
	if ( (t = gpair()) < 0 )	goto setmin;
	if ( (d = gpair()) < 0 )	goto sethrs;
	if ( (h = gpair()) < 0 )	goto setday;
	if ( (m = gpair()) < 0 )	goto setmon;

setime:
	if ( t<1 || t>12 )	goto bad;
	if ( d<1 || d>31 )	goto bad;
	if ( h>=24 )  {
		if ( h>24 )	goto bad;
		h = 0;
		if ( ++d>31 )  {
			d = 1;
			if ( ++t>12 )  {
				t = 1;
				y++;
			}
		}
	}
	if ( m>59 )		goto bad;

	timbuf = 0;
	y =+ 1900;
	for(i=1970; i<y; i++)
		timbuf =+ dysize(i);
	/* Leap year */
	if (dysize(y)==366 && t >= 3)
		timbuf++;
	while(--t)
		timbuf =+ dmsize[t-1];
	timbuf =+ d-1;
	timbuf = timbuf*24 + h;
	timbuf = timbuf*60 + m;
	timbuf =* 60;
	return(0);

setmin:	m = y;
	h = tvec[2];
getday:	d = tvec[3];
getmon:	t = tvec[4]+1;
getyrs:	y = tvec[5];
	goto setime;

sethrs:	m = t;
	h = y;
	goto getday;

setday:	m = d;
	h = t;
	d = y;
	goto getmon;

setmon:	m = h;
	h = d;
	d = t;
	t = y;
	goto getyrs;

bad:
	return(1);
}

gpair()
{
	register int c, d;
	register char *cp;

	cp = cbp;
	if(*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c<0 || c>100)
		return(-1);
	if(*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	cbp = cp;
	return (c+d);
}

#endif
