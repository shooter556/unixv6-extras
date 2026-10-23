#
/*
 *	the print routines
 */

#include	<local-system>
#include	"ac.h"



/*
 *	print	the monolithic print routine - all data printed here are zeroed
 *	=====
 */

print( now )
  long now;
{
	char		sa1[PTIMSIZ], sa2[PTIMSIZ], sa3[PTIMSIZ], sa4[PTIMSIZ];
	extern char	*ptime();

# ifdef	DEBUG
  if ( flg.y )
	syswarn( "\nPRINT: %s\n", ctime( now ) );
# endif

  if ( now <= lastprint )
	return;

  /*
   *	form feed
   */

  putchar( flg.f ? '\014' : '\n' );

  /*
   *	time range
   */

  if ( (now - lastprint + DAY-1)/DAY > 1 )
  {
	pday( lastprint );
	printf( " - " );
  }
  pday( now-1 );
  putchar( '\n' );

  /*
   *	people
   */

  if ( flg.p )
  {
	register struct u_ent	*ep, **hp;
	struct u_ent		**people = ualloc( nu_ents * (sizeof *people) );
	int			persons = 0;
	unsigned		logons = 0;
	long			logtotal = 0;
	long			cputotal = 0;
	long			asynctotal = 0;

	printf( "\n  PEOPLE\nlogons  logged  average    cpu    %%cpu   async   name\n" );

	for ( hp = uhasht ; hp < &uhasht[HASHSIZ] ; hp++ )
		if ( ep = *hp )
			do
				if ( ep->ue_flags & (UPDATE|LOGIN) )
				{
					if ( ep->ue_flags & LOGIN )
					{
						register struct tty	*ttyp;

						for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
							if ( ep == ttyp->t_userp )
								ep->ue_logtime =+ now - max( lastprint, ttyp->t_logintime );
					}

					people[persons++] = ep;
				}
			while
				( ep = ep->ue_next );

	qsort( people, persons, (sizeof *people), porder );

	for ( hp = people ; hp < &people[persons] ; hp++ )
	{
		ep = *hp;
		printf( "%5l  %s  %s  %s %5.1f  %s  %s\n"
			,ep->ue_logons
			,ptime( ep->ue_logtime, sa1 )
			,ptime( ep->ue_logons ? ep->ue_logtime/ep->ue_logons : 0L, sa2 )
			,ptime( ep->ue_cputime/HZ, sa3 )
			,ep->ue_logtime ? (100. * ep->ue_cputime)/(ep->ue_logtime * HZ.) : 0.
			,ptime( ep->ue_asynctime/HZ, sa4 )
			,ep->ue_namep
		      );
		logons =+ ep->ue_logons;
		logtotal =+ ep->ue_logtime;
		cputotal =+ ep->ue_cputime;
		asynctotal =+ ep->ue_asynctime;
		ep->ue_logons = 0;
		ep->ue_logtime = 0;
		ep->ue_cputime = 0;
		ep->ue_asynctime = 0;
		ep->ue_flags =& ~UPDATE;
	}

	free( people );

	printf( "---------------------------------------------------------\n%5l  %s  %s  %s %5.1f  %s  %d\n"
		,logons
		,ptime( logtotal, sa1 )
		,ptime( logons ? logtotal/logons : 0L, sa2 )
		,ptime( cputotal/HZ, sa3 )
		,logtotal ? (100. * cputotal)/(logtotal * HZ.) : 0.
		,ptime( asynctotal/HZ, sa4 )
		,persons
	      );
  }

  /*
   *	ttys
   */

  if ( flg.t )
  {
	register struct tty	*ttyp;
	register struct u_ent	*ep;
	register unsigned	nttys = 0;
	unsigned		logons = 0;
	long			logtotal = 0;

	printf( "\n  TTYS\ntty logons  logged  average\n" );

	for ( ttyp = ttys ; ttyp < LAST_TTY ; ttyp++ )
		if ( (ep = ttyp->t_userp) || ttyp->t_logtime )
		{
			long	logtime = ttyp->t_logtime;

			if ( ep )
				logtime =+ now - max( lastprint, ttyp->t_logintime );

			nttys++;
			printf( " %c  %5l  %s  %s\n"
				,ttyp - ttys
				,ttyp->t_logons
				,ptime( logtime, sa1 )
				,ptime( ttyp->t_logons ? logtime/ttyp->t_logons : 0L, sa2 )
			      );
			logons =+ ttyp->t_logons;
			logtotal =+ logtime;
			ttyp->t_logons = 0;
			ttyp->t_logtime = 0;
		}

	printf( "---------------------------\n%2d  %5l  %s  %s\n"
		,nttys
		,logons
		,ptime( logtotal, sa1 )
		,ptime( logons ? logtotal/logons : 0L, sa2 )
	      );
  }

  /*
   *	hourly averages
   */

  if ( flg.h )
  {
	register		i;
	register struct clock	*cp;
	register unsigned	days = (now - lastprint + DAY-1)/DAY;
	long			logmax = 0;
	int			scale;

	if ( flg.h & DATE_CHANGE )
	{
		flg.h =& ~DATE_CHANGE;
		printf( "\n*** large date change during this period! ***\n" );
	}

	printf( "\n  HOURLY AVERAGES:%39ttty\ntime  : ttys logons logoffs  logged  hours    cpu     async   logged ...\n" );

	for ( cp = hours ; cp < &hours[24] ; cp++ )
		if ( cp->c_logtime > logmax )
			logmax = cp->c_logtime;

	scale = logmax/CHISTCOLS;

	for ( i = 0, cp = hours ; i < 24 ; i++, cp++ )
	{
		int	j;

		printf( "%2d-%-2d : %4.1f   %4.1f   %4.1f  %s  %4.1f   %s  %s  "
			,i ,i+1
			,(cp->c_ttys * 1.)/days
			,(cp->c_logons * 1.)/days
			,(cp->c_logoffs * 1.)/days
			,ptime( cp->c_logtime/days, sa1 )
			,((cp->c_logtime * 1.)/days)/HOUR
			,ptime( cp->c_cputime/(days * HZ), sa2 )
			,ptime( cp->c_async/(days * HZ), sa3 )
		      );

		for ( j = cp->c_logtime/scale + 1 ; --j ; )
			putchar( '-' );
		printf( "*\n" );

		cp->c_ttys = 0;
		cp->c_logons = 0;
		cp->c_logoffs = 0;
		cp->c_logtime = 0;
		cp->c_cputime = 0;
		cp->c_async = 0;
	}

	if ( days > 1 )
		printf( "\nNote: period covers %d days.\n", days );
  }

  /*
   *	system
   */

  if ( flg.s )
  {
	printf( "\n  SYSTEM\nAvailable: %s (%.1f%% uptime"
		,ptime( sys.st_uptime, sa1 )
		,(100. * sys.st_uptime)/(now - lastprint)
	      );
	if ( sys.st_boots || sys.st_crashes )
	{
		printf( " with " );
		if ( sys.st_boots )
		{
			printf( "%d reboo%s"
				,sys.st_boots
				,(sys.st_boots>1) ? "ts" : "t"
			      );
			sys.st_boots = 0;
			if ( sys.st_crashes )
				printf( " and " );
		}
		if ( sys.st_crashes )
		{
			printf( "%d cras%s"
				,sys.st_crashes
				,(sys.st_crashes > 1) ? "hes" : "h"
			      );
			sys.st_crashes = 0;
		}
	}
	printf( ").\n" );

	if ( !flg.o )
	{
		long	totcpu = sys.st_syscpu + sys.st_usercpu;

		printf( "\n  CPU USAGE\nCpu time: %s system, %s user, %.1f%% idle;\n"
			,ptime( sys.st_syscpu/HZ, sa1 )
			,ptime( sys.st_usercpu/HZ, sa2 )
			,sys.st_uptime ? 100. - (totcpu*100.)/(sys.st_uptime*HZ) : 0.
		      );
	
		printf( " of which %.1f%% was consumed by asynchronous processes,\n and %.1f%% was spent in the system.\n"
			,totcpu ? (100. * sys.st_async)/totcpu : 0.
			,totcpu ? (100. * sys.st_syscpu)/totcpu : 0.
		      );
	
		sys.st_syscpu = 0;
		sys.st_usercpu = 0;
		sys.st_async = 0;
	}

	sys.st_uptime = 0;

	{
		register struct tty	*ttyp = ttys;
		register		users = 0;

		do
			if ( ttyp->t_userp )
				users++;
		while
			( ++ttyp < LAST_TTY );

		if ( users )
			printf( "\nNote: %d user%scurrently logged on.\n"
				,users
				,(users > 1) ? "s " : " "
			      );
	}
  }

  lastprint = now;
}



/*
 *	pday	print day in form "mmm nn"
 *	====
 */

pday( now )
  long now;
{
	register char	*s;
	register	i;
	extern char	*ctime();

  s = ctime( now );
  for ( i = 4 ; i < 10 ; i++ )
	putchar( s[i] );
}




int	zero;

/*
 *	ptime	put time in string in form "nnnXnnY" where X&Y can be any of 'd', 'h', 'm', 's'
 *	=====
 */

char *ptime( time, sp )
  long time;
  char *sp;
{
	struct { int hiword; unsigned loword; };
	char	s[8];

  if ( time < 0 )
	goto negativ;

  {
	register	trac, sepc, divn;

	sepc = 'm';	trac = 's';
	divn = 60;

	if ( time > 18000 )	/* more than 300 mins. */
	{
		trac = sepc;
		sepc = 'h';
		time =/ 60;

#ifdef	TIME_IN_DAYS
		if ( time > 10080 )	/* more than 1 week */
		{
			trac = sepc;
			sepc = 'd';
			divn = 24;
			time =/ 60;

			if ( time > 23999 )	/* more than 1000 days */
				trac = 0;
		}
#else
		if ( time > 59999L )	/* more than 1000 hours */
			trac = 0;
#endif	TIME_IN_DAYS
	}

	zero = 1;

	if ( !trac )
	{
		time =/ divn;
		if ( time.hiword )
			goto overflo;
		ptn( time.loword, s, 6 );
		s[6] = sepc;
	}
	else
	{
		unsigned	t;

		ptn( (t=time/divn), s, 3 );
		s[3] = zero ? ' ' : sepc;
		ptn( (t=time%divn), &s[4], 2 );
		s[6] = zero ? '0' : trac;
	}
	s[7] = 0;
  }
  {
	register char	*cp1, *cp2;

	cp1 = s;
out:
	for ( cp2=sp ; *cp2++ = *cp1++ ; );
	return( sp );

negativ:
	cp1 = "negativ";
	goto out;
overflo:
	cp1 = "overflo";
	goto out;
  }
}



/*
 *	ptn	put a decimal number into string with leading spaces
 *	===
 */

char *ptn( t, s, n )
  register unsigned t,n;
  register char *s;
{
  if ( --n )
	s = ptn( t/10, s, n );
  if ( (t =% 10) || !zero )
  {
	zero = 0;
	*s++ = t + '0';
  }
  else
	*s++ = ' ';
  return( s );
}
