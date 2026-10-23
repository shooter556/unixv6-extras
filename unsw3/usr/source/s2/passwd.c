#
/*
 *	passwd [name [[old password] [new password]]]
 *
 *	Piers Lauder	Mar '78
 */

#include	<local-system>

#ifdef	AUSAM

#include	<passwd.h>
#include	<gtty.h>

struct	pwent	pe;
char		*args[4];




main( argc, argv )
  register argc;
  char **argv;
{
	register char		**argp;
		 char		sbuf[3][100];
	extern			finish();

  signal( 2, finish );
  argp = args;

  {
	register		i = argc;

	do
		*argp++ = *argv++;
	while
		( --i );
  }

  {
	register unsigned	uid = getreal();

	if ( argc < 2 )
	{
		pe.pw_uid = uid;
		if ( getpwlog( &pe, sbuf[0], (sizeof sbuf[0]) ) < 0 )
		{
			prints( 2, "Cannot access password!\n" );
			return( -1 );
		}
		*argp++ = sbuf[0];
	}
	else
	{
		pe.pw_strings[LNAME] = args[1];
		if ( (getpwuid( &pe, 0, 0 ) < 0 ) || (uid && (uid != pe.pw_uid)) )
		{
			prints( 2, "Who?\n" );
			return( -1 );
		}
	}

	if ( argc < 3 )
	{
		if ( uid == pe.pw_uid && pe.pw_pword[0] )
			*argp = getstring( "Old password: ", sbuf[1] );
		argp++;
	}

	if ( argc < 4 )
	{
		*argp = getstring( "New password: ", sbuf[2] );
		if( **argp == '\0' ) *argp = 0;
		argp++;
	}
  }

  {
	register char		*cp;

	if ( (cp = args[2]) && pwcmp( crypt( cp ), pe.pw_pword ) )
		prints( 2, "Sorry.\n" );
	else
	{
		if ( cp = *--argp )
			cp = crypt( cp );
		else
			cp = "\0\0\0\0\0\0\0";
		for ( argc = 0 ; argc < 8 ; )
			pe.pw_pword[argc++] = *cp++;
		if ( updtpwent( &pe ) <= 0 )
			prints( 2, "Unable to change password!\n" );
		else
			return( 0 );
	}
  }

  return( -1 );
}




pwcmp( cp1, cp2 )
  register char *cp1, *cp2;
{
	register i = 8;

  do
	if ( *cp1++ != *cp2++ )
		return( -1 );
  while
	( --i );

  return( 0 );
}




unsigned	savemode;
struct sgttyb	tty;



char *getstring( s, buf )
  char		*s;
  register char	*buf;
{

  prints( 1, s );

  gtty( 0, &tty );
  savemode = tty.mode;
  tty.mode =& ~ECHO;
  stty( 0, &tty );

  {
	register char 	*cp = buf;

	do
		if ( read( 0, cp, 1) <= 0 )
			exit( 0 );
	while
		( *cp++ != '\n' );

	*--cp = '\0';
  }

  tty.mode = savemode;
  stty( 0, &tty );
  savemode = 0;

  return( buf );
}



finish()
{
  if ( savemode )
  {
	tty.mode = savemode;
	stty( 0, &tty );
	prints( 2 , "\n" );
  }
  exit( -1 );
}

#endif
