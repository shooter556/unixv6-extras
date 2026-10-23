#
/*
 *	routines associated with wtmp records
 */

#include	"ac.h"


/*
 *	login
 *	=====
 */

login( up )
  register struct utmp *up;
{
	register struct u_ent	*ep;
	register struct tty	*ttyp;
	extern struct u_ent	*getuser();


  ep = getuser( up );

  if ( ep->ue_flags & LOGIN )
	ep->ue_multilogins++;
  else
	ep->ue_flags =| LOGIN;
  ep->ue_logins++;
  ep->ue_logons++;

  ttyp = gtty( up );
  if ( ttyp->t_userp )
  {
	if ( ttyp != BADTTY )
		warn( "tty multi-logon", up );
	logoff( ttyp, up );
  }
  ttyp->t_userp = ep;
  ttyp->t_logintime = up->u_logintime;
  ttyp->t_logons++;

  if ( flg.h )
  {
	hourp->c_logons++;
	ttycount( ttyp );
  }
}



/*
 *	logout
 *	======
 */

logout( op )
  register struct otmp *op;
{
	register struct u_ent	*ep;
	register struct tty	*ttyp;
	extern long		cputimes();

  ttyp = gtty( op );
  if ( !(ep = ttyp->t_userp) )
  {
	if ( ttyp != BADTTY && op->o_uid )
		warn( "unexpected logoff", op );
	ep = getuser( op );
  }
  else
  {
	logoff( ttyp, op );
	if ( ep->ue_uid != op->o_uid )
	{
		unsigned	uid = op->o_uid;

		op->o_uid = ep->ue_uid;
		warn( "wrong user logging off", op );
		op->o_uid = uid;
		ep = getuser( op );
	}
  }

  ep->ue_cputime =+ cputimes( op );
  ep->ue_flags =| UPDATE;
}



/*
 *	logoff	 log user off a particular tty
 *	======
 */

logoff( ttyp, op )
  register struct tty *ttyp;
  register struct otmp *op;
{
	register struct u_ent	*ep = ttyp->t_userp;
	long			logtime;

  logtime = op->o_logofftime - ttyp->t_logintime;
  ttyp->t_logtime =+ logtime;
  ttyp->t_userp = 0;

  if ( flg.h )
  {
	if ( ttyp->t_logintime < lasthour )
		hourp->c_logtime =+ op->o_logofftime - lasthour;
	else
		hourp->c_logtime =+ logtime;

	hourp->c_logoffs++;
	ttycount( ttyp );
  }

  if ( --ep->ue_logins <= 0 )
  {
	if ( ep->ue_logins < 0 )	/* program bug ? */
	{
		warn( "user multi-logoff - program bug?", op );
		ep->ue_logins = 0;
	}
	ep->ue_flags =& ~LOGIN;
  }
  ep->ue_logtime =+ logtime;
  ep->ue_flags =| UPDATE;
}



/*
 *	async
 *	=====
 */

async( wp )
  register struct wtmp *wp;
{
	register struct u_ent	*ep;
	extern struct u_ent	*getuser();
	long			cput;
	extern long		cputimes();

  ep = getuser( wp );
  cput = cputimes( wp );
  sys.st_async =+ cput;
  hourp->c_async =+ cput;
  ep->ue_asynctime =+ cput;
  ep->ue_cputime =+ cput;
  ep->ue_flags =| UPDATE;
}



/*
 *	update
 *	======
 */

update( dp )
  register struct dtmp *dp;
{
	register struct tty	*ttyp = ttys;
	long			change;

# ifdef	DEBUG
  if ( flg.y )
	warn( "update trace", dp );
# endif

  if ( !flg.z )
  {
	init( dp );
	return;
  }

  change = dp->d_newtime - dp->d_oldtime;

#ifdef	DEBUG
  if ( flg.y )
	syswarn( "update in: hour %s", ctime( hour ) );
#endif
  do
	if ( ttyp->t_userp )
		ttyp->t_logintime =+ change;
  while
	( ++ttyp < LAST_TTY );

# ifdef	DEBUG
  if ( flg.y )
  {
	char	s[PTIMSIZ];

	syswarn( "update: boottime %s\t%screased by %s\n"
		,ctime( sys.st_boottime )
		,change < 0 ? "de" : "in"
		,ptime( change < 0 ? -change : change, s )
	);
  }
# endif

  sys.st_boottime =+ change;

# ifdef	DEBUG
  if ( flg.y )
	syswarn( "update: boottime now %s", ctime( sys.st_boottime ) );
# endif

  if ( change < 0 )
  {
	do
		midnight =- DAY;
	while
		( midnight > dp->d_newtime );
	midnight =+ DAY;
	hour = midnight;

	do
		hour =- HOUR;
	while
		( hour > dp->d_newtime );

	if ( lasthour > hour )	/* hourly statistics ruined ! */
	{
		if ( flg.h )
			flg.h =| DATE_CHANGE;
		warn( "large negative date change", dp );
		nextmonth();
	}

	lasthour = hour;
	hour =+ HOUR;

	hourp = &hours[23 - (midnight - hour)/HOUR];
  }
  else
	if ( change >= DAY )
	{
		if ( flg.d || dp->d_newtime > month )
		{
			print( dp->d_oldtime );
			lastprint = dp->d_newtime;
		}
		if ( flg.h )
			flg.h =| DATE_CHANGE;
		fixuptime( dp );
	}
	else
		if ( hour <= dp->d_newtime )
			chime( dp, 1 );
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "update out: hour %s\n", ctime( hour ) );
#endif

  timenow = dp->d_newtime;
}



/*
 *	boot
 *	====
 */

boot( sp )
  register struct stmp *sp;
{
	register struct tty	*ttyp = ttys;
	register		users = 0;
	unsigned		uid;

# ifdef	DEBUG
  if ( flg.y )
	warn( "boot trace", sp );
# endif

  if ( !flg.z )
  {
	init( sp );
	return;
  }

  do
	if ( ttyp->t_userp )
	{
		uid = ttyp->t_userp->ue_uid;
		users++;
		logoff( ttyp, sp );
	}
  while
	( ++ttyp < LAST_TTY );

  if ( users > 1 || (users && uid) )
  {
	syswarn( "CRASH: %d user%slogged on when system crashed on %s"
		,users
		,users > 1 ? "s " : " "
		,ctime( sp->s_boottime )
	       );
	sys.st_crashes++;
  }
  else
	sys.st_boots++;

  sys.st_uptime =+ sp->s_boottime - sys.st_boottime;
  sys.st_boottime = sp->s_boottime;

  timenow = sp->s_boottime;
# ifdef	DEBUG
  if ( flg.y )
  {
	char	s[PTIMSIZ];

	syswarn( "boot: uptime %s\n\n", ptime( sys.st_uptime, s ) );
  }
# endif
}
