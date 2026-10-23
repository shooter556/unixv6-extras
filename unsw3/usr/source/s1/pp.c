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
#define	CORE	/* for core limits */
/*
#define	PAGE		/* for page limits */

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

main(argc, argv)
char	*argv[];
{

	if(argc > 1)
		{
		/* some args */
		argv++;
		argc--;
		if(*argv[0] == '-')
			{
			/* assume a 'u' flag for now */
			uflag++;
			argv++;
			argc--;
			}
		while (argc-- > 0)
			{
			printu(*argv++);
			}
		}
	  else
		{
		getlogin(getreal());
		pent();
		}

}

/*****************************************************************************/

#define	TIME_IN_DAYS

int	zero;

char *ptime( time, sp )
  long time;
  char *sp;
{
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

			if ( time > 24000 )	/* more than 1000 days */
				trac = 0;
		}
#else
		if ( time > 60000L )	/* more than 1000 hours */
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

/*****************************************************************************/




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

/*****************************************************************************/

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
printf("Disc Usage:%9u units\n\n",ln.l_duse);
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
printf("Account last used at: %14s\n", ctime(pe.pw_extime));
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


/*	check out to wan him to login again	*/

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

/*****************************************************************************/

#include	<stat16.h>

struct statbuf statb;

/*
 *	"tdu" returns the disc usage
 *		Inodes are weighted at IWEIGHT blocks.
 *		Links are shared equally.
 */

long gettdu()
{
	register fd, posn;
	long inodes, blocks;
	static struct { int f_ino; char f_name[14]; char f_end; } dirbuf;
	static char dot[]	".";
	unsigned size();


  inodes = 0X10000;
  blocks.loword = 0;
  blocks.hiword = size();

  posn = 32;
  dirbuf.f_end = '\0';

loop:
  fd = open( dot , 0 );
  seek( fd , posn , 0 );

  while ( read( fd , &dirbuf , 16 ) == 16 )  {
	posn =+ 16;
	if ( dirbuf.f_ino && newstat( dirbuf.f_name , &statb ) >= 0 )
		if ( (statb.sb_flags & IFTYP) == IFDIR )  {
			if ( chdir( dirbuf.f_name ) >= 0 )  {
				close( fd );
				blocks =+ gettdu();
				chdir( ".." );
				goto loop;
			}
		}else  {
			/* fixed point arithmetic */
			inodes =+ (0X10000 / (statb.sb_nlinks&0377));
			blocks =+ ((0X10000 * size()) / (statb.sb_nlinks&0377));
		}
  }

  close( fd );
  return( blocks + inodes*IWEIGHT );
}

/*****************************************************************************/

unsigned size()
{
	register unsigned b, ib;

  b = ((statb.sb_size0 & 0377) << 7) + (((statb.sb_size1 + 0777) >> 9) & 0177);
  if ( ib = b>>3 )  {
	b++;
	if ( ib =>> 5 )  {
		b =+ ib;	/* indirect */
		if ( ib > 8 )
			b++;	/* double indirect */
	}
  }

  return( b );
}

/*****************************************************************************/

printu(p)
register unsigned p;
{
/*	get the relevant entries from passwords and lnodes	*/

	if(uflag)
		{
		/* have asked to treat arg as uid */
		getlogin(atoi(p));
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
		getuserid(p);
		pent();
		if(buf != -1)
			{
			free(buf);
			buf = -1;
			}
		}
}

/*****************************************************************************/

getlogin(uid)
register unsigned	uid;
{

	lflag = 0;
	pe.pw_uid = uid;
	if(getpwlog(&pe, 0, 0) == 0)
		{
		put();
		}
	  else
		{
		printf("Uid does not exist !?!\n");
		}

}

/*****************************************************************************/

put()
{
long	l;

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
/*	while TDU is not maintained in core... leave commented out
 *	if(chdir(pe.pw_strings[DIRPATH]) < 0)
 *		{
 *		printf("cant chdir to home directory ?!?\n\n");
 *		ln.l_duse=0;
 *		}
 *	  else
 *		{
 *		newstat( "." , &statb );
 *		l = gettdu();
 *		ln.l_duse = l.hiword;
 *		}
 *	and when in comment out stuff below to closing }	*/
	}
if(chdir(pe.pw_strings[DIRPATH]) < 0)
	{
	printf("cant chdir to home directory ?!?\n\n");
	ln.l_duse=0;
	}
  else
	{
	newstat( "." , &statb );
	l = gettdu();
	ln.l_duse = l.hiword;
	}
}

/*****************************************************************************/

getuserid(s)
register char	*s;
{

	lflag = 0;
	pe.pw_strings[LNAME] = s;
	if(getpwuid(&pe, 0, 0) == 0)
		{
		put();
		}
	  else
		{
		printf("Login name does not exist !?!\n");
		}

}

/*****************************************************************************/

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
