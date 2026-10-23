#include	<local-system>

#ifdef	EECF
char	dname[]	"rms??";
#endif
#ifdef	AGSM
char	dname[]	"rhp?\0";
#endif

char	icheck[]	"/etc/icheck";
char	dcheck[]	"/etc/dcheck";

char	wbuf[2];

main(c,a)
char *a[];
{
	signal(2,1);
	if( c < 2 )
	{
		prints(2, "Usage: chk nn [nn ...]\n");
		return 1;
	}
	if( chdir("/dev") < 0 )
	{
		prints(2, "Cant chdir to dev\n");
		return 1;
	}
	while( c > 1 )
	{
		a++;
		c--;
		dname[3] = a[0][0];
		dname[4] = a[0][1];
		if( !fork() )
		{
			prints(1, "ICHECK ");
			signal(2,0);
			execl(icheck, icheck+5, dname, 0);
			prints(2, "Cant exec icheck\n");
			return 1;
		}
		waitx( wbuf );
		if( wbuf[0] && wbuf[0] != 2 )
			prints(2, "Non-zero status from icheck\n" );
		if( !fork() )
		{
			prints(1, "DCHECK ");
			signal(2,0);
			execl(dcheck, dcheck+5, dname, 0);
			prints(2, "Cant exec dcheck\n");
			return 1;
		}
		waitx( wbuf );
		if( wbuf[0] && wbuf[0] != 2 )
			prints(2, "Non-zero status from dcheck\n" );
	}
}
