#
/*
 *	ac [ [[-]f...]... ] [wtmp]
 *
 *		where 'f' is any of 'dfhnopstwCLNSU'
 *		and "wtmp" is an alternate accounting file.
 *
 *	Piers Lauder	Mar '78
 */

#include	<local-system>

#ifdef	AUSAM

#include	"ac.h"		/* for definitions of flags, data and include files */

char	*wtmpf	"/usr/adm/wtmp";

int	wf;			/* file descriptor for wtmp file */

#ifdef	DIAGNOSTIC_REDIRECT
char	*diagf	"ac.diagnostics";
#endif



/*
 *	the main program - deal with parameters, set up files, call ac()
 */

main( argc, argv )
  register argc;
  register char **argv;
{
	register	c;
	extern		fout;

  fout = 2;			/* for errors */
  name = *argv++;		/* invoked name */

  while ( --argc )
  {
	while ( c = *(*argv)++ )
		switch ( c )
		{
		 case '-':	continue;

		 case 'd':	flg.d++;	continue;
		 case 'f':	flg.f++;	continue;
		 case 'h':	flg.h++;	continue;
		 case 'n':	flg.n++;	continue;
		 case 'o':	flg.o++;	continue;
		 case 'p':	flg.p++;	continue;
		 case 's':	flg.s++;	continue;
		 case 't':	flg.t++;	continue;
		 case 'w':	flg.w++;	continue;
#		ifdef	DIAGNOSTIC_REDIRECT
		 case 'x':	flg.x++;	continue;
#		endif
#		ifdef	DEBUG
		 case 'y':	flg.y++;	continue;
#		endif

		 case 'C':	porder = bycpu;		continue;
		 case 'L':	porder = bylog;		continue;
		 case 'N':	porder = byname;	continue;
		 case 'S':	porder = bylogons;	continue;
		 case 'U':	porder = byuid;		continue;

		 default:
			printf( "%s: bad flag '%c',\n", name, c );
			continue;
		}

	argv++;

	if ( flg.w )
		if ( --argc )
		{
			wtmpf = *argv++;
			flg.w = 0;
		}
		else
		{
			printf( "%s: no filename for 'w' flag.\n", name );
			return( -1 );
		}
  }

  if ( (wf = open( wtmpf, 0 )) < 0 )
  {
	printf( "%s: cannot open %s!\n", name, wtmpf );
	return( -1 );
  }

  fout = dup( 1 );

# ifdef	DIAGNOSTIC_REDIRECT
  if ( flg.x )
  {
	close( 2 );
	if ( open( diagf, 1 ) < 0 )
	{
		if ( creat( diagf, 0600 ) < 0 )
		{
			printf( "%s: cannot create %s!\n", name, diagf );
			return( -1 );
		}
	}
	else
		seek( 2, 0, 2 );
  }
# endif

  close( 0 );
  close( 1 );

  if ( !flg.p && !flg.t && !flg.h )
  {
	flg.s++;		/* default statistics */
#	ifdef	DIAGNOSTIC_REDIRECT
	if ( !flg.x )
#	endif
	flg.n++;		/* with no warnings */
  }

  if ( flg.p && !porder )
	porder = byname;	/* default people sort order */

  if ( flg.o )
	printf( "\nNote: cpu times not available in pre-AUSAM accounts.\n" );

  ac();

  flush();

  return( 0 );
}




struct	wtmp	wtmp[512/WTMPSIZ];

/*
 *	ac	main loop to read accounting file and call appropriate accounting routines
 *	==
 */

ac()
{
	register struct wtmp	*wp;
	register		n;
	extern			login(), logout(), async();

  while ( ((n = read( wf, (wp=wtmp), (sizeof wtmp) )) > 0) && (n =/ WTMPSIZ) )
  {
#	ifdef	WTMP_AUTOLOCK
	unlock();
#	endif	WTMP_AUTOLOCK

	do
	{
		switch ( wp->w_type )
		{
		 case U_TYPE:	time( login, wp );	break;
		 case O_TYPE:	time( logout, wp );	break;
		 case W_TYPE:	time( async, wp );	break;
		 case S_TYPE:	boot( wp );		break;
		 case D_TYPE:	update( wp );		break;
		 case SU_TYPE:	continue;

		 default:
			warn( "unrecognised type ignored", wp );
			continue;
		}
	}
	while
		( wp++ , --n );

#	ifdef	WTMP_AUTOLOCK
	readlock( wf );
#	endif	WTMP_AUTOLOCK
  }

# ifdef	WTMP_AUTOLOCK
  unlock();
# endif	WTMP_AUTOLOCK

  if ( flg.z )
	print( timenow );
}



/*
 *	time	initialises and keeps trace of time
 *	====
 */

time( f, wp )
  int			(*f)();
  register struct wtmp	*wp;
{
  if ( wp->w_finishtime >= hour )
	if ( flg.z )
		chime( wp, 0 );
	else
		init( wp );

  if ( wp->w_finishtime < timenow )
  {
	static long	sometime;

	if ( sometime != timenow )
	{
		warn( "date error", wp );
		sometime = timenow;
	}

	if ( wp->w_finishtime == 0 )
	{
		syswarn( " -- zero entry ignored\n" );
		return;
	}
  }
  else
	timenow = wp->w_finishtime;

  (*f)( wp );
}



/*
 *	init	initialise all times
 *	====
 */

init( dp )
  register struct dtmp *dp;
{
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "init at %s", ctime( dp->d_newtime ) );
#endif
  sys.st_boottime = dp->d_newtime;
  lastprint = dp->d_newtime;

  do
	year =+ YEAR;
  while
	( year <= dp->d_newtime );

  midnight = year - YEAR;
  fixuptime( dp );

  flg.z++;
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "init: hour is %s", ctime( hour ) );
#endif
}

#endif	AUSAM
