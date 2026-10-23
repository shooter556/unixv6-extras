#

# include	"mas0.h"



char	*symtmp, *fbtmp;
int	sfile, fbfile;

char	*itmp, *atmp, *etmp;
int	afile, efile, ifile;
int	acount, icount;

char	*ltmp;

extern	struct aform abuf[];
extern	struct iform ibuf[];
struct	aform *abptr;
struct	iform *iptr;

extern	char inbuf[];
char	*fins;
int	tbuf[];				/* mod12 */
extern	struct symbol symtab[];
extern	hashtab[];
extern	mnemlook();

struct	evalx r;

extern	struct fb fbtable[];

struct	expr exptab[10];


struct	segment segopt;



extern	curfb[];
int	nextfb;
int	lc,errc,err2;

int	pc,pc_def;
int	numsym;
char	tch;
int	lab;
int	accvalid;
int	gargc;
char	**gargv;
extern	char symbuf[];

mexit()				/* mod002 */
{
	signal(2,1);
	signal(3,1);
	unlink(ltmp);
	unlink(symtmp);
	unlink(fbtmp);
	unlink(itmp);
	unlink(atmp);
	unlink(etmp);
	exit();
}

main(argc, argv)
char **argv;
{
	register char c;
	register *n, l;		/* mod005 */

	gargc = argc;
	gargv = argv;

	signal(2,mexit);
	signal(3,mexit);	/* mod002 */

	n = inbuf; *n = -1;		/* so that i-o channels not closed */	/* mod007 */
	if (newfile() == 0) exit();

	ltmp = "/tmp/motma.5";	/* lock file */
	c = 'a';
	while (creat(ltmp,0000) == -1)
		if ((*(ltmp+9) = ++c) > 'z') help();

	symtmp = "/tmp/motmx.0"; *(symtmp+9) = c;
	fbtmp = "/tmp/motmx.1"; *(fbtmp+9) = c;
	itmp = "/tmp/motmx.2"; *(itmp+9) = c;
	atmp = "/tmp/motmx.3"; *(atmp+9) = c;
	etmp = "/tmp/motmx.4"; *(etmp+9) = c;

	if ((afile = creat(atmp,0600)) == -1 ||		/* NOTE : ONLY OPENED FOR WRITING */
	     (efile = creat(etmp,0600)) == -1 ||
	     (ifile = creat(itmp,0600)) == -1)  help();

	close(afile); close(efile);
	if ((afile = open(atmp,2)) == -1 ||		/* now readable too */
	     (efile = open(etmp,2)) == -1)  help();

	abptr = abuf; iptr = ibuf;

	numsym = 1;
	pc_def = ABS;
	nextfb = 20;

	for (l=0; l<20; l++) curfb[l] = l;		/* mod005 */

	if (getch() == '#')		/* mod005 */
	{
		lc++;
		while ((c = getch()) == SP || c == TAB);
		if (getsym(c) != 3 || mnemlook() != o_SEG || tch != SP && tch != TAB) segerr();
		while (((c = getch()) == SP) || (c == TAB));
		if ((l = getsym(c)) == 0)
		{
			if (tch != MINUS || (l = getsym(0)) == 0) segerr();
			segopt.segtype = 2;
		}
		else segopt.segtype = 1;

		if (l != 1) segerr();
		c = symbuf[0];
		if (DIGIT(c)) segerr();
		segopt.segchar = c;
		segopt.segcount++;
		getcomm();
	}
	else
	{

		*(n+1) =+ 1;		/* mod005 */
		*(n+2) =- 1;
	}

	for(;;)			/* start of main loop when NL read */
	{
		setexit();
		lc++;
		lab = 0;
		if ((c = getch()) == EOF)
		{
			if (newfile()) continue;
			break;
		}

		switch (c)
		{
	    case NL:	continue;

	    case SP:
	    case TAB:	break;

	    case STAR:	tch = c;
			getnl();

	    default:	getlabel(c);
		}
		while (((c = getch()) == SP) || (c == TAB));
		if (c == NL) continue;
		if (c == EOF) eoferr();

		getmnem(c);	/* read operator field */
		getcomm();	/* reset */
	}

	pass1a();		/* pass1 completed */
	newif(i_REL);

	close(afile); unlink(atmp);
	close(efile); unlink(etmp);

	if (err2) mexit();	/* don't run pass2 if errors */	/* mod002 */

	write(ifile, ibuf, icount<<2);		/* write rest of long form */
	close(ifile);

	if ((sfile = creat(symtmp,0600)) == -1) help();
	if ((fbfile = creat(fbtmp,0600)) == -1) help();
	write(fbfile, fbtable, nextfb<<2 );
	write(sfile, hashtab, 256);
	write(sfile, symtab, numsym * sizeof symtab[0]);	/* mod006 */
	argv[argc] = 0;
	argv[0][0] = *(ltmp+9);
	execv("/lib/mas2", argv);
	printf("/lib/mas2 has gone\n");
	mexit();			/* mod002 */
}



newfile()
{
	register *ip, *tp;

	close(*(ip = inbuf));			/* mod005, mod011 */
	if (*(tp = tbuf))
	{
		read(*ip++ = *tp, inbuf+6, 512);
		*tp++ = 0;
		*ip++ = *tp++; *ip++ = *tp++;
		lc = *tp++; fins = *tp++;
		errc = 0;
		return(1);
	}
	if (--gargc)
	{
		fins = *++gargv;
		if (*fins == '-') return(newfile());
		if ((fopen(fins,inbuf)) == -1)	/* mod005 */
		{
			printf("Cannot open: %s\n",fins);
			return(newfile());
		}
		lc = 0;
		errc = 0;
		return(1);
	}
	return(0);
}



getlabel(ch)
char ch;
{
	register char c;
	register l;

	if ((l = getsym(c = ch)) == 0) syntax();
	if (DIGIT(c))
	{
		if (l != 1) syntax();
		deffb(c - '0');
	}
	else if (accsym(l,c)) error("acc symbol"); else defsym();

	if ((c = tch) == NL) reset();
	if (c != SP && c != TAB && c != COLON) syntax();
}



getmnem(ch)
char ch;
{
	register char c, acc;
	register n;

	if ((n = getsym(c = ch)) == 4)
	{
		if ((acc = accsym(1, symbuf[3])) == 0) syntax();
		symbuf[3] = 0;
	}
	else if (n != 3) syntax(); else acc = 0;

	n = mnemlook();
	if (acc && (n > o_PUSH)) syntax();

	switch (n)
	{
    case o_INH:		if (acc || (tch > SP)) syntax();
			pc++;
			return;

    case o_DUAL:	if (acc == 0) getacc();
			getimm(2);
			return;

    case o_STA:		if (acc == 0) getacc();
			getdirec(0);
			return;

    case o_AEI:		if (acc) pc++;
			else
			{
				accvalid++;
				getind(0);
				accvalid = 0;
			}
			return;

    case o_PUSH:	if (!acc) getacc();
			pc++;
			return;

    case o_IDEI:	getimm(3);	/* 16 bit immediate operand */
			return;

    case o_DEI:		getdirec(0);
			return;

    case o_JMPS:	getind(0);
			return;

    case o_BNCH:	getexpr(0);
			pc =+ 2;
			return;

    case -1:		syntax();

    default:		if (n > 50)
			{
				pseudop(n);
				return;
			}
			getexpr(0);
			if (n == o_JBRS)
			{
				outaform(a_JBRS);
				pc =+ 3;
			}
			else
			{
			 	outaform(a_JEQS);
				pc =+ 5;
			}
			pc_def = EST;
	}
}




outaform(at)
{
	register n;
	register struct expr *x;
	register struct aform *ab;

	ab = abptr;
	ab->a_pc = pc;
	ab->a_def = n = at;
	if (n >= a_EQU || n == a_JEQS || n == a_JBRS)
	{
		for (x = exptab; x->e_rator; x++);
		x++;
		write(efile, &exptab[0], (x-&exptab[0]) << 2);
	}

	abptr++;
	if (++acount == 128)
	{
		write(afile, abuf, 512);
		acount = 0;
		abptr = abuf;
	}
}



defsym()
{
	register struct symbol *sym;

	sym = &symtab[(lab = lookup())];
	if (sym->s_def) error("m*");
	else
	{
		sym->s_def = pc_def;
		sym->s_pc = pc;
	}
}



deffb(ind)
{
	register struct fb *fbp;
	register i, n;

	if (nextfb >= NFB-1) fberr();		/* mod008 */
	i = ind;
	n = curfb[i] = curfb[i+10];
	fbp = &fbtable[n];
	fbp->fb_pc = pc;
	fbp->fb_def = pc_def;
	n = curfb[i+10] = nextfb++;
	fbp = &fbtable[n];
	fbp->fb_lab = i;
}



iclerr()
{
	error("included file"); mexit();	/* mod011 */
}



segerr()
{
	error("seg"); mexit();		/* mod002 */
}



syntax()
{
	error("syntax"); getnl();
}



numerr()
{
	error("number"); getnl();
}


charerr()
{
	error("char constant"); getnl();
}


parerr()
{
	error("')' missing"); getnl();
}



eoferr()
{
	error("eof ?"); mexit();	/* mod002 */
}



help()
{
	printf("help: can't open temp files\n"); mexit();		/* mod002 */
}



error(s)
char *s;
{
	if (errc == 0) printf("%s:\n", fins);
	printf("%5.d\t%s\n", lc, s);
	errc++;
	err2++;
}


syserr()
{
	printf("assembler error\n"); mexit();		/* mod002 */
}



relerr()
{
	error("relocation"); mexit();		/* mod002 */
}


symtblerr()					/* mod008 */
{
	error("symbol table overflow"); mexit();
}


fberr()						/* mod008 */
{
	error("temp. symbol table overflow"); mexit();
}
