/*
 *	A program to print the contents of a users password entry
 *	and his limits structure from core and his TDU figure
 */

/*
 *	some defines to configure certain things
 */

/*
#define	SCHEDULE	/* for sched params */
/*
#define	CORE		/* for core limits */
/*
#define	PAGE		/* for page limits */
/*
#define	CORE_DUSAGE	/* if disk usage maintained in core */

#include <passwd.h>
#include <lnode.h>

struct { int hiword; unsigned loword; };
struct pwent	pe;
struct lnode	ln;
char	tbuf[8];
char	*buf;
int	bufs;
int	uflag;
int	lflag;



/*
**	MAIN
*/

main(argc, argv)
  register argc;
  register char *argv[];
{

	if(argc > 1)
		{
		argc--;
		argv++;
		/* some args */
		if(*argv[0] == '-')
			{
			/* assume a 'u' flag for now */
			uflag++;
			argc--;
			argv++;
			}
		while (argc-- > 0)
			{
			printu(*argv++);
			}
		}
	  else
		{
		if ( getlogin(getreal()) )
			pent();
		}

}



/*
**	Print user details
*/

pent()
{
printf("Password and Lnode structures for UID:%7u\n\n", pe.pw_uid);
printf("LIMITS:\n");
printf("Process Limit:%6u%40tCore Limit:%11u Kbytes\n",ln.l_plimit, ln.l_climit);
if( !lflag)
	{
#ifdef	CORE
	printf("Processes:%10u%40tCore Usage:%11u KBytes\n",
		ln.l_refcount, ln.l_cusage);
#endif
#ifndef	CORE
	printf("Processes:%10u\n", ln.l_refcount);
#endif
	}
#ifdef	PAGE
printf("Line Printer:%6u0 pages%40tLA180 Printer:%8u pages\n",
	pe.pw_pages, pe.pw_pages*10/4);
#endif
printf("Disc Primary:%7u units%40tDisc Overshoot:%7u units\n",
	ln.l_dlimit, ln.l_doverflw);
printf("Disc Usage:");
if( ln.l_duse == -1 )
	printf("%9s\n","???");
else
	printf("%9u units\n\n",ln.l_duse);
#ifdef	SCHEDULE
printf("SCHEDULING:\n");
printf("Shares:%13u%40tAccumulated Usage:%13D\n\n",ln.l_shares, ln.l_usage);
#endif
printf("GENERAL:\n");
printf("Encrypted Password: \"%8.8s\"%40tWarnings:%5u\n",
	pe.pw_pword,
	pe.pw_warn);
printf("Total Connect Time: %s",ptime(pe.pw_contime,tbuf));
printf("%40tTotal CPU Time: %s\n",ptime(pe.pw_cputime / 60l, tbuf));
printf("Account last used at: %14s\n", pe.pw_extime ? ctime(pe.pw_extime) : "the epoch     " );
printf("STRINGS:\n");
printf("Login name: %s\n",pe.pw_strings[LNAME]);
printf("Initial Direcory: %s\n",pe.pw_strings[DIRPATH]);
if(*pe.pw_strings[SHELLPATH] == 0)
	{
	printf("Initial shell: /bin/sh\n");
	}
  else
	{
	printf("Initial shell: %s\n",pe.pw_strings[SHELLPATH]);
	}
printf("Names: %s %s\n\n", pe.pw_strings[FIRSTNAME], pe.pw_strings[LASTNAME]);


/*	check out to warn him to login again	*/

	if(	(!lflag) &&
		((ln.l_dlimit != pe.pw_dlimit) ||
		(ln.l_plimit != pe.pw_plimit) ||
		((ln.l_flags&DLIMIT) && (ln.l_duse < ln.l_dlimit))
			)
	  )
		{
		printf("Logout and Login again to reset limits\n");
		}
}



/*
**	Get the relevant entries from passwords and lnodes
*/

printu(p)
register unsigned p;
{

	if(uflag)
		{
		/* have asked to treat arg as uid */
		if ( getlogin(atoi(p)) )
			pent();
		if(buf != -1)
			{
			free(buf);
			buf = -1;
			}
		}
	  else
		{
		/* have an arg to try as login name */
		if ( getuserid(p) )
			pent();
		if(buf != -1)
			{
			free(buf);
			buf = -1;
			}
		}
}





/*
**	Get login name
*/

getlogin(uid)
register unsigned	uid;
{

	lflag = 0;
	pe.pw_uid = uid;
	if(getpwlog(&pe, 0, 0) == 0)
		{
		put();
		return( 1 );
		}
	  else
		{
		printf("Uid does not exist !?!\n");
		return( 0 );
		}

}




/*
**	Set up user limits and strings
*/

put()
{

ln.l_uid = pe.pw_uid;
buf = alloc(bufs = sum());
getpwlog(&pe, buf, bufs);
if(lflag = (limits(&ln, L_OTHLIM) < 0))
	{
	printf("No limits structure in core\n\n");
	ln.l_shares = pe.pw_shares;
	ln.l_usage = pe.pw_shares;
	ln.l_dlimit = pe.pw_dlimit;
	ln.l_doverflw = pe.pw_doverflw;
	ln.l_plimit = pe.pw_plimit;
	ln.l_climit = pe.pw_climit;
#	ifndef	CORE_DUSAGE
	}
#	endif	CORE_DUSAGE
	if ( pe.pw_dlimit )
		ln.l_duse = tdu( pe.pw_strings[DIRPATH] );
#	ifdef	CORE_DUSAGE
	}
#	endif	CORE_DUSAGE
}




/*
**	Get user user-id
*/

getuserid(s)
register char	*s;
{

	lflag = 0;
	pe.pw_strings[LNAME] = s;
	if(getpwuid(&pe, 0, 0) == 0)
		{
		put();
		return( 1 );
		}
	  else
		{
		printf("Login name does not exist !?!\n");
		return( 0 );
		}

}




/*
**	Add up string lengths for password entry
*/

sum()
{
register int	i;
register int	s;

s=0;
for(i=0; i<PWSLENCNT; i++)
	{
	s =+ pe.pw_strings[i];
	}
return(s);
}
