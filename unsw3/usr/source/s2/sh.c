#
#ifndef VN6_ORIG
/*
 * 	UNIX shell program
 *
 *	Modified 6/27/1976 by Kenneth Birman
 *	changes:
 *		extention of $0..$9 to $0...$9,$a...$z,$A...$Z
 *		change of $$ to $$$ for shell id
 *		addition of $$? feature, equivalent to
 *		$?,....$Z (whole of list after '?') i.e.
 *		$$1 is entire argument list
 *		$$c is entire list starting with $c (argument 12)
 *		modifications of glob to support:
 *			@name feature to refer to directory of name, i.e.
 *			%name feature to refer to directory of name, i.e.
 *				cp %birman/sh.c .
 *	 			ls %* /*.c
 *			extention of *,?,[..] to refer to directories, i.e.
 *	 			ls /l* /*.c
 *	 			rm %* /*.out
 *		The new glob is no slower than the old, but
 *		somewhat larger.  This appears unavoidable.
 *
 *	Modified 10/77 by Piers Lauder
 *	changes:
 *		everything necessary "ifdef"ed properly
 *		use shorter version of "ptime"
 *		new accounting:
 *			remove special treatment of "login"
 *		root bin directories are now in order:	/etc
 *							/bin
 *							/usr/bin
 *							current directory
 *		"echoall" after argument substitution
 *		AUSAM.
 *	fixes:
 *		fix0 -- "quoted quote" bug ( also glob )
 *		fix1 -- ":not found" name bug for command files
 *		fix2 -- ":not found" bug if arg list too long
 *		fix3 -- "quoted name" bug
 *		fix5 -- ignore single "$"
 *		fix6 -- return null string if $$? doesn't exist
 *
 *	General fixes:
 *		fix000 -- new cc changes
 *		fix001 -- do proper exits & use "waitx"
 */

#define	AUSAM
#define	ECHO_ALL
#define	ETC_FOR_ROOT
#define	NEW_ACCT
#define	TIME_OUT
#define	KILL_IT
/*
#define	SHELL_ACCT
 */
#define	FINISH_TIMES
/*#define CREF		/* DO NOT DEFINE! -- for cross-reference block nesting only */
#endif

#ifdef	AUSAM
#include	<uid.h>
unsigned	uid;
#endif

#ifdef	FINISH_TIMES
#define	HZ	50
/*
#include	<defines.h>
#include <param.h>
 */
#endif

#ifdef	TIME_OUT
#define	MAXWAIT	10*60
#define	ROOTWAIT 30*60
#define	CONSROOT 0177777
unsigned maxwait MAXWAIT;
#define	CLOCK	15
#endif
#define	INTR	2
#define	QUIT	3
#define LINSIZ 1000
#define ARGSIZ 150
#define TRESIZ 100

#define QUOTE 0200
#define FAND 1
#define FCAT 2
#define FPIN 4
#define FPOU 8
#define FPAR 16
#define FINT 32
#define FPRS 64
#define TCOM 1
#define TPAR 2
#define TFIL 3
#define TLST 4
#define DTYP 0
#define DLEF 1
#define DRIT 2
#define DFLG 3
#define DSPR 4
#define DCOM 5
#define	ENOMEM	12
#define	ENOEXEC 8
#define	E2BIG	7

char	*dolp;
char	pidp[6];
int	ldivr;
char	**dolv;
#ifndef VN6_ORIG
int	dolf;
int	doln;
#endif
int	dolc;
char	*promp;
char	*linep;
char	*elinep;
char	**argp;
char	**eargp;
int	*treep;
int	*treeend;
char	peekc;
char	gflg;
char	error;
#ifdef	SHELL_ACCT
char	acctf;
#endif
#ifndef	AUSAM
char	uid;
#endif
char	setintr;
char	*arginp;
int	onelflg;
#ifdef	ECHO_ALL
int	verbflg	0;	/* set then all commands read will be echoed
			   turned on by -v flag (shell args)
			   turned on by echoall
			   turned off by echonone
			*/
#endif

char	*mesg[] {
	0,
	"Hangup",
	0,
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Segmentation violation",
	"Bad system call",
#ifndef VN6_ORIG
	"Broken pipe",
	"system termination",
	"timeout",
	"cpu time exceeded",
#endif
#ifdef VN6_ORIG
	0,
	"Sig 14",
	"Sig 15",
	"Sig 16",
#endif
	"Sig 17",
	"Sig 18",
	"Sig 19",
};

#ifndef VN6_ORIG
long time();
#endif

struct stime {
	long	proct[2];	/* fix000 */
	long	cputim;	/* fix000 */
	long	systim;	/* fix000 */
};

#ifdef	SHELL_ACCT
	struct stime timeb;
#endif
#ifdef	FINISH_TIMES
	long start_time;
#endif

char	line[LINSIZ];
char	*args[ARGSIZ];
int	trebuf[TRESIZ];

main(c, av)
int c;
char **av;
{
	register f;
	register char *acname, **v;
#ifdef	TIME_OUT
	extern clock();
#endif

#ifdef	FINISH_TIMES
	start_time = time();
#endif

	for(f=2; f<15; f++)
		close(f);
	if((f=dup(1)) != 2)
		close(f);
	dolc = getpid();
	for(f=4; f>=0; f--) {
		dolc = ldiv(0, dolc, 10);
		pidp[f] = ldivr+'0';
	}
	promp = "% ";
#ifndef	AUSAM
	if ( (uid = getuid()&0377) == 0 )
#endif
#ifdef	AUSAM
	if ( (uid = EFTVUID()) == 0 )
#endif
		promp = "# ";

#ifdef	TIME_OUT
	if( uid==0 )
#ifdef	CONSROOT
		if( ttyn(0)=='8' ) maxwait = CONSROOT;	/* no timeout for console if root */
		else
#endif
			maxwait = ROOTWAIT;	/* super users get timed out after 1/2 hour */
#endif

#ifdef	SHELL_ACCT
	acname = "/usr/adm/sha";
	acctf = open(acname, 1);
#endif

	v = av;
	if(c > 1) {
		promp = 0;
		if (*v[1]=='-') {
			**v = '-';
			if (v[1][1]=='c' && c>2)
				arginp = v[2];	/* -c -- execute argument */
			else if (v[1][1]=='t')
				onelflg = 2;	/* -t -- read one line only */
#ifdef	ECHO_ALL
			else if (v[1][1]=='v')
				verbflg = 1;	/* -v -- start with "echoall" */
#endif
		} else {
			close(0);
			f = open(v[1], 0);
			if(f < 0) {
				prs(v[1]);
				err(": cannot open");
			}
		}
	}
	if(**v == '-') {
		setintr++;
		signal(QUIT, 1);
		signal(INTR, 1);
#ifdef	TIME_OUT
		signal(CLOCK, clock);
#endif
	}
	dolv = v+1;
	dolc = c-1;

loop:
	if(promp != 0)
		prs(promp);
	peekc = getc();
	main1();
	goto loop;
}

main1()
{
	register char c, *cp;
	register *t;

	argp = args;
	eargp = args+ARGSIZ-5;
	linep = line;
	elinep = line+LINSIZ-5;
	error = 0;
	gflg = 0;
	do {		/* read a complete line */
		cp = linep;
		word();
	} while(*cp != '\n');

#ifdef	ECHO_ALL
	if ( verbflg )
		for ( cp = line ; cp < linep-1 ; )
			if ( c = *cp++ )
				putc( c );
			else
				putc( ' ' );
#endif

	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	if(gflg == 0) {
		if(error == 0) {
			setexit();
			if (error)
				return;
			t = syntax(args, argp);
		}
		if(error != 0)
			err("syntax error"); else
			execute(t);
	}
}

#ifdef	FINISH_TIMES

int	tpflag;

finish(flag)
{
	struct stime now;

	if( (!promp || !uid) && !flag) return;
	times(&now);
	prs("\nconnect  time "); ptime((time() - start_time)*HZ);
	prs("\nuser cpu time "); ptime(now.cputim+now.proct[0]);
	prs("\nsys  cpu time "); ptime(now.systim+now.proct[1]);
	prs("\n\n");
}

ptime( tics )
  long	tics;
{
	register long *t = &tics;
	struct { int hiint; unsigned loint; };

  tpflag = 0;
  tprintf( "%d:%d:%d.%d" , (*t=/60,t)->loint , (*t=/60,t)->loint%60 , (*t=/HZ,t)->loint%60 , (t->loint%HZ)*2 );
}

tprintf( s , a )
  register char *s;
  int a;
{
	register *ap = &a;
	register char c;

  while ( c = *s++ )
	if ( c == '%' )  {
		tprint1n( (*ap)/10 );
		if ( *++s == '.' )  tpflag++;
		tprint1n( (*ap++)%10 );
	}else
		putc( tpflag ? c : ' ' );
}

tprint1n( n )
  char n;
{
  putc( ((n && ++tpflag) || tpflag) ? ( n + '0' ) : ' ' );
}

#endif

int	quoteflg;	/* piers_fix0 */

word()
{
	register char c, c1;

	*argp++ = linep;

loop:
	switch(c = getc()) {

	case ' ':
	case '\t':
		goto loop;

	case '\'':
	case '"':
		c1 = c;
		quoteflg++;	/* piers_fix0 */
		while((c=getc()) != c1) {	/* piers_fix0 */
			if(c == '\n') {
				error++;
				peekc = c;
				quoteflg = 0;	/* piers_fix0 */
				return;
			}
			*linep++ = c|QUOTE;
		}
		quoteflg = 0;	/* piers_fix0 */
		goto pack;

	case '&':
	case ';':
	case '<':
	case '>':
	case '(':
	case ')':
	case '|':
	case '^':
	case '\n':
		*linep++ = c;
		*linep++ = '\0';
		return;
	}

	peekc = c;

pack:
	for(;;) {
		c = getc();
		if(any(c, " '\"\t;&<>()|^\n")) {
			peekc = c;
			if(any(c, "\"'"))
				goto loop;
			*linep++ = '\0';
			return;
		}
		*linep++ = c;
	}
}

tree(n)
int n;
{
	register *t;

	t = treep;
	treep =+ n;
	if (treep>treeend) {
		prs("Command line overflow\n");
		error++;
		reset();
	}
	return(t);
}

getc()
{
	register char c;
	static char backc;	/* piers_fix5 */

	if(peekc) {
		c = peekc;
		peekc = 0;
		return(c);
	}
	if(backc) {	/* piers_fix5 */
		c = backc;	/* piers_fix5 */
		backc = 0;	/* piers_fix5 */
		return(c);	/* piers_fix5 */
	}	/* piers_fix5 */
	if(argp > eargp) {
		argp =- 10;
		while((c=getc()) != '\n');
		argp =+ 10;
		err("Too many args");
		gflg++;
		return(c);
	}
	if(linep > elinep) {
		linep =- 10;
		while((c=getc()) != '\n');
		linep =+ 10;
		err("Too many characters");
		gflg++;
		return(c);
	}
getd:
	if(dolp) {
		c = *dolp++;
		if(c != '\0')
			return(c);
#ifndef VN6_ORIG
		if(dolf) {
			if(doln < dolc) {
				dolp = dolv[doln++];
				return(' ');
			}
			dolf = 0;
		}
#endif
		dolp = 0;
	}
	c = readc();
	if ( quoteflg )		/* piers_fix0 */
		return( c );	/* piers_fix0 */
	if(c == '\\') {
		c = readc();
		if(c == '\n')
			return(' ');
		return(c|QUOTE);
	}
	if(c == '$' && !promp) {	/* piers-fix4 */
		c = readc();
#ifndef VN6_ORIG
	setd:		/* piers-fix6 */
		if((c>='0' && c<='9') || (c>='a' && c<='z') || (c>= 'A' && c<='Z')) {
			if(c>='0' && c<='9') c = c - '0';
			else if(c>='a' && c<='z') c = 10 + c - 'a';
			else c = 36 + c - 'A';
			if(c < dolc)
				dolp = dolv[c];
			else  dolf = 0;	/* piers-fix6 */
			doln = c + 1; 
#endif
#ifdef VN6_ORIG
		if(c>='0' && c<='9') {
			if(c-'0' < dolc)
				dolp = dolv[c-'0'];
#endif
			goto getd;
		}
		if(c == '$') {
#ifndef VN6_ORIG
			c = readc();
			if(c != '$') {
				dolf = 1;	/* piers-fix6 */
				goto setd;
			}
#endif
			dolp = pidp;
			goto getd;
		}
		backc = c;  c = '$';	/* piers_fix5 */
#ifndef	VN6_ORIG
		dolf = 0;	/* piers_fix6 */
#endif
	}
	return(c);	/* piers_fix0 */
}
#ifdef	CREF
}
#endif

readc()
{
	char cc;
	register c;
#ifdef	TIME_OUT
	static newlflg 1;
#endif

	if (arginp) {
#ifdef	FINISH_TIMES
		if (arginp == 1) {
			finish(0);
			exit(0);
		}
#endif
#ifndef	FINISH_TIMES
		if (arginp == 1)
			exit(0);	/* fix001 */
#endif
		if ((c = *arginp++) == 0) {
			arginp = 1;
			c = '\n';
		}
		return(c&0177);	/* piers_fix0 */
	}
#ifndef	FINISH_TIMES
	if(onelflg==1)
		exit(0);	/* fix001 */
#endif
#ifdef	FINISH_TIMES
	if (onelflg==1) {
		finish(0);
		exit(0);
	}
#endif
#ifdef	TIME_OUT
	if ( newlflg )  {
		newlflg = 0;
#ifdef	CONSROOT
		if ( maxwait != CONSROOT )
#endif
			clktim( maxwait );
	}
#endif
#ifdef	FINISH_TIMES
	if(read(0, &cc, 1) != 1) {
		finish(0);
		exit(0);
	}
#endif
#ifndef	FINISH_TIMES
	if(read(0, &cc, 1) != 1)
		exit(0);	/* fix001 */
#endif
	cc =& 0177;	/* piers_fix0 */
#ifndef	TIME_OUT
	if (cc=='\n' && onelflg)
		onelflg--;
#endif
#ifdef	TIME_OUT
	if ( cc == '\n' )  {
		if ( onelflg )
			onelflg--;
#ifdef	CONSROOT
		if ( maxwait != CONSROOT )
#endif
			clktim( 0 );
		newlflg++;
	}
#endif
	return(cc);
}

/*
 * syntax
 *	empty
 *	syn1
 */

syntax(p1, p2)
char **p1, **p2;
{
	while(p1 != p2) {
		if(any(**p1, ";&\n"))
			p1++; else
			return(syn1(p1, p2));
	}
	return(0);
}

/*
 * syn1
 *	syn2
 *	syn2 & syntax
 *	syn2 ; syntax
 */

syn1(p1, p2)
char **p1, **p2;
{
	register char **p;
	register *t, *t1;
	int l;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		if(l < 0)
			error++;
		continue;

	case '&':
	case ';':
	case '\n':
		if(l == 0) {
			l = **p;
			t = tree(4);
			t[DTYP] = TLST;
			t[DLEF] = syn2(p1, p);
			t[DFLG] = 0;
			if(l == '&') {
				t1 = t[DLEF];
				t1[DFLG] =| FAND|FPRS|FINT;
			}
			t[DRIT] = syntax(p+1, p2);
			return(t);
		}
	}
	if(l == 0)
		return(syn2(p1, p2));
	error++;
}

/*
 * syn2
 *	syn3
 *	syn3 | syn2
 */

syn2(p1, p2)
char **p1, **p2;
{
	register char **p;
	register int l, *t;

	l = 0;
	for(p=p1; p!=p2; p++)
	switch(**p) {

	case '(':
		l++;
		continue;

	case ')':
		l--;
		continue;

	case '|':
	case '^':
		if(l == 0) {
			t = tree(4);
			t[DTYP] = TFIL;
			t[DLEF] = syn3(p1, p);
			t[DRIT] = syn2(p+1, p2);
			t[DFLG] = 0;
			return(t);
		}
	}
	return(syn3(p1, p2));
}

/*
 * syn3
 *	( syn1 ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 */

syn3(p1, p2)
char **p1, **p2;
{
	register char **p;
	char **lp, **rp;
	register *t;
	int n, l, i, o, c, flg;

	flg = 0;
	if(**p2 == ')')
		flg = FPAR;
	lp = 0;
	rp = 0;
	i = 0;
	o = 0;
	n = 0;
	l = 0;
	for(p=p1; p!=p2; p++)
	switch(c = **p) {

	case '(':
		if(l == 0) {
			if(lp != 0)
				error++;
			lp = p+1;
		}
		l++;
		continue;

	case ')':
		l--;
		if(l == 0)
			rp = p;
		continue;

	case '>':
		p++;
		if(p!=p2 && **p=='>')
			flg =| FCAT; else
			p--;

	case '<':
		if(l == 0) {
			p++;
			if(p == p2) {
				error++;
				p--;
			}
			if(any(**p, "<>("))
				error++;
			if(c == '<') {
				if(i != 0)
					error++;
				i = *p;
				continue;
			}
			if(o != 0)
				error++;
			o = *p;
		}
		continue;

	default:
		if(l == 0)
			p1[n++] = *p;
	}
	if(lp != 0) {
		if(n != 0)
			error++;
		t = tree(5);
		t[DTYP] = TPAR;
		t[DSPR] = syn1(lp, rp);
		goto out;
	}
	if(n == 0)
		error++;
	p1[n++] = 0;
	t = tree(n+5);
	t[DTYP] = TCOM;
	for(l=0; l<n; l++)
		t[l+DCOM] = p1[l];
out:
	t[DFLG] = flg;
	t[DLEF] = i;
	t[DRIT] = o;
	return(t);
}

scan(at, f)
int *at;
int (*f)();
{
	register char *p, c;
	register *t;

	t = at+DCOM;
	while(p = *t++)
		while(c = *p)
			*p++ = (*f)(c);
}

tglob(c)
int c;
{

#ifndef VN6_ORIG
	if(any(c,"%@[?*"))
#endif
#ifdef VN6_ORIG
	if(any(c, "[?*"))
#endif
		gflg = 1;
	return(c);
}

trim(c)
int c;
{
	return(c&0177);
}

trim1( cp )	/* piers_fix3 */
  register char *cp;	/* piers_fix3 */
{	/* piers_fix3 */
  while ( *cp++ =& 0177 );	/* piers_fix3 */
}	/* piers_fix3 */

execute(t, pf1, pf2)
int *t, *pf1, *pf2;
{
	int i, f, pv[2];
	register *t1;
	register char *cp1, *cp2;
	extern errno;
	struct { char **cpp; };

	if(t != 0)
	switch(t[DTYP]) {

	case TCOM:
		cp1 = t[DCOM];
		if( (equal(cp1, "cd")) || (equal(cp1, "chdir")) ) {
			if(t[DCOM+1] != 0) {
#ifndef VN6_ORIG
				scan(t, &tglob);
				if(gflg) {
					t[DSPR] = "/etc/glob";
					t[DCOM] = "/etc/chdir";
					execv(t[DSPR], t+DSPR);
				}
#endif
				if(chdir(t[DCOM+1]) < 0)
					err("chdir: bad directory");
			} else
				err("chdir: arg count");
			return;
		}
		if(equal(cp1, "shift")) {
			if(dolc < 1) {
				prs("shift: no args\n");
				return;
			}
			dolv[1] = dolv[0];
			dolv++;
			dolc--;
			return;
		}
#ifdef	ECHO_ALL
		if(equal(cp1, "echoall"))  { verbflg = 1; return; }
		if(equal(cp1, "echonone")) { verbflg = 0; return; }
#endif
#ifndef	NEW_ACCT
		if(equal(cp1, "login")) {
			if(promp != 0) {
#ifdef	SHELL_ACCT
				close(acctf);
#endif
#ifdef	FINISH_TIMES
				finish(0);
#endif
				execv("/bin/login", t+DCOM);
			}
			prs("login: cannot execute\n");
			return;
		}
#endif
#ifdef	KILL_IT
		if( equal ( cp1 , "kill" ) ) {
			killer( &t[DCOM+1] );
			return;
		}
#endif	KILL_IT
		if(equal(cp1, "wait")) {
			pwait(-1, 0);
			return;
		}
		if(equal(cp1, ":"))
			return;
#ifdef	FINISH_TIMES
		if(equal(cp1, "times")) {
			finish(1);
			return;
		}
#endif

	case TPAR:
		f = t[DFLG];
		i = 0;
		if((f&FPAR) == 0)
			i = fork();
		if(i == -1) {
			perror("");
			return;
		}
		if(i != 0) {
			if((f&FPIN) != 0) {
				close(pf1[0]);
				close(pf1[1]);
			}
			if((f&FPRS) != 0) {
				prn(i);
				prs("\n");
			}
			if((f&FAND) != 0)
				return;
			if((f&FPOU) == 0)
				pwait(i, t);
			return;
		}
		if(t[DLEF] != 0) {
			close(0);
			trim1( t[DLEF] );	/* piers_fix3 */
			i = open(t[DLEF], 0);
			if(i < 0) {
				prs(t[DLEF]);
				err(": cannot open");
				exit(0);	/* fix001 */
			}
		}
		if(t[DRIT] != 0) {
			trim1( t[DRIT] );	/* piers_fix3 */
			if((f&FCAT) != 0) {
				i = open(t[DRIT], 1);
				if(i >= 0) {
					seek(i, 0, 2);
					goto f1;
				}
			}
			i = creat(t[DRIT], 0666);
			if(i < 0) {
				prs(t[DRIT]);
				err(": cannot create");
				exit(0);	/* fix001 */
			}
		f1:
			close(1);
			dup(i);
			close(i);
		}
		if((f&FPIN) != 0) {
			close(0);
			dup(pf1[0]);
			close(pf1[0]);
			close(pf1[1]);
		}
		if((f&FPOU) != 0) {
			close(1);
			dup(pf2[1]);
			close(pf2[0]);
			close(pf2[1]);
		}
		if((f&FINT)!=0 && t[DLEF]==0 && (f&FPIN)==0) {
			close(0);
			open("/dev/null", 0);
		}
		if((f&FINT) == 0 && setintr) {
			signal(INTR, 0);
			signal(QUIT, 0);
		}
		if(t[DTYP] == TPAR) {
			if(t1 = t[DSPR])
				t1[DFLG] =| f&FINT;
			execute(t1);
			exit(0);	/* fix001 */
		}
#ifdef	SHELL_ACCT
		close(acctf);
#endif
		gflg = 0;
		scan(t, &tglob);
		if(gflg) {
			t[DSPR] = "/etc/glob";
			execv(t[DSPR], t+DSPR);
			prs("glob: cannot execute\n");
			exit(0);	/* fix001 */
		}
		scan(t, &trim);
#ifndef	ETC_FOR_ROOT
		texec(t[DCOM], t);
		cp1 = linep;
		cp2 = "/usr/bin/";
		while(*cp1 = *cp2++)
			cp1++;
		cp2 = t[DCOM];
		while(*cp1++ = *cp2++);
#endif
#ifdef	ETC_FOR_ROOT
		if ( !uid && t.cpp[DCOM][0] != '/' )  {
			cat( linep , "/etc/" , t[DCOM] );
			texec( linep , t );
		}else
			texec( t[DCOM] , t );
		cat( linep , "/usr/bin/" , t[DCOM] );
#endif
		texec(linep+4, t);
		texec(linep, t);
#ifdef	ETC_FOR_ROOT
		if ( !uid )
			texec( t[DCOM] , t );
#endif
		prs(t[DCOM]);
		err(": not found");
		exit(0);	/* fix001 */

	case TFIL:
		f = t[DFLG];
		pipe(pv);
		t1 = t[DLEF];
		t1[DFLG] =| FPOU | (f&(FPIN|FINT|FPRS));
		execute(t1, pf1, pv);
		t1 = t[DRIT];
		t1[DFLG] =| FPIN | (f&(FPOU|FINT|FAND|FPRS));
		execute(t1, pv, pf2);
		return;

	case TLST:
		f = t[DFLG]&FINT;
		if(t1 = t[DLEF])
			t1[DFLG] =| f;
		execute(t1);
		if(t1 = t[DRIT])
			t1[DFLG] =| f;
		execute(t1);
		return;

	}
}

#ifdef	ETC_FOR_ROOT
cat( s1 , s2 , s3 )
  register char *s1, *s2, *s3;
{
  while(*s1++ = *s2++);
  s1--;
  while(*s1++ = *s3++);
}
#endif


texec(f, t)
register *t;
{
	extern errno;
	register pcfd,pcrd;	int pcmag;

	execv(f, t+DCOM);
	if (errno==ENOEXEC) {
		t[DCOM] = f;	/* piers_fix1 */
		/*
		 *	if file exists but not executable
		 *	it may be a pascal obj file
		 *	magic number = 0404
		 */
		if( (pcfd=open(t[DCOM],0))>0 ){
			pcrd = read(pcfd,&pcmag,2); close(pcfd);
			if(pcrd==2)
				if( pcmag==0404 ) {
					t[DSPR] = "/bin/px";
					execv(t[DSPR], t+DSPR);
					t[DSPR] = "/usr/bin/px";
					execv(t[DSPR], t+DSPR);
					prs("No px!\n");
					exit(-1);
				}
		}
		/*
		 *	not pascal obj must be shell cmd file
		 */
		t[DSPR] = "/bin/sh";
		execv(t[DSPR], t+DSPR);
		prs("No shell!\n");
		exit(-1);	/* fix001 */
	}
	if (errno==ENOMEM) {
		prs(t[DCOM]);
		err(": too large");
		exit(-1);	/* fix001 */
	}
	if ( errno==E2BIG )  {		/* fix2 */
		err( "Arg list too long" ); /* fix2 */
		exit( -1 );		/* fix2 */
	}				/* fix2 */
}

err(s)
char *s;
{
	prs(s);
	prs("\n");
	if(promp == 0) {
		seek(0, 0, 2);
		exit(0);	/* fix001 */
	}
}

prs(as)
char *as;
{
	register char *s;

	s = as;
	while(*s)
		putc(*s++);
}

putc(c)
{
	write(2, &c, 1);
}

prn(n)
int n;
{
	register a;

	if(a=ldiv(0,n,10))
		prn(a);
	putc(lrem(0,n,10)+'0');
}

any(c, as)
int c;
char *as;
{
	register char *s;

	s = as;
	while(*s)
		if(*s++ == c)
			return(1);
	return(0);
}

equal(as1, as2)
char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while(*s1++ == *s2)
		if(*s2++ == '\0')
			return(1);
	return(0);
}

pwait(i, t)
int i, *t;
{
	register p, e;
	int s;

	if(i != 0)
	for(;;) {
#ifdef	SHELL_ACCT
		times(&timeb);
		time(timeb.proct);	/* fix000 */
#endif
		p = waitx(&s);	/* fix001 */
		if(p == -1)
			break;
		e = s&0177;
		if(mesg[e] != 0) {
			if(p != i) {
				prn(p);
				prs(": ");
			}
			prs(mesg[e]);
			if(s&0200)
				prs(" -- Core dumped");
		}
		if(e != 0)
			err("");
#ifdef	SHELL_ACCT
		if(i == p) {
			acct(t);
			break;
		} else
			acct(0);
#endif
#ifndef	SHELL_ACCT
		if ( i == p )  break;
#endif
	}
}

#ifdef	SHELL_ACCT
acct(t)
int *t;
{
	if(t == 0)
		enacct("**gok"); else
	if(*t == TPAR)
		enacct("()"); else
	enacct(t[DCOM]);
}

enacct(as)
char *as;
{
	struct stime timbuf;
	struct {
		char cname[14];
#ifndef	AUSAM
		char shf;
		char uid;
#endif
#ifdef	AUSAM
		int  shf;
		unsigned uid;
#endif
		long datet;	/* fix000 */
		long realt;	/* fix000 */
		long bcput;	/* fix000 */
		long bsyst;	/* fix000 */
	} tbuf;
	register i;
	register char *np, *s;

	s = as;
	times(&timbuf);
	timbuf.proct[0] = time();	/* fix000 */
	tbuf.realt = timbuf.proct[0] - timeb.proct[0];	/* fix000 */
	tbuf.bcput = timbuf.cputim - timeb.cputim;	/* fix000 */
	tbuf.bsyst = timbuf.systim - timeb.systim;	/* fix000 */
	do {
		np = s;
		while (*s != '\0' && *s != '/')
			s++;
	} while (*s++ != '\0');
	for (i=0; i<14; i++) {
		tbuf.cname[i] = *np;
		if (*np)
			np++;
	}
	tbuf.datet = timbuf.proct[0];	/* fix000 */
	tbuf.uid = uid;
	tbuf.shf = 0;
	if (promp==0)
		tbuf.shf = 1;
	seek(acctf, 0, 2);
	write(acctf, &tbuf, sizeof(tbuf));
}
#endif

#ifdef	TIME_OUT
clock()
{
}
#endif

#ifdef	KILL_IT

/*
 * kill .. an updated version with ranges implemented.
 *
 */

char nonnum[] "non-numeric arg\n";
int sig 9 ;

killer( args )
register char **args;
{
	register pid,upper;

	sig = 9;	/* re-init */
	while( *args )
		if( **args == '-' )
			{
			*args =+1 ;
			sig = atoi(args);
			if(**args++) prs(nonnum);
			if(sig >= 20)
				prs("signal too large\n");
			}
		else {
			pid = atoi(args);
			if(**args == '-') {
				*args =+1 ;
				upper = atoi(args);
				if(**args++) prs(nonnum);
				if(pid > upper)
					prs("lower limit > upper limit\n");
				for(;pid <= upper;pid++) killit(pid,0);
			}
			else	if(**args++) prs(nonnum);
				else killit(pid,1);
		}
}

killit(pid,flag)
int pid,flag;
{
	if((kill(pid,sig) == -1) && flag) {
		putd( pid );
		prs( ": not found\n" );
	}
}

atoi(s)
char **s;
{
	register char *c;
	register i;

	c = *s;
	i = 0;
	while((*c <= '9') && (*c >= '0')) i = i*10 + *c++ - '0';
	*s = c;
	return(i);
}

putd( n )
register n;
{
	register m;

	if( (m=n/10) ) putd( m );
	putc( n%10 + '0' );
}
#endif	KILL_IT
