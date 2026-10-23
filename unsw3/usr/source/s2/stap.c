#
/*
 *	stap [x[n]...] ...
 *
 * manipulate tape position
 *
 *  where 'x' represents functions available:
 *	r - rewind
 *	f - skip forward file
 *	F - skip reverse file
 *	b - skip forward block
 *	B - skip reverse block
 *	e - write tape mark
 *	m - write tape mark
 *	o - put drive off line
 *	c - clear controller
 *  and 'n' is a (decimal) count ( default 1 ).
 *
 * Manipulations proceed in the order given.
 * Default will position tape at the beginning of the next file.
 *
 *		    by
 *	       piers lauder
 *	DEPT. OF COMPUTER SCIENCE
 *	   UNIVERSITY OF SYDNEY
 *		 feb 1978
 */

#include	<local-system>


char	tapf[]	"/dev/rnmt0";

int	args[3];

char	*defv[]	{ "", "f" };

main( argc, argv )
  char **argv;
{
	register char *av;
	register c, n;
	int fd;

  close( 0 );	close( 1 );

  if ( (fd = open( tapf , 0 )) < 0 )
  {
	printf( "%s: cannot open %s", argv[0], tapf );
	perror();
	return( -1 );
  }
  if ( argc == 1 )
  {
	argc = (sizeof defv)/sizeof defv[0];
	argv = defv;
  }

  defv[0] = argv[0];
  while ( --argc )
  {
	av = *++argv;

	while ( *av )
	{
		switch ( c = *av++ )
		{
		 case 'f':	args[0] = 0;	break;
		 case 'F':	args[0] = 1;	break;
		 case 'c':	args[0] = 2;	break;
		 case 'm':
		 case 'e':	args[0] = 3;	break;
		 case 'b':	args[0] = 4;	break;
		 case 'B':	args[0] = 5;	break;
		 case 'o':	args[0] = 6;	break;
		 case 'r':	args[0] = 7;	break;

		 default:	printf( "%s: bad arg %c\n" , defv[0] , c );
				continue;
		}

		n = 0;
		while ( (c = *av) <= '9' && c >= '0' )
		{
			av++;
			n = n*10 + c-'0';
		}
		args[1] = n?n:1;

		if ( args[0] == 3 )	/* write tape mark */
		{
			close( fd );
			if ( (fd = open( tapf, 2 )) < 0 )
			{
				printf( "%s: cannot open %s to write tape mark", defv[0], tapf );
				perror();
				return( -1 );
			}
		}

		stty( fd , args );
	}
  }

  return( 0 );
}


#ifdef	SPRINTF
#define		CHARACTERS

#include	<sprintf.h>

#endif	SPRINTF
