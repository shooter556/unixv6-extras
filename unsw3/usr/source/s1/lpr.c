#
#include	<local-system>

/*
 *	lpr -- spool printout for online printer.
 *		has been modified extensively from distrib
 *
 */


#ifdef	AUSAM
#include	<stat.16.h>
struct	statbuf	statbuf;
#include	<passwd.h>
#endif

#define TLIMIT	3	/* max number of 256 block increments allowed
			   for copy file */
#ifndef	AUSAM
#define privlusr 5		/* the elite can have prty print */
#endif

char tfnmh[]	"/tmp/lpd/tfaXXXXXX";
char lfnmh[]	"/tmp/lpd/lfaXXXXXX";
char cfnmh[]	"/tmp/lpd/cfaXXXXXX";
char dfnmh[]	"/tmp/lpd/x/dfxXXXXXX";
#define	xposn	dfnmh[9]
char lockfile[]	"/tmp/lpd/lock\0";
#define	lockposn	lockfile[13]

 
int	nact;
int	tff;
#ifndef	AUSAM
int	quick;
#endif
int	iden;
#ifndef	AUSAM
char	statbuf[20];
#endif
int	buf[256];
char	remmsg[]	": can't remove\n";
unsigned ncopies	0;

struct {
	int	hi_int;
	int	lo_int;
};

struct {
	char	lo_byte;
	char	hi_byte;
}

main(argc, argv)
int argc;
char *argv[];
{
	int lockfd,lpdpid;
	register char *arg ,*cb;
	int flag=0,xban=0,push=0,yflg=0;
	char *xmsg=0;
	int register f;
	extern rubout();

	if( ! (signal(2, 1) & 01) ) signal(2, rubout);
	mktemp(lfnmh); mktemp(cfnmh);
	tff = creat(mktemp(tfnmh), 0600);
	if(tff < 0)
	{
		perror(tfnmh);
		exit(1);
	}
	while (argc > 1 && (arg = argv[1])[0] == '-')
	{
		switch (arg[1])
		{

	    case 'c':
			flag = '+';
			break;

	    case 'r':
			flag = '-';
			break;

#ifndef	AUSAM
	    case 'e':
			if(arg[2] > '0' && arg[2] <= '9')
				quick =+ arg[2]-'0';
			else quick++;
			break;

	    case 's':
			quick--;
			break;
#endif
	    case 'i':
			cb = &buf[100];
			arg++;
			while(*cb++ = *++arg);
#ifndef	AUSAM
			cb[-1] = ':';
#endif	AUSAM
			iden++;
			break;
	    case 'x':
			xban++;
			break;
	    case 'p':
			push++;
			break;
	    case 'm':
			xmsg = arg+1;
			break;
	    case 'n':
			ncopies = &arg[2];
			break;
	    case 'y':
			yflg++;
			break;		/* cause supression of page feeds
					   when printing */
	    case '1':
	    case '2':
			lockposn = xposn = arg[1];
			break;		/* select another printer */
		}
		argc--;
		argv++;
	}
#ifndef	AUSAM
	f = getuid();
	buf[1] = f;
	if( ( f == 0 ) && ( quick >= 0 ) ) quick=9;	/* ensure us super user types dont wait */
	if(f <= privlusr)
		quick++;
	else	if(quick > 0) quick = 0;
	if(quick < -1) quick = -1;
	if(quick >= 9) quick = 9-(f!=0);
#else
	buf[1] = getreal();
#endif	AUSAM
	card('I',&buf->hi_byte);
	if(xban==0) ident();
	if(push) card('P',"p");
	if( ncopies ) card('N', ncopies);
	if( yflg) card('Y',"y");
	if(argc == 1)
		copy(0);
	while(--argc)
	{
		arg = *++argv;
		if( access(arg, 4) )
		{
			perror(arg);
			continue;
		}
		if(flag == '+')
			goto cf;
		if(*arg == '/'  )
		{
			if( (f=open(arg,0))<=0 )
			{
				perror(arg);
				continue;
			}
			close(f);
			card('F', arg);
			if( flag == '-' )
				if(remvp(arg) == 0)
					card('U',arg);
				else
				{
					prints(2, arg);
					prints(2, remmsg);
				}
			nact++;
			continue;
		}
		f = ranname(lfnmh);
		if(link(arg, f))
				goto cf;
		card('F', f);
		card('U', f);
		nact++;
		goto df;
	cf:
		f = open(arg, 0);
		if(f < 0)
		{
			perror(arg);
			continue;
		}
		copy(f);
		close(f);
	df:
		if( (flag == '-') )
			if( remvp(arg) == 0 )	/* check for remove permission */
				f = unlink(arg);
			else
			{
				prints(2, arg);
				prints(2, remmsg);
			}
	}

	if( xmsg ){
		*xmsg = ttyn(2);
		card('M', xmsg);
	}
	if(nact)
	{
#ifndef	AUSAM
		dfnmh[13] =- quick;
#endif
		if(link(tfnmh,mktemp(dfnmh)))
		{
			prints(2, "can't rename in LPD\n");
			exit();
		}
		unlink(tfnmh);
		if( ! (
			((lockfd=open(lockfile,0))>0) &&
			(read(lockfd,&lpdpid,2)==2)	     &&
			(kill(lpdpid,4)!= -1)
		 )) prints(2, "LPD not accessible tell a guru\n");
	}
	unlink(tfnmh);
}

copy(f)
int f;
{
	int register i;	long nc;
	int ff, fn;

	ff = creat(fn=ranname(cfnmh),0666);
	if(ff < 0) {
		prints(2, "can't create in LPD spool\n");
		return;
	}
	nc = 0;
	while((i = read(f, buf, 512)) > 0) {
		write(ff, buf, i);
		nc =+ i;
		if(nc.hi_int > TLIMIT) {
			prints(2, "copy file is too large\n");
			break;
		}
	}
	close(ff);
	card('F', fn);
	card('U', fn);
	nact++;
}

card(c, s)
int c;
char s[];
{
	write(tff,&c,1);
	write(tff,s,39);	/* lpd record size is 40 chars */
}







#ifndef	AUSAM
ident()
{
	register char *b1p, *b2p;

	b2p = &buf[100];
	if(!iden && getpw(getuid(), b2p)) b2p = "pdp:";
	b1p = b2p;
	while(*b2p++ != ':') ;
	b2p[-1] = 0;
	card('L', b1p);
}
#endif	AUSAM
#ifdef	AUSAM
ident()
{
	register char *b2p;
	struct	pwent	pe;

	b2p = &buf[100];
	if( !iden)
	{
		pe.pw_uid = getreal();
		if( getpwlog(&pe, b2p, 300) < 0 )
		{
			b2p = "PHANTOM";
		}
		else if( (*b2p >= '0') && (*b2p <= '9') )
			b2p = pe.pw_strings[LASTNAME];
	}
	card('L', b2p);
}
#endif	AUSAM

ranname(s)
char s[];
{

loop:
#ifndef	AUSAM
	if(stat(s, statbuf))
#endif
#ifdef	AUSAM
	if ( newstat( s, &statbuf ) )
#endif
		return(s);
	s[11]++;
	goto loop;
}

remvp(file)
char	*file;
{
	int	wflag;
	register char *p,*q,*r;

	wflag = 0;
#ifdef	AUSAM
	newstat(file, &statbuf);
	if(statbuf.sb_flags & IFTYP)
		return -1;
#else
	stat(file, statbuf);
	if(statbuf[5] & 0140)
		return -1;
#endif	AUSAM
	p = file;
	r = p+1;
	if( *p == 0 )
		return -1;
	if( *p == '/' )
		q = "/";
	else
		q = "";
	while( *r)
	{
		if( *r++ == '/' )
		{
			while( *r == '/' )
				r++;
			if( *r )
				p = r;
			q = file;
		}
	}
	if( q == file )
	{
		*--p = 0;
		wflag = access( q, 2);
		*p = '/';
	}
	else
		wflag = access(q, 2);
	return wflag;
}
rubout()
{
	unlink(tfnmh); unlink(cfnmh);
	unlink(lfnmh); unlink(dfnmh);
	exit(-1);
}
