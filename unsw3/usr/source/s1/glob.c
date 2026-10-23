#
#include	<local-system>

/* global command --

   glob params

   "*" in params matches r.e ".*"
   "?" in params matches r.e. "."
   "[...]" in params matches character class
   "[...a-z...]" in params matches a through z.
#ifndef VN6_ORIG
   "@name" looks up name in /etc/passwd (supports *?[])
   "%name" looks up name in /etc/passwd (supports *?[])
           match substitutes the default directory of name
#endif

   perform command with argument list
  constructed as follows:
#ifndef VN6_ORIG
     if param starts with "@" or "%" search /etc/passwd for matches
     if a match is found, continue search in default directory of user
#endif
     if param does not contain "*", "[", or "?", use it as is
     if it does, find all files in current directory
     which match the param, sort them, and use them

   prepend the command name with "/bin" or "/usr/bin"
   as required.
#ifndef VN6_ORIG

  Note: glob is only invoked by sh if it has work to do
	Modifications June 27, 1976 by Kenneth Birman:
	   "@" feature
	   "%" feature
	   extention of "*","?","[]" to work on directories too
#endif
*/

#ifndef VN6_ORIG
/*
 *	the following bugs fixed by i.j.
 *
 *	1) file descriptor 3 not closed prior to exec.
 *	2) some how someone had the misguided impression
 *	   that file names in diretory entries are null terminated
 *	   - this is not the case.
 *	3) if 'chdir' to be called and no match made
 *	   just exit - this causes an effective logoff.
 *
 *	fix0	piers lauder	Oct '77
 *
 *	dequote strings before passing as arguments
 *	( shell turns on bit 0200 if char was quoted )
 */

#endif

/*  ( no ETC_FOR_ROOT )
#define	ETC_FOR_ROOT
	/*
	 *	Piers Lauder	Nov '77
	 *
	 *	allow different default bin directory search for super users
	 *
	 *	is now:-	/etc,  /bin,  /usr/bin,  current.
	 */
#define	EXECC
	/*
	 *	use 'execc' to exec desired command.
	 *	execc handles command files, pascal
	 *	interpreter files and correct searching
	 *	of etc bin and usr/bin
	 */

#define	E2BIG	7
#define	ENOEXEC	8
#define	ENOENT	2

#ifndef VN6_ORIG
#define	MAXSTR	2000
#define SYSLIM	5120
#define	GOBBLE	200
#define	NFILES	16
#define	SFILE	"/etc/passwd"
char	*ava[600];		/* generated arguments */
#endif
#ifdef VN6_ORIG
#define	STRSIZ	522
char	ab[STRSIZ];		/* generated characters */
char	*ava[200];		/* generated arguments */
#endif
char	**av &ava[1];
#ifndef VN6_ORIG
char	matstr[MAXSTR];		/* intermediate results */
#ifndef	AUSAM
int	fin;			/* used by getchar */
#endif
int	memused;		/* keep track of generated arguments */
int	memlow;
int	memhigh;
int	ncat;
int	wflg	0;		/* set if any wild char */
int	fout;
#endif
#ifdef VN6_ORIG
char	*string ab;
#endif
int	errno;
int	ncoll;
#ifdef	ETC_FOR_ROOT
#ifdef	AUSAM
#include	<uid.h>
#endif
unsigned uid;
#endif

main(argc, argv)
char *argv[];
{
	register char *cp;
#ifndef VN6_ORIG
	register int ii;
/*	register char **ap;	/* piers_fix0 */
#endif

#ifndef VN6_ORIG
	if (argc < 2) {
#endif
#ifdef VN6_ORIG
	if (argc < 3) {
#endif
		write(2, "Arg count\n", 10);
		return;
	}
#ifndef VN6_ORIG
#ifndef	AUSAM
	fin = open(SFILE, 0);
#endif
	fout = 2;
	memhigh = memlow = sbrk(0);
	expand(*++argv);
	if(ncat != 1) {
		write(2,"command?\n",9);
		exit();
	}
#endif
#ifdef VN6_ORIG
	argv++;
	*av++ = *argv;
#endif
	while (--argc >= 2)
#ifndef VN6_ORIG
			expand(*++argv);
	if (wflg && ncoll==0) {
		if( !compar("/etc/chdir",ava[1]) ) execl("/etc/chdir",0);
#endif
#ifdef VN6_ORIG
		expand(*++argv);
	if (ncoll==0) {
#endif
		write(2, "No match\n", 9);
		return;
	}
#ifndef VN6_ORIG | AUSAM
	close(fin);	/* close that fucking file !! */
#endif
/*	for ( ap = &ava[1] ; cp = *ap++ ; )	/* piers_fix0 */
/*		while ( *cp++ =& 0177 );	/* piers_fix0 */

#ifndef	EXECC
#ifdef	ETC_FOR_ROOT
#ifndef	AUSAM
	if ((uid = getuid() & 0377) == 0 && ava[1][0] != '/' )  {
#endif
#ifdef	AUSAM
	if ((uid = EFTVUID()) == 0 && ava[1][0] != '/' )  {
#endif
		cp = cat( "/etc/" , ava[1] );
		execute( cp , &ava[1] );
	}else
#endif
	execute(ava[1], &ava[1]);
	cp = cat("/usr/bin/", ava[1]);
	execute(cp+4, &ava[1]);
	execute(cp, &ava[1]);
#ifdef	ETC_FOR_ROOT
	if ( !uid )
		execute( ava[1] , &ava[1] );
#endif
#else	EXECC
	execc( ava[1] , &ava[1] );
#endif	EXECC
	write(2, "Command not found\n", 18);
}

expand(as)
char *as;
{
#ifndef VN6_ORIG
	register char *s,*cs;
	register int c;
	int cf,backc,wild,mflg;
	char uname[16],*proto,**oav,*oas,ocs,*oc,*os;
#endif
#ifdef VN6_ORIG
	register char *s, *cs;
	register int dirf;
	char **oav;
	static struct {
		int	ino;
		char	name[16];
	} entry;
#endif

#ifndef VN6_ORIG
	cs = as;
	oav = av;
	cf = 0;
	mflg = 0;
	wild = 1;
	c = *cs++;
	if( (c == '@') || (c == '%') ) {
#ifndef	AUSAM
		if(fin == -1) {
			write(2,"No passwd file\n",15);
			exit();
		}
#endif
#endif
#ifdef VN6_ORIG
	s = cs = as;
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) {
			*av++ = cat(s, "");
			return;
		}
#endif
#ifndef VN6_ORIG
		while((c = *cs) && c != '/') {
			cs++;
			if(c == '*' || c == '?' || c == '[') { wild = 0; wflg = 1; }
		}
		if(c)
			cf++;
		*cs++ = 0;
		s = uname;
		proto = as + 1;
#ifndef	AUSAM
		seek(fin,0,0);
		/* buffered since fin>2 */
		while(c = getchar())
		if(c != ':')
			*s++ = c;
		else {
			backc = c;
			*s = 0;
#endif
#ifdef	AUSAM
		/*
		 * this quick fix needs improvement
		 */
		{
#include	<passwd.h>
			char sbuf[SSIZ];
			struct pwent pe;

			pe.pw_strings[LNAME] = proto;	/* no matching ! */
			if ( getpwuid( &pe, sbuf, SSIZ ) > 0 )  {
#endif
#ifndef	AUSAM
			if(amatch(uname,proto)) {
				mflg = 1;
				s = 0;
				do
					if((c = getchar()) == ':') s++;
				while(s<4 && c);
				if(!c)
					break;	/* bad passwd file */
#endif
				s = matstr;
#ifndef	AUSAM
				while((c = getchar()) != ':' && c != '\n' && c) *s++ = c;
				backc = c;
				if(!c)
					break;
				*s = 0;
#endif
#ifdef	AUSAM
				oc = cs;
				cs = pe.pw_strings[DIRPATH];
				while ( *s++ = *cs++ );
				s--;
				cs = oc;
#endif
				if(cf) {
					os = s++;
					for(oc = cs;(c = *cs++) && c!='*' && c!='?' && c!='[';*s++ = c);
					if(!c) {
						*os = '/';
						cf = 0;
					}
					cs = oc;
					*s = 0;
					s = os;
				}
				if(cf)
					dmatch(s - matstr, cs);
				else {
					ncoll++;
					*av++ = cat(matstr, "");
				}
				if(wild) {
					sort(oav);
					return;
				}
			}
#ifndef	AUSAM
			s = uname;
			if(backc != '\n') {
				while((c = getchar()) && c != '\n');
				if(!c)
					break;
			}
#endif
#ifdef	AUSAM
			pwclose();	/* close password file */
#endif
		}
		sort(oav);
		if(!mflg) wflg = 1;
		return;
#endif
	}
#ifndef VN6_ORIG
	ocs = as;
	proto = 0;
	s = matstr;
	if(c == '/') {
		*s++ = c;
		ocs = cs;
		c = *cs++;
		proto = s;
	}
	do {
		if(c == '/') {
			proto = s;
			ocs = cs;
#endif
#ifdef VN6_ORIG
	for (;;) {
		if (cs==s) {
			dirf = open(".", 0);
			s = "";
			break;
#endif
		}
#ifndef VN6_ORIG
		else if(c == '*' || c == '?' || c == '[') {
			if(proto == 0) {
				s = matstr;
				*s++ = '.';
				proto = s;
			}
			*proto = 0;
			dmatch(proto - matstr, ocs);
			sort(oav);
			return;
		}
		*s++ = c;
	} while(c = *cs++);
	*av++ = cat(as, "");
}

dmatch(mof,ps)
int mof;
char *ps;
{
	register char *cs,*s;
	register int c;
	int chan,df,loc,ncos, nread;
	char *estr,*e,*sp;
	struct {
		int ino;
		char name[16];
	} *entry, etab[32];
	static struct {
		int f_skip[2];
		int f_flags;
		int f_pad[15];
	} f;

	int *ep,*bp,n,buf[256]; /* expand whats been read using these */
	df = 0;
	s = cs = ps;
	/* special case ./anything to strip the ./ */
	ncos = 1;
	if(mof == 1)
	if(matstr[0] == '.') { mof = 0; ncos = 0; }
	else if(matstr[0] == '/') ncos = 0;
	estr = &matstr[mof];
	if(stat(matstr, &f) == -1)
		return;
	if((f.f_flags & 060000) != 040000)
		return;
	chan = open(matstr, 0);
	if(chan == -1)
		return;
	while((c = *s++) && c != '/');
	e = s;
	if(*--s)
		*(sp = s) = 0;
	else
		df++;
	loc = 0;
	while((nread = read(chan, buf, 512)) > 0)  {
		c = nread >> 4; /* no of directory entries */
		ep = etab; bp =buf;
		while( c-- ){
			n=8;
			do
				*ep++ = *bp++;
			while( --n ); /* copy one entry */
			*ep++ = 0;	/* terminate it */
		}
		loc =+ nread;
		nread =>> 4;
		for(entry = &etab[0]; entry < &etab[nread]; entry++) {
			if(entry->ino == 0)
				continue;
			if(match(entry->name, ps)) {
				s = estr;
				c = mof + ncos;
				cs = entry->name;
				if(ncos) *s++ = '/';
				while(*s++ = *cs++)
					c++;
				if(chan == NFILES) {
					close(chan);
					chan = -1;
				}
				if(df) {
					ncoll++;
					*av++ = cat(matstr, "");
				} else
					dmatch(c, e);
				if(chan == -1) {
					chan = open(matstr, 0);
					seek(chan, loc, 0);
				}
			}
		}
#endif
#ifdef VN6_ORIG
		if (*--cs == '/') {
			*cs = 0;
			dirf = open(s==cs? "/": s, 0);
			*cs++ = 0200;
			break;
		}
#endif
	}
#ifndef VN6_ORIG
	close(chan);
	if(!df)
		*sp = '/';
#endif
#ifdef VN6_ORIG
	if (dirf<0) {
		write(2, "No directory\n", 13);
		exit();
	}
	oav = av;
	while (read(dirf, &entry, 16) == 16) {
		if (entry.ino==0)
			continue;
		if (match(entry.name, cs)) {
			*av++ = cat(s, entry.name);
			ncoll++;
		}
	}
	close(dirf);
	sort(oav);
#endif
}

sort(oav)
char **oav;
{
	register char **p1, **p2, **c;

	p1 = oav;
	while (p1 < av-1) {
		p2 = p1;
		while(++p2 < av) {
			if (compar(*p1, *p2) > 0) {
				c = *p1;
				*p1 = *p2;
				*p2 = c;
			}
		}
		p1++;
	}
}

#ifndef	EXECC
execute(afile, aarg)
char *afile;
char **aarg;
{
	register char *file, **arg;

	arg = aarg;
	file = afile;
	execv(file, arg);
	if (errno==ENOEXEC) {
		arg[0] = file;
		*--arg = "/bin/sh";
		execv(*arg, arg);
	}
	if (errno==E2BIG)
		toolong();
}

#endif	EXECC
toolong()
{
	write(2, "Arg list too long\n", 18);
	exit();
}

match(s, p)
char *s, *p;
{
#ifndef VN6_ORIG
	wflg = 1;
#endif
	if (*s=='.' && *p!='.')
		return(0);
	return(amatch(s, p));
}

amatch(as, ap)
char *as, *ap;
{
	register char *s, *p;
	register scc;
	int c, cc, ok, lc;

	s = as;
	p = ap;
	if (scc = *s++)
		if ((scc =& 0177) == 0)
			scc = 0200;
	switch (c = *p++) {

	case '[':
		ok = 0;
		lc = 077777;
		while (cc = *p++) {
			if (cc==']') {
				if (ok)
					return(amatch(s, p));
				else
					return(0);
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++))
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return(0);

	default:
		if (c!=scc)
			return(0);

	case '?':
		if (scc)
			return(amatch(s, p));
		return(0);

	case '*':
		return(umatch(--s, p));

	case '\0':
		return(!scc);
	}
}

umatch(s, p)
char *s, *p;
{
	if(*p==0)
		return(1);
	while(*s)
		if (amatch(s++,p))
			return(1);
	return(0);
}

compar(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return(0);
	return (*--s1 - *s2);
}

cat(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;
	register int c;
#ifndef VN6_ORIG
	int n;
	char *s;
#endif

#ifndef VN6_ORIG
	c = 0;
#endif
#ifdef VN6_ORIG
	s2 = string;
#endif
	s1 = as1;
#ifndef VN6_ORIG
	while (*s1++) c++;
	s1 = as2;
	while(*s1++) c++;
	s = s2 = getcore(++c);
	n = 2;
	for(s1 = as1; n--; s1 = as2)
		while(c = *s1++)
		if(c =& 0177)
			*s2++ = c;
		else
#endif
#ifdef VN6_ORIG
	while (c = *s1++) {
		if (s2 > &ab[STRSIZ])
			toolong();
		c =& 0177;
		if (c==0) {
#endif
			*s2++ = '/';
#ifndef VN6_ORIG
	ncat++;
	*s2++ = 0;
	return(s);
#endif
#ifdef VN6_ORIG
			break;
		}
		*s2++ = c;
	}
	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ])
			toolong();
		*s2++ = c = *s1++;
	} while (c);
	s1 = string;
	string = s2;
	return(s1);
#endif
}
#ifndef VN6_ORIG

getcore(n)
register int n;
{
	register char *memi, *newmem;

	memi = memlow;
	memlow =+ n;
	memused =+ n;
	newmem = memhigh;
	while(newmem < memlow) newmem =+ GOBBLE;
	if(memused > SYSLIM || (memhigh < newmem && brk(newmem) == -1))
		toolong();
	memhigh = newmem;
	return(memi);
}
#endif
