#include	<local-system>

#ifdef	AUSAM

/*
 *	who  [-c] [-d] [-s] [-f] [-l] [-o] [ wtmp ]|[ am i ]
 */

/*
 *	piers lauder	oct '77
 *
 *	use new utmp & wtmp formats
 *
 *	Piers Lauder	Feb '78
 *
 *	the AUSAM thing ...
 *
 *	also new flags:-
 *		-c	output 20 lines at a time
 *		-d	initial directory
 *		-s	initial shell
 *		-f	first name
 *		-l	last name
 *		-o	other ( may be many lines ! )
 */


#define	NLINES	20		/* screen size for 'c' flag */

#include	<utmp.h>
struct	utmp	buf[512/UTMPSIZ];
#include	<passwd.h>
#include	<gtty.h>
struct	sgttyb	tbuf;
int	savemode;

int	fout;
struct	flags	{	char  c, d, s, f, l, o, z;	}	flg;

struct	{	char *cp;	};


main(argc, argv)
  register int argc;
  char **argv;
{
	register	i;
	char		*s, *cbuf;
	int		n, fi;
	char		tty;
	register struct utmp	*p;
	int		c;
	char		pbuf[SSIZ];
	struct	pwent	pe;
	int		lines;
	extern		fix();

  while ( argc > 1 && argv[1][0] == '-' )  {
	while ( c = *(++argv[1]) )  switch ( c )  {
		case 'c':	flg.c++;	lines = 0;	break;
		case 'd':	flg.d++;	break;
		case 's':	flg.s++;	break;
		case 'f':	flg.f++;	break;
		case 'l':	flg.l++;	break;
		case 'o':	flg.o++;	break;
		default:	write( 2 , "bad flag " , 9 );
				write( 2 , &c , 1 );
				write( 2 , "\n" , 1 );
	}
	argc--;
	argv++;
  }

	s = "/etc/utmp";
	if(argc == 2)
		s = argv[1];
	fi = open(s, 0);
	if(fi < 0) {
		write(2 , "cannot open wtmp\n" , 17);
		exit( -1 );
	}

	fout = dup(1);
	close(1);

	if (argc==3)
		tty = ttyn(0);

	if ( flg.c )
	{
		gtty( 0, &tbuf );
		savemode = tbuf.mode;
		tbuf.mode =& ~ECHO;
		tbuf.mode =| RAW;
		stty( 0, &tbuf );
		signal( 2, fix );
		signal( 3, fix );
	}

loop:
	n = read(fi, buf, sizeof buf);
	if(n == 0) {
		flush();
		if (argc==3)
			prs( "Nobody.\n" );
		if ( flg.c )
			fix();
		exit( 0 );
	}

  for(p = &buf[0]; (n =- (sizeof buf[0]))>=0; p++) {
	if (argc==3 && tty!=p->u_ttyid)
		continue;
	if ( argc == 1 && p->u_type != U_TYPE )
		continue;

	switch ( p->u_type )  {
	 case U_TYPE:
		if ( (!(flg.d|flg.s|flg.f|flg.l|flg.o) || flg.z++) && p->u_u_name[0] )  {
			int f;

			for ( f = i = (sizeof p->u_u_name)+1 , cbuf = p->u_u_name ; --i ; )
				putchar( (((c = *cbuf++) || (f = 0)) && f) ? c : '  ' );
		}else  {
			pe.pw_uid = p->u_u_id;
			if ( getpwlog( &pe , pbuf , SSIZ ) > 0 )  {
				prs( pe.pw_strings[LNAME] );
				for ( i = 0 , cbuf = pe.pw_strings[LNAME] ; *cbuf++ ; i++ );
				if ( i < 8 )
					putchar( '\t' );
			}else  {
				prs( "????    " );
			}
		}
		putchar( '\t' );
ttyc:
		prs( "tty" );
		putchar(p->u_ttyid);
time:
		cbuf = ctime(p->u_logintime)+3;
		for(i=3; i<16; i++)
			putchar(*cbuf++);
		if ( flg.z )
		{
			if ( flg.f )  {  putchar( ' ' );  prs( pe.pw_strings[FIRSTNAME] );  }
			if ( flg.l )  {  putchar( ' ' );  prs( pe.pw_strings[LASTNAME] );  }
			if ( flg.d )  {  putchar( ' ' );  prs( pe.pw_strings[DIRPATH] );  }
			if ( flg.s )  {  putchar( ' ' );  prs( pe.pw_strings[SHELLPATH] );  }
			if ( flg.o )  
			{
				if ( flg.f | flg.l | flg.d | flg.s )
				{
					prs( "\n " );
					lines++;
				}
				else
					putchar( ' ' );
				for ( i.cp = pe.pw_strings[OTHER] ; *i.cp ; )
					if ( *i.cp++ == '\n' )
						lines++;
				prs( pe.pw_strings[OTHER] );
				putchar( '\n' );
				lines++;
			}
			flg.z = 0;
		}
		putchar('\n');

		if (argc==3) {
			flush();
			if ( flg.c )
				fix();
			exit( 0 );
		}
		if ( flg.c && (++lines >= NLINES) )  {
			flush();
			read( 0, &c, 1 );
			lines = 0;
		}
		continue;

	 case O_TYPE:
		if ( p->w_ttyid )  {
			prs( "log off " );
			goto ttyc;
		}
	 case W_TYPE:
		prs( "async proc   " );
		goto time;

	 case S_TYPE:
		prs( "system boot  " );
		goto time;

	 case D_TYPE:
		prs( "date change  from " );
		cbuf = ctime( p->d_oldtime ) + 3;
		for ( i = 3 ; i < 16 ; i++ )
			putchar( *cbuf++ );
		prs( " to " );
		goto time;

	 case SU_TYPE:
		prs( "SUPER-USER: " );
		goto time;

	 default:
		flush();
		write( 2 , "bad wtmp\n" , 9 );
	}
  }
  goto loop;
}


prs( s )
  register char *s;
{
  while ( *s )  putchar( *s++ );
}


fix()
{
	tbuf.mode = savemode;
	stty( 0, &tbuf );
	exit( 0 );
}

#endif
