#
/*
 *	various subroutines in alphabetical order
 */

#include	"ac.h"

#include	<passwd.h>



/*
 *	bycpu		return reverse order of two user cpu times
 *	=====
 */

int bycpu( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_cputime < (*p2)->ue_cputime )
	return( 1 );
  return( -1 );
}



/*
 *	bylog		return reverse order of two user logged times
 *	=====
 */

int bylog( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_logtime < (*p2)->ue_logtime )
	return( 1 );
  return( -1 );
}



/*
 *	bylogons	return reverse order of two user logon counts
 *	========
 */

int bylogons( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_logons < (*p2)->ue_logons )
	return( 1 );
  return( -1 );
}



/*
 *	byname		return order of two user names
 *	======
 */

int byname( p1, p2 )
  struct u_ent **p1, **p2;
{
	register char	*s1 = (*p1)->ue_namep;
	register char	*s2 = (*p2)->ue_namep;
	register	c;

  while ( c = *s2++ )
	if ( c != *s1++ )
		if ( (c|040) > (*--s1|040) )
			return( -1 );	/* 1 < 2 */
		else
			return( c );	/* 1 > 2 */

  return( *s1 );	/* 1 >= 2 */
}



/*
 *	byuid		return order of two user ids
 *	=====
 */

int byuid( p1, p2 )
  struct u_ent **p1, **p2;
{
  if ( (*p1)->ue_uid > (*p2)->ue_uid )
	return( 1 );
  return( -1 );
}



/*
 *	chime	keep trace of time changes - hour-day-month
 *	=====
 */

chime( dp, date_change )
  register struct dtmp *dp;
{
  do
  {
	if ( flg.h )
	{
		register struct tty *ttyp = ttys;

		do
		{
			ttyp->t_flags =& ~NEWHR;
			if ( !date_change && ttyp->t_userp )
				if ( ttyp->t_logintime < lasthour )
				{
					hourp->c_ttys++;
					hourp->c_logtime =+ HOUR;
				}
				else
					hourp->c_logtime =+ hour - ttyp->t_logintime;
		}
		while
			( ++ttyp < LAST_TTY );
	}

	if ( !date_change )	/* && sys.st_boottime >= lasthour && sys.st_boottime < hour ) */
	{
		sys.st_uptime =+ hour - sys.st_boottime;
		sys.st_boottime = hour;
#		ifdef	DEBUG
		if ( flg.y )
		{
			char	s[PTIMSIZ];

			syswarn( "chime: uptime %s\n", ptime( sys.st_uptime, s ) );
		}
#		endif
	}

	lasthour = hour;
	hour =+ HOUR;
	if ( hour > midnight )
	{
		if ( flg.d )
			print( midnight );
		if ( hour > month )
		{
			if ( !flg.d )
				print( month );
			nextmonth();
#ifdef	DEBUG
			if ( flg.y )
				syswarn( "chime - midnight: %s", ctime( midnight ) );
#endif
		}
		midnight =+ DAY;
		hourp = hours;
	}
	else
		hourp++;
  }
  while
	( hour <= dp->d_newtime );
}



/*
 *	copystring	allocate new string area and copy string into it
 *	==========
 */

char *copystring( string )
  char *string;
{
	register char	*s, *ns;
	register	i;
	char		*new;
	extern char	*ualloc();

  for ( i = 1 , s = string ; *s++ && (!flg.o || i <= 8) ; i++ );

  new = ns = ualloc( i );

  for ( s = string ; i-- ; )
	*ns++ = *s++;
  if ( flg.o )
	*--ns = 0;

  return( new );
}



/*
 *	cputimes	update system cputimes and return total
 *	========
 */

long cputimes( op )
  register struct otmp *op;
{
	long	t;

  if ( !flg.o )
  {
	sys.st_syscpu =+ op->o_systime;
	sys.st_usercpu =+ op->o_usertime;
	t = op->o_systime + op->o_usertime;
	if ( flg.h )
		hourp->c_cputime =+ t;
	return( t );
  }
  else
	return( 0 );
}



/*
 *	fixuptime	bring time variables up to date
 *	=========
 */

fixuptime( dp )
  register struct dtmp *dp;
{
  do
	midnight =+ DAY;
  while
	( midnight <= dp->d_newtime );

  hour = midnight - DAY;
  lasthour = hour - HOUR;
  hourp = &hours[-1];

  chime( dp, 1 );
  nextmonth();
}



/*
 *	getuser		if user can't be found in hash table,
 *	=======		allocate a new user structure, and enter it in the table
 */

struct u_ent *getuser( up )
  register struct utmp *up;
{
	register struct u_ent	*ep;
	register		hashval = uhash( up->u_u_id );
	struct pwent		pen;
	char			sbuf[SSIZ];
	extern char		*ualloc(), *copystring();

  if ( ep = uhasht[hashval] )
  do
	if ( up->u_u_id == ep->ue_uid )
		goto found;
  while
	( ep = ep->ue_next );

  nu_ents++;
  ep = ualloc( sizeof *ep );

  /*
   *	initialise structure elements in order of declaration
   */

  ep->ue_next = uhasht[hashval];
  uhasht[hashval] = ep;

  pen.pw_uid = ep->ue_uid = up->u_u_id;
  ep->ue_flags = 0;
  ep->ue_logins = 0;
  ep->ue_multilogins = 0;;
  ep->ue_logons = 0;

  if ( ( !flg.o && getpwlog( &pen, sbuf, SSIZ ) < 0 )
      || ( flg.o && ( !up->u_u_name[0] || up->u_type != U_TYPE ) )
     )
  {
	warn( "unrecognised user", up );
	ep->ue_namep = copystring( "????????" );
  }
  else
	ep->ue_namep = copystring( flg.o ? up->u_u_name : pen.pw_strings[LNAME] );

  ep->ue_asynctime = 0;
  ep->ue_cputime = 0;
  ep->ue_logtime = 0;

found:
  return( ep );
}



/*
 *	nextmonth	calculate end of current month
 *	=========
 */

nextmonth()
{
	register	i,j;
	register	day = hour/DAY;
	int		f = 0;
	int		mon = 0;
	extern int	dysize(), dmsize[];

#ifdef	DEBUG
  if ( flg.y )
	syswarn( "nextmonth in hour: %s", ctime( hour ) );
#endif
  for ( i = 1970 ; day >= 0 ; i++ )
  {
	j = dysize( i );
	day =- j;	mon =+ j;
  }
  day =+ j;	mon =- j;

  if ( j == 366 )	/* leap year */
	f++;

  for ( i = 0 ; day >= 0 ; i++ )
  {
	j = dmsize[i];
	if ( f && i == 1 )	/* february */
		j++;
	day =- j;	mon =+ j;
  }

  month = mon*DAY;
#ifdef	DEBUG
  if ( flg.y )
	syswarn( "nextmonth out month: %s", ctime( month ) );
#endif
}



/*
 *	syswarn		general warning on file 2
 *	=======
 */

syswarn( s, a1, a2, a3, a4, a5 )
{
	register	of;
	extern		fout;

  if ( !flg.n )
  {
	flush();
	of = fout;
	fout = 2;
	printf( s, a1, a2, a3, a4, a5 );
	flush();
	fout = of;
  }
}



/*
 *	ttycount	count a tty if first time it has been used in current hour
 *	========
 */

ttycount( ttyp )
  register struct tty *ttyp;
{
  if ( !(ttyp->t_flags & NEWHR) )
  {
	hourp->c_ttys++;
	ttyp->t_flags =| NEWHR;
  }
}



/*
 *	ualloc		allocate new area, if none left, attempt garbage collection from hash table
 *	======
 */

char *ualloc( n )
{
	char			*s;
	extern char		*alloc();

loop:
  if ( (s = alloc(n)) == -1 )
  {
#	ifdef	GARBAGE_COLLECTION
	register struct u_ent	*ep, **bp, **hp;

	for ( hp = &uhasht[HASHSIZ-1] ; hp >= uhasht ; --hp )
		if ( ep = *hp )
		{
			bp = hp;
			do
				if ( (ep->ue_flags & (LOGIN|UPDATE)) == 0 )
				{
					nu_ents--;
					*bp = ep->ue_next;
					free( ep->ue_namep );
					free( ep );
					goto loop;
				}
			while
				( bp = &ep->ue_next , ep = ep->ue_next );
		}
#	endif	GARBAGE_COLLECTION

	flg.n = 0;
	syswarn( "%s: not enough memory!\n", name );
	exit( -1 );
  }

  return( s );
}



/*
 *	warn		accounting record related warning
 *	====
 */

warn( s, up )
  char *s;
  register struct utmp *up;
{
	register	c;
	register	type;
	extern char	*ctime();

  if ( flg.o && up->u_u_id == 0xffff )
	return;

  switch ( type = up->u_type )
  {
   case U_TYPE:	type = 'U';	break;
   case O_TYPE:	type = 'O';	break;
   case W_TYPE:	type = 'W';	break;
   case S_TYPE:	type = 'S';	break;
   case D_TYPE:	type = 'D';	break;

   default:	switch ( type =+ ' ' )
		{
		 case 'U': case 'O': case 'W': case 'S': case 'D':
			type =| 040;
		}
  }

  syswarn( "Warning: %s;\n\ttype:'%c' tty:'%c' uid:%-5l  %s"
	   ,s
	   ,type
	   ,(c = up->u_ttyid) ? c : '?'
	   ,up->u_u_id
	   ,ctime( up->u_logintime )
	 );
}
