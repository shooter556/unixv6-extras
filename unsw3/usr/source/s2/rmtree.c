#
/*
 *	rmtree name [ initdir ]
 *
 * Rmtree removes all files and (recursively) directories contained
 *  in directory ____name which is in _______initdir (default "/tmp"). Finally
 *  it removes directory ____name itself. ____name need not be a directory.
 *
 *  Rmtree understands circular directories, and misuse of ".." and
 *  ".". Any irregularities encountered are reported on file 2, and
 *  if unresolvable, stop all ancestor directory removal.
 *
 *	Piers Lauder	Feb '78
 */

#include	<local-system>
#include	<stat16.h>

struct	statbuf	statb;

char	tmpd[]	"/tmp";

char	*progname;

char	*pathname, *pathend;
char	*nextpath();
#define	SSIZ	16

/*
#define	TRACE		/* traces unlinks */

struct	{ unsigned sb_dev; };
unsigned	dev;

int		errorflag;


main( argc , argv )
  char **argv;
{
	extern	end;
	extern	fout;
	unsigned parino;

  close( 0 );
  close( 1 );
  fout = 2;

  progname = argv[0];

  pathname = "/";

  if ( argc < 2 )
	fatalerror( "treename?", "" );

  if ( argv[1][0] == '/' || dots( argv[1] ) )
	fatalerror( "unwise", argv[1] );

  if ( argc == 2 )
	argv[2] = tmpd;
  else
	if ( argv[2][0] != '/' )
		fatalerror( "full pathname required", argv[2] );

  if ( newstat( argv[2], &statb ) < 0 || chdir( argv[2] ) < 0 )
	fatalerror( "bad directory!", argv[2] );

  parino = statb.sb_inumber;

  circular();	/* enter parent node */

  pathname = argv[2];

  if ( newstat( argv[1], &statb ) < 0 )
	fatalerror( "file not found!", argv[1] );

  if ( parino == statb.sb_inumber || statb.sb_inumber == 1 )
	fatalerror( "damned unwise!", argv[1] );

  dev = statb.sb_dev;

  pathend = &end;
  pathname = &end;

  rmtree( statb.sb_inumber, nextpath( pathname, argv[2] ) );

  return( errorflag );
}



/*
 * rmtree removes all files and (recursively) directories belonging to user in the current directory
 */
rmtree( ino, path )
  unsigned ino;
  char *path;
{
	register fd, posn;
	static struct { int f_ino; char f_name[14]; char f_dotdot[4]; } dirb;
	register char *name = dirb.f_name;
	int	i, errorkeep;
	extern	errno;

# ifdef	TRACE
	trace( "" );
# endif	TRACE

  posn = 0;

  if ( (fd = open( pathname , 0 )) >= 0 )
  {
	while ( (i = read( fd , &dirb , 16 )) == 16 )
	{
		posn =+ 16;
		errno = 0;

		if ( dirb.f_ino && ( !ino || ino == dirb.f_ino ) )
		{
			if ( newstat( name , &statb ) >= 0 )
			{
				if ( (statb.sb_flags & IFTYP) == IFDIR )
				{
					if ( statb.sb_dev != dev )
					{
						/* should exec a copy of itself and pass back error states */
						error( "mounted file system", name );
						continue;
					}

					if ( !circular() )
					{
						if ( chdir( name ) >= 0 )
						{
							close( fd );
							errorkeep = errorflag;
							if ( posn <= 32 )
							{
								if ( dots( name ) )
									error( "bad directory link", name );
								else
									error( "\".\" or \"..\" displaced", name );
							}
							errorflag = 0;

							rmtree( 0, nextpath( path, name ) );

							*path = 0;
							chdir( pathname );
							fd = open( pathname , 0 );
							seek( fd , posn-16 , 0 );
							read( fd, &dirb , 16 );
							if ( !errorflag )
							{
								while ( *name++ );
								--name;
								*name++ = '/';
								*name++ = '.';
								*name++ = '.';
								*name = '\0';
								if ( unlink( dirb.f_name ) < 0 )
									error( "non-existent link", dirb.f_name );
								*--name = '\0';
								if ( unlink( dirb.f_name ) < 0 )
									error( "non-existent link", dirb.f_name );
								*--name = '\0';
								*--name = '\0';
								name = dirb.f_name;
								errorflag = errorkeep;
							}
							else
							{
								chown( name, 0 );
								chmod( name, 0 );
								continue;
							}
						}
						else
						{
							error( "cannot chdir", name );
							continue;
						}
					}
					else
						if ( dots( name ) )
						{
							if ( posn > 32 )
							{
								errorkeep = errorflag;
								error( "directory link displaced", name );
								errorflag = errorkeep;
							}
							continue;
						}
				}
				else
				{
					if ( ino )
						fatalerror( "not tree node", name );

					if ( posn <= 32 )
					{
						errorkeep = errorflag;
						error( "directory link displaced", name );
						errorflag = errorkeep;
					}
				}

				unlink( name );

#				ifdef	TRACE
					trace( name );
					printf( "%d - %d\n", ino, dirb.f_ino );
#				endif	TRACE
			}
			else
				error( "cannot stat", name );

			if ( ino )
				return;
		}
	}

	if ( i )
		error( "read fail or malformed directory", "." );

	close( fd );
  }
  else
	error( "open fail", "." );
}



error( s1, s2 )
  char *s1, *s2;
{
	extern	errno;

  errorflag++;
  printf( "%s: %s in %s", progname, s1, pathname );
  if ( errno )
  {
	printf( "\n  " );
	perror( s2 );
	printf( "  modes = %o, uid = %d\n", statb.sb_flags, statb.sb_uid );
  }
  else
	printf( ": \"%s\"\n", s2 );
}


char *nextpath( at, name )
  register char *at, *name;
{
  if ( at != pathname && at[-1] != '/' )
	*at++ = '/';

  do
	if ( at >= pathend )
		if ( sbrk( SSIZ ) >= 0 )
			pathend =+ SSIZ;
		else
		{
			*--at = 0;
			fatalerror( "out of memory", "" );
		}
  while
	( *at++ = *name++ );

  return( --at );
}



unsigned	map[07777];

int circular()
{
	register unsigned i, *j;
	register unsigned n = statb.sb_inumber;

  i = 1<<(n & 017);
  j = &map[(n >> 4) & 07777];

  if ( *j & i )
	return( 1 );
  else
	*j =| i;

  return( 0 );
}



dots( np )
  register char *np;
{
	register i;

  for ( i = 0 ; i < 2 ; i++ , np++ )
	if ( *np != '.' )
		break;

  return( *np == 0 && i );
}



fatalerror( s1, s2 )
  char *s1, *s2;
{
  error( s1, s2 );
  exit( -1 );
}


#ifdef	TRACE
trace( s )
{
  printf( "trace: %s in %s\n", s, pathname );
  sleep( 2 );
}
#endif	TRACE



#ifdef	SPRINTF
#define	NUMBERS
#define	OCTAL

#include	<sprintf.h>

#endif	SPRINTF
