#
/*
 *	execc - call texec with default bin directories
 *		prepended to "name".
 */

#include	<local-system>

#ifdef	AUSAM
#include	<uid.h>
#endif



execc( name , argv )
  register char *name;
  register char *argv[];
{
	register unsigned uid;
	char *execat();
	char buf[50];

#ifndef	AUSAM
  uid = ( getuid() >> 8 ) & 0377;
#endif
#ifdef	AUSAM
  uid = EFTVUID();
#endif

  if ( !uid && name[0] != '/' )  {
	texec( execat( buf, &buf[sizeof buf], "/etc/" , name ) , argv );
  }else
	texec( name , argv );

  if ( name[0] == '/' )
	return( -1 );

  texec( execat( buf, &buf[sizeof buf], "/bin/" , name ) , argv );
  texec( execat( buf, &buf[sizeof buf], "/usr/bin/" , name ) , argv );

  if ( !uid )
	texec( name , argv );

  return( -1 );
}


char *execat( buf, ebuf, s2 , s3 )
  register char *s2, *s3;
{
	register char *s1 = buf;

  while(*s1++ = *s2++);
  s1--;
  while(*s1++ = *s3++)  if ( s1 > ebuf )  return( "?" );

  return( buf );
}


/*
 *	"texec" knows about command files and pascal interpreter
 */

#define	ENOEXEC	8
#define	ENOMEM	12
#define	E2BIG	7

texec( name , argv )
  register char *name;
  char *argv[];
{
	extern errno;

  execv( name, argv );
  if (errno==ENOEXEC) {
	char *v[256];  register char **vp = &v[1];
	{
		register char **avp = argv;

		*vp++ = name;
		if ( !*avp++ )  {
			*vp++ = name;
			*vp = 0;
		}else
			while ( *vp++ = *avp++ )
				if ( vp > &v[sizeof v/sizeof v[0]] )  {
					exeprs( "too many args\n" );
					exit( -1 );
				}
		vp = v;
	}
	{
		register pcfd;
		/*
		 *	if file exists but not executable
		 *	it may be a pascal obj file
		 *	magic number = 0404
		 */
		if( (pcfd=open(name,0))>0 ){
			int pcrd, pcmag;

			pcrd = read(pcfd,&pcmag,2); close(pcfd);
			if(pcrd==2)
				if( pcmag==0404 ) {
					*vp = "px";
					execv( "/usr/bin/px" , vp );
					execv( "/bin/px" , vp );
				}
		}
	}
	/*
	 *	not pascal obj must be shell cmd file
	 */
	*vp = "sh";
	execv( "/bin/sh" , vp );
	exeprs("No shell!\n");
	exit( -1 );	
  }
  if (errno==ENOMEM) {
	exeprs( name );
	exeprs(": too large\n");
	exit( -1 );	
  }
  if (errno==E2BIG) {
	exeprs( name );
	exeprs(": arglist too long\n");
	exit( -1 );	
  }
}


exeprs( s )  register char *s;  {  while ( *s )  write( 2 , s++ , 1);  }
