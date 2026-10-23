#
#include	<defines.h>
#include	<param.h>
#include	<user.h>
#define		RAW	040
#define		TABSIZ	12*600
#define		NOOFBP	9
#define		EOT	04
#define		r0	0775
#define		r1	0773
#define		r2	0764
#define		r3	0765
#define		r4	0766
#define		r5	0767
#define		sp	0772
#define		pc	0776
#define		ps	0777
struct	list {	int badr;	/* break point address		 */
		int bcount;	/* break point repeat count	*/
		int bcnt;	/* break point contents		 */
	     }	bkpt[NOOFBP];
char *regs[] { "r0","r1","r2","r3","r4","r5","sp","pc","ps" };
int 	r[]  {  r0 , r1 , r2 , r3 , r4 , r5 , sp , pc , ps  };
int	ttyarg[3];
int	ttymod;			/* teletype mode		*/
int	pid;
int	savebp;			/* saver for last break point */
char	*cfil	"core";		/* core file name */
int	corfil;			/* core file descriptor */
int	curfil;			/* current file descriptor */
int	orgfil;			/* original file descriptor */
char	*sfil;			/* name of the debugged file */
int	symfil;			/* it's descriptor */
int	namfil;			/* file hoped to contain a symbol table */
int	txtsiz;			/* text segment size */
int	symsiz;			/* symbol table size */
int	symcntr;		/* symbol counter */
int	bytwrd;			/* indicates how many bytes are open */
int	valfnd;			/* value found flag */
int	svalfnd;		/* it's saver */
int	symfnd;			/* symbolic reference flag */
int	adrval;			/* result of expression */
int	tadrval;		/* temporary result holder */
int	scflg;			/* semicolon flag */
int	dot;			/* current value of dot */
char	ch;			/* current character */
char	cmode	'o';		/* current output mode */
int	header;			/* size of the header */
char	cfdes;			/* current output file descriptor */
int	sign;			/* signal number holder */
int	odbfil;			/* debugged file descriptor */
char	linbuf[512];		/* output line buffer */
char	line[128];		/* input line buffer */
int	corbuf[512];		/* buffer for core image header	*/
char	symbuf[12];		/* symbol table entry holders   */
char	symbol[12];
char	stable[TABSIZ];
char	*lp, *lbp;		/* pointers */
char	**users corbuf;
int	insyms, *tblptr;


main(argc,argv)
char **argv;
	{
	register n,tadr;
	int setraw();
	init(argc,argv);
	gtty(0, ttyarg);
	ttymod = ttyarg[2];
	setty(RAW);
	signal(4,setraw);
	setexit();
	signal(2,setraw);			/* catch interrupts */
	if (ttyarg[2] == ttymod)
		setty(RAW);
	clear();				/* clear variables */
	lbp = linbuf;
	while (1) {				/* loop until exit */
		rdline();
		if (rch() == EOT)
				{
				if (pid)
					ptrace(8, pid, 0, 0);
				ex();
				}
		tadrval = svalfnd = 0;
		valfnd = expr();
		tadr = adrval;
		if (!bytwrd) orgfil=curfil;	/* if nothing is open */
		if ( ch == ',' )
			{
			if ( (n = getarg()) >= tadr )
				{
				if ( match(ch,"/\\") )
					{
					bytwrd = (ch == '/' ? 2 : 1);
					getnfil();
					}
				else
					error();
				dot = tadr;
					while (dot<=n)
						{
						openxt();
						putlp("\n");
						dot =+ bytwrd;
						if (lbp>linbuf+512)
							linput();
						}
				linput();
				setraw();
				}
			else
				error();
		   }
		else
			{
			cmndch();
			linput();
			}
		}
  	}


init(argc,argv)	
char **argv;
	{
	register char *p, *q, *p1;
	int inbuf[10];
	if (argc >= 2) sfil= argv[1];
	else
		{
		printf("file name required\n");
		exit();
		}
	if ((corfil=open(cfil,0)) >= 0)		/* if core file exists */
		{
		read(corfil, corbuf, 1024);
		sign = corbuf[0].u_arg[0]&017;
		}
	if ((symfil=namfil=open(sfil,2)) < 0)
		finmsg("open error");
	if ( argc > 2 )
		{
		if ( (namfil = open(argv[2],0)) > 0 )
				read(namfil,inbuf,020);
		}
	else
		read(symfil,inbuf,020);			/* read-in the header */
	if (inbuf[0]==0410 || inbuf[0]==0407)	/* if executable */
		{
		header = 020;
		txtsiz=inbuf[1]+inbuf[2];
		symsiz=inbuf[4];
		if ( !inbuf[7] )
			txtsiz =+ txtsiz;
		txtsiz=+  020;  	/* allow for the header */
		}
	seek(namfil, txtsiz, 0);
	insyms = read(namfil, stable, TABSIZ);
	if (insyms > 0)
		txtsiz =+ insyms;
	for (p=stable; p<= &stable[insyms]; p=+ 12)
		{
		if ( *p == '_' || *p == '~' )
			{
			p1 = p;
			q = p+1;
			while (p1 < p+7)
				*p1++ = *q++;
			*p1 = '\0';
		}
	}
	if ( cmpst(sfil,cfil) )
		{
		odbfil = corfil;
		header = 1024;
		}
	else
		odbfil = symfil;
	cbplist();
	}

finmsg(s)
	{
	printf("%s %s\n",s,sfil);
	exit();
}

clear()	{
	write(1,"\r\n*",3);
	cfdes = 1;
	orgfil=scflg=symfnd=bytwrd=0;/* clear things */
	}



error() {
	write(1,"\n\r?",3);
	reset();
	}


setty(mode)
	{
	if (mode == RAW)
		{
		ttyarg[2] =| RAW;
		ttyarg[2] =& ~020;
	}
	else
		ttyarg[2] = ttymod;
	stty(0,ttyarg);
}

ex()
	{
	setty(1);
	printf("\n");
	exit();
	}

getarg()
	{
	register tad1;
	tad1 = adrval;
	rch();
	if ( !expr() )
		return(tad1);
	return(adrval);
	}

cmpst(s1,s2)
char *s1,*s2;
	{
	while ( *s1++ == *s2 )
		if ( *s2++ == '\0' )
			return(1);
	return(0);
}
