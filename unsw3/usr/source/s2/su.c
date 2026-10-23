#include	<defines.h>
#include	<param.h>
#include	<passwd.h>
#include	<wtmp.h>

/*
 * su -- become super-user
 */

char	password[65];
#ifndef	AUSAM16
char	pwbuf[100];
#endif
#ifdef AUSAM16
struct pwent pe;
char	wtmpf[]	"/usr/adm/wtmp";
#endif
int	ttybuf[3];




main(argc,argv)
  int argc;
  char **argv;
{
	register char *p, *q;
	register unsigned uid;
	extern fin;

#ifndef AUSAM16
	if(getpw(0, pwbuf))
		goto badpw;
#endif
#ifdef AUSAM16
	uid = getreal();
	p = q = argv[0];
	while( *p )
		if( *p++ == '/' ) q = p;
	if( *q == 's' ) {
		/* super user command */
		if( argc != 1 ) goto error;
		pe.pw_uid = 0;
		if(getpwlog(&pe, 0, 0))
			goto badpw;
	} else if( *q == 'l' ) {
		/* login command */
		if( argc != 2 ) goto error;
		pe.pw_strings[LNAME] = argv[1];
		if(getpwuid( &pe, 0, 0))
			goto badpw;
	        } else goto error;
#endif
#ifndef AUSAM16
	(&fin)[1] = 0;
	p = pwbuf;
	while(*p != ':')
		if(*p++ == '\0')
			goto badpw;
	if(*++p == ':')
		goto ok;
#endif
#ifdef AUSAM16
	if(pe.pw_pword[0] == 0)
		goto ok;
	p = pe.pw_pword;
#endif
	gtty(0, ttybuf);
	ttybuf[2] =& ~010;
	stty(0, ttybuf);
	printf("password: ");
	q = password;
	while((*q = getchar()) != '\n')
		if((*q++ == '\0')||(q>&password[64]))
			return;
	*q = '\0';
	ttybuf[2] =| 010;
	stty(0, ttybuf);
	printf("\n");
	q = crypt(password);
	while(*q++ == *p++);
#ifndef AUSAM16
	if(*--q == '\0' && *--p == ':')
#endif
#ifdef AUSAM16
	if(p >= &pe.pw_pword[8])
#endif
		goto ok;
	goto error;

badpw:
	printf("bad password file\n");
	goto error;	/* ian j july 1976 */
			/* no super user default */
ok:
#ifndef	AUSAM16
	setuid(0);
#else
	if ( uid > SYSUSERS && pe.pw_uid <= SYSUSERS )
	{
		struct sutmp safe;

		safe.su_type = SU_TYPE;
		safe.su_olduid = uid;
		safe.su_newuid = pe.pw_uid;
		safe.su_usetime = time();
		if ( (uid = open( wtmpf, 2 )) != -1 )
		{
			seek( uid, 0, 2 );
			write( uid, &safe, sizeof safe );
			close( uid );
		}
	}
	setuid( pe.pw_uid );
#endif	AUSAM16
	execl("/bin/sh", "-", 0);
	printf("cannot execute shell\n");
error:
	printf("sorry\n");
}
