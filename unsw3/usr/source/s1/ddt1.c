#
/*

	Dynamic Debugging Tool
		Bill Allen
			Naval Postgraduate School
			March, 1975

			Revised for UNIX version 6 October, 1975

*/

#include	<defines.h>
#include <param.h>
#include <user.h>

extern pinstr();
extern prtop();

char *regname[] {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"sp",
	"pc"
};

int regloc[] {
	-3,		/* r0 */
	-5,		/* r1 */
	-12,	/* r2 */
	-11,	/* r3 */
	-10,	/* r4 */
	-9,		/* r5 */
	-6,		/* sp */
	-2,		/* pc */
	-1		/* ps */
};


struct sfregs {
	int	junk[2];
	int	fpsr;
	float	sfr[6];
};

struct lfregs {
	int	junk[2];
	int	fpsr;
	double	lfr[6];
};

int	frnames[] { 0, 3, 4, 5, 1, 2 };

extern int locsr5;
char *sbrk();
char *symbuf;
int *symptr;
int	fcore	-1;
int	fsym	-1;
int	wcore	-1;
int	wsym	-1;
int fdout;
char	*gargv[10];
int	pid;
int	coroffset;
int	symoff;
char	*lp;
int	errflg;
int	symlen;
int	symct;
char	symbol[8];
int	symflg;
int	symval;
char	ssymbol[8];
int	ssymflg;
int	ssymval;
char	line[128];
int	regbuf[512];
char	**uregs	&regbuf[512];
char	*rtsize;
int	ps -1;
int	pc -2;
int	r0 -3;
int	r1 -5;
int	sp -6;
int	r5 -9;
int	r4 -10;
int	r3 -11;
int	r2 -12;
int	dot;
int	tdot;
int	dotinc 2;
int	lastcom '/';
int	lastype	'i';
char	*symfil	"a.out";
char	*corfil	"core";
int	callist[50];
int	*callp	&callist[0];
int	leffect;		/* last effective address */
int	headsize;	/* length of file header */
int	tsize;		/* length of text segment */
int	dsize;		/* length of data segment */
int	ssize;		/* length of bss segment */
int	gargc;		/* number of arguments from shell */
int	sdot;		/* symbolic operand temporary dot */
int	ldot;
int	elastype	'o';	/* = display mode */
int signo;		/* signal type */
int	adrfg;	/* set if this command had an address */
int	status;
#define BREAK 3		/* system call breakpoint */
#define BRKLEN 20
#define CONDLEN 82
struct {
	int value;		/* old value of breakpoint */
	int addr;		/* address of breakpoint */
	char cond[CONDLEN];	/* breakpoint condition */
} brktab[BRKLEN];
int brktx -1;			/* breakpoint table index */

char	*signals[] {
	"",
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BTP",
	"IOT",
	"EMT",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"",
	"",
	"",
};
extern int tempbra[];	/* tamporary breakpoint address */

extern getspsym();
extern putspsym();
extern ssval[];
extern char *ssname[];
extern initssv();


#define	RUSER	1
#define	RIUSER	2
#define	WUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	SETTRC	0
#define	CONTIN	7
#define	EXIT	8

#define SSR0	0
#define SSR1	1
#define SSR2	2
#define SSR3	3
#define SSR4	4
#define SSR5	5
#define SSR6	6
#define SSR7	7
#define SSSP	6
#define SSPC	7
#define SSPS	8
#define SS8	9
#define SS9	10
#define SSL	11
#define SSM	12
#define SSQ	13
#define SSA	14
#define SSD	15
#define SSRG	16

main(argc, argv)
char **argv;
{
	int onintr();
	int endit();
	register cpid, rpid;
	int cstatus;

	gargc = argc;
	if(gargc > 1)
		symfil = argv[1];
	if(gargc > 2)
		corfil = argv[2];
	if((fsym = open(symfil,0))<0) {
		printf("Unable to open %s\n",symfil);
		exit();
	}
	wsym = open(symfil,1);
	if(gargc != 2) {	/* try for core file */
		fcore = open(corfil,0);
		wcore = open(corfil,1);
	}
	initfsym();		/* init symbolic file sizes */
	if((symbuf=sbrk(symlen+2)) == -1) {
		printf("Symbol table too large\n");
		exit();
	}
	symbuf =& 0177776;	/* word address */
	seek(fsym,symoff,0);
	if(read(fsym,symbuf,symlen) != symlen)
		printf("Can't read symbol table\n");
	if(fcore>=0 && gargc<=3)	/* have a core file */
		initfcor();	/* initialize the known symbols */
	if(gargc>3)		/* file is not a core image */
		headsize = 0;
	initssv();		/* set default special symbol values */
	for(dot=0; dot<BRKLEN; dot++)
		brktab[dot].addr = -1;	/* all breakpoints are unused */
	dot = 0;
	if(symlook("savr5\0\0\0"))
		locsr5 = ssymval;
	if(fcore>0)
		setstack();
	setexit();
	signal(2, onintr);	/* rubout */
	signal(1,endit);	/* hangup (EOT) */
	putchar('\n');	/* ready to go */
loop:
	if (errflg) {
		printf("?\t");
		errflg = 0;
	}
	lp = line;
	while ((*lp = getchar()) != '\n') {
		if(lp==line && *lp=='!'){  /* call the shell */
			if((cpid=fork()) == 0) {
				execl("/bin/sh","sh","-t",0);
				exit();
			}
			while((rpid=wait(&cstatus)) != cpid && rpid != -1);
			printf("!\n");
			goto loop;
		}
		if (*lp++ == '\0')
			endit();
	}
	lp = line;
	command();
	goto loop;
}

command()
{
	register  n;

	adrfg = expr();
	if (errflg)
		return;
	n = getcnt();
	if (lastcom=='$')
		lastcom = '/';
	if (*lp == '\n') {
		lastcom = '/';
		if (!adrfg)
			dot =+ dotinc;
	} else
		lastcom = *lp++;
	if (adrfg) {
		ldot = dot;
		dot = tdot;
	}
	while(n) {
		scommand(n);
		if (*lp != '\n') {
			printf("** unrecognized command or argument\n");
			return;
		}
		if (--n){
			dot =+ dotinc;
			putchar('\n');
		}
	}
}

scommand(n)
{
	register char *p1;
	register w, c;
	double fw;
	struct { int i[4]; };

	if(lastcom == '>') {		/* follow the path */
		dot = leffect;
		lastcom = '/';
	}

	switch(lastcom) {

	case '/':
		psym(dot);		/* print address symbolically */
		putchar('\t');
		w = cget(dot);
		ssval[SSQ] = w;	/* set :q */
		leffect = w;	/* set default last effective address */
		if(*lp != '\n')
			lastype = *lp++;
		switch(lastype) {

		case 'o':	/* word octal */
			printf("%.1o\t", w);
			dotinc = 2;
			return;

		case 'x':	/* word hexidecmal */
			printf("%.x\t", w);
			dotinc = 2;
			return;

		case 'd':	/* word decimal */
			printf("%d\t", w);
			dotinc = 2;
			return;

		case 'l':	/* word unsigned decimal */
			printf("%l\t", w);
			dotinc = 2;
			return;

		case 'f':	/* floating point */
			fw = 0.0;
			fw.i[0] = w;
			fw.i[1] = cget(dot+2);
			printf("%e\t", fw);
			dotinc = 4;
			return;

		case 'e':	/* double precision floating point */
			fw.i[0] = w;
			fw.i[1] = cget(dot+2);
			fw.i[2] = cget(dot+4);
			fw.i[3] = cget(dot+6);
			printf("%e\t", fw);
			dotinc = 8;
			return;

		case 'i':	/* instruction in assembler format */
			pinstr();	/* print instruction symbolically */
			return;

		case '&':
			c = ssval[SSL];	/* save low symbol address */
			ssval[SSL] = 0;	/* use all symbols */
			psym(w);	/* print as symbol and offset */
			ssval[SSL] = c;
			if (errflg)
				reset();
			putchar('\t');
			return;

		case 'b':	/* byte in octal */
			printf("%.1o\t", w & 0377);
			dotinc = 1;
			return;
	
		case '\'':	/* byte in ASCII */
			printc(w & 0377);
			dotinc = 1;
			putchar('\t');
			return;

		case '"':	/* word in ASCII */
			printc(w&0377);
			printc((w&0177400)>>8);
			dotinc = 2;
			putchar('\t');
			return;
	
		case 's':	/* string in ASCII */
			w = dot;
			while(c = cget(w++)&0377)
				printc(c);
			putchar('\t');
			dotinc = w-dot;
			return;
		}
		printf("** unrecognized display mode\n");
		return;

	case '=':
		ssval[SSQ] = tdot;	/* set :q value */
		dot = ldot;	/* restore dot */
		lastcom = '/';	/* this was an aside */
		if(*lp != '\n')
			elastype = *lp++;
		switch(elastype){	/* display mode */

		case 'o':	/* word octal */
			printf("%.1o\t",tdot);
			return;

		case 'x':	/* word hexidecimal */
			printf("%.x\t",tdot);
			return;

		case 'd':	/* word decimal */
			printf("%d\t",tdot);
			return;

		case 'l':	/* word unsigned decimal */
			printf("%l\t",tdot);
			return;

		case 'b':	/* byte decimal */
			printf("%o\t",tdot&0377);
			return;

		case '&':	/* address */
			c = ssval[SSL];	/* save low symbol address */
			ssval[SSL] = 0;	/* use all symbols */
			psym(tdot);	/* print as symbol and offset */
			ssval[SSL] = c;
			if (errflg)
				reset();
			putchar('\t');
			return;

		case '\'':	/* byte ASCII */
			putchar(tdot&0377);
			putchar('\t');
			return;

		case '"':	/* word ASCII */
			putchar(tdot&0377);
			putchar((tdot&0177400)>>8);
			putchar('\t');
			return;

		}
		printf("** unrecognized display mode\n");
		return;

	case '!':	/* patch the file */
		if(!adrfg){		/* no value to store */
			printf("** no address for store\n");
			return;
		}
		dot = ldot;		/* reset dot */
		if(*lp == ':'){		/* store value in special symbol */
			lp++;
			putspsym(tdot);		/* set sp symbol to value in tdot */
			return;
		}
		cfput(tdot);	/* put tdot in file at address dot */
		return;


	case '$':
		prtermst();		/* print termination status */
		printtrace();
		return;

	case '?':	/* display termination status */
		prtermst();
		return;

	case '^':	/* preceeding cell */
		dot =- 2;
		lastcom = '/';	/* display word value */
		scommand(n);
		return;

	case ';':	/* long command */
		lastcom = '/';	/* display */
		switch(*lp++){

		case 'g':	/* begin file execution */
			w = 1;	/* only once */
			if(pid>0){	/* have a child at breakpoint */
				ptrace(EXIT,pid,0,0);	/* kill old child */
				pid = 0;
			}
			if(fsym<0) {
				printf("no symbolic file to execute\n");
				return;
			}
			gargv[0] = symfil;	/* first name is file name */
			w = 1;
loop:
			while(*lp == ' ') *lp++ = '\0';
			if(*lp == '\n') {
				*lp = '\0';		/* end of string */
				gargv[w] = 0;	/* end of argument list */
			}
			else {
				gargv[w++] = lp;	/* form argument pointers */
				while(*lp != ' ' && *lp != '\n') lp++;	/* skip over args */
				goto loop;
			}
			while((pid = fork())<0);
			if(pid==0){			/* child */
				ptrace(SETTRC,0,0,0);		/* expect trace */
				signal(SIGINT,0);
				signal(SIGINS,0);
				execv(symfil,gargv);		/* execute the file */
				printf("Can't execute %s\n",symfil);
				exit();					/* just in case */
			}
			else {		/* parent */
				if(fcore>=0){	/* core file open */
					rmcore();	/* close core files */
				}
			}
			w = 0;
			connotb();
			return;

		case 'p':	/* proceed from  breakpoint */
			if(adrfg)
				w = dot;	/* repetition factor */
			else
				w = 1;
			while(w--) {
				proceed();
			}
			return;

		case 's':	/* single instruction step */
			lastcom = '/';
			lastype = 'i';
			*lp = '\n';
			if(adrfg)
				w = dot;	/* repetition factor */
			else
				w = 1;
			while(w--){		/* one instruction each time */
				stempbpt(0);	/* set temp brkpt */
				restcore();
				ptrace(CONTIN,pid,0,0);
				bpwait();
				rtempbpt();
				dot = (ssval[SSPC] =+ 2);
				scommand(1);	/* print the instruction */
				if(w) putchar('\n');
			}
			return;

		case '.':	/* set temp bpt and continue */
			if(adrfg==0) {
				printf("*** no address specified\n");
				return;
			}
			w = dot;
			setbpt(w);	/* set breakpoint */
			proceed0();	/*  ;p */
			clbkpt(w);	/* clear breakpoint */
			dot = ssval[SSPC];
			return;

		case '=':	/* search equal */
			for(dot=ssval[SS8];dot<=ssval[SS9];dot =+ 2){
				if((cget(dot)&(ssval[SSM])) == tdot){
					scommand(n);	/* display it */
					putchar('\n');
				}
			}
			putchar('\n');
			return;

		case '#':	/* search not equal */
			for(dot=ssval[SS8];dot<=ssval[SS9];dot =+ 2){
				if((cget(dot)&(ssval[SSM])) != tdot){
					scommand(n);	/* display it */
					putchar('\n');
				}
			}
			putchar('\n');
			return;

		case 'b':	/* set breakpoint */
			if(adrfg)
				setbpt(dot);
			else
				printf("*** no address specfied\n");
			return;

		case 'c':		/* clear a breakpoint */
			clbkpt(dot);	/* clear it or all */
			return;

		case 'd':		/* display breakpoints */
			if(brktx<0) goto dout;
			for(c=0;c<=brktx;c++){
				if(brktab[c].addr != -1){		/* valid breakpoint */
					psym(brktab[c].addr);	/* print breakpoint address */
					putchar('\t');
					if(brktab[c].cond[0] != '\0')	/* print conditional */
						printf("%s",brktab[c].cond);
					else
						putchar('\n');
					ssval[SSQ] = brktab[c].addr;	/* set :q */
				}
			}
dout:
			putchar('\n');
			return;

		case 'r':		/* display registers in octal and symbollically */
			for(c=0;c<=7;c++){
				printf("%s\t\t\t%.1o\t\t\t",ssname[c],ssval[c]);
				psym(ssval[c]);
				putchar('\n');
			}
			return;

		case 'e':	/* print floating regs */
			printfregs();
			return;

		case 'f':	/* stop */
			endit();

		case 'w':	/* write copy of breakpointed pgm */
			while(*lp==' ')
				lp++;
			p1 = lp;
			while(*lp != '\n')
				lp++;
			*lp = '\0';			/* end of string */
			if((fdout=creat(p1,0777))<0){
				printf("**unable to open output file %s\n",line);
				return;
			}
			if(fcore>0) {
				w = (headsize+tsize+dsize+ssize+511)/512;
				seek(fcore,coroffset,3);
				while(w--) {
					read(fcore,regbuf,512);
					write(fdout,regbuf,512);
				}
				close(fdout);
				initfcor();
				putchar('\n');
				*lp = '\n';
				return;
			}
			printf("**no core file to copy\n");
			return;

		}
	}
	printf("** unrecognized command\n");
}

rmcore()
{

	close(fcore);
	close(wcore);
	fcore = -1;
	wcore = -1;
	initfsym();
}

/* test for a valid breakpoint address */
notbpt()
{

	register i,j;

	j=ssval[SSPC];
	for(i=0;i<=brktx; i++)
		if(brktab[i].addr==j)
			return(0);

	for(i=0; i<2; i++)
		if(tempbra[i]==j)
			return(0);
	return(1);				/* not valid */
}

bpwait()
{
	extern int onintr();
	register w;
	int stat;

loop:
	signal(SIGINT, 1);
	while ((w = wait(&stat))!=pid && w != -1);
	signal(SIGINT, onintr);
	if (w == -1) {
		ptrace(EXIT, pid, 0, 0);
		pid = 0;
		printf("Wait error\n");
		reset();
	}
	if ((stat & 0377) != 0177) {
		if (signo = stat&0177)
			printf("%s\n", signals[signo]);
		printf("Process terminated.\n");
		if (pid == w) {
			pid = 0;
			reset();
		}
		goto loop;
	}
	signo = stat>>8;
	initfmem();
	if (signo!=SIGTRC) {
		printf("%s\n", signals[signo]);
		reset();
	}
	setstack();
}

/*  set a breakpoint */
setbpt(adr)
{
	register c;

	dot = adr;
	for(c=0;c<=brktx;c++)
		if(brktab[c].addr==dot){	/* already got it */
			getcond(c);		/* may want to change conditional */
			return;
		}
	for(c=0;c<=brktx;c++)
		if(brktab[c].addr == -1){	/* not used */
bptstr:
			brktab[c].value = cget(dot);	/* remember old value */
			brktab[c].addr = dot;
			cfput(BREAK);			/* insert system breakpoint */
			if(errflg){
				printf("**Unable to set breakpoint\n");
				brktab[c].addr = -1;	/* no breakpoint */
			}
			getcond(c);				/* get breakpoint condition */
			dot = ldot;				/* reset dot */
			return;
		}
	if(++brktx > BRKLEN){	/* no more breakpoints */
		printf("?? too many breakpoints\n");
		return;
	}
	c = ++brktx;
	goto bptstr;
}
/*  proceed from a breakpoint */
proceed()
{
	if(pid<=0) {
		printf("no process to continue\n");
		return;
	}
	if(stempbpt(1)) {;		/* set temporary breakpoint maybe */
		restcore();		/* restore special symbol values */
		ptrace(CONTIN,pid,0,0);
		bpwait();
		rtempbpt();		/* remove temporary breakpoints */
		ssval[SSPC] =+ 2;	/* trapped by T bit */
	}
	restcore();		/* reset PC */
	ptrace(CONTIN,pid,0,0);
	connotb();
}

/*  continue not from breakpoint */
connotb()
{
	bpwait();
	if(!condbpt()) {			/* conditinal? */
		proceed();
		return;
	}
	if(notbpt()){				/* not valid breakpoint */
		printf("breakpoint error %o\n",ssval[SSPC]);
		endit();
	}
	dot = ssval[SSPC];			/* location of breakpoint */
	lastcom = '/';				/* display */
	lastype = 'i';
	*lp = '\n';
	scommand(1);				/* display breakpoint instr */
}


printfregs()
{
	register i;
	double f;

	if(pid)
		for(i=2; i<25; i++)
			regbuf[i] = ptrace(RUREGS,pid,2*i,0);	/* read fregs */
	printf("fpsr	%.1o\n", regbuf[0].fpsr);
	for (i=0; i<6; i++) {
		if (regbuf[0].fpsr&0200)	/* long mode */
			f = regbuf[0].lfr[frnames[i]];
		else
			f = regbuf[0].sfr[frnames[i]];
		printf("fr%d	%e\n", i, f);
	}
}

getchar()
{

	int lastc;

	if(read(0,&lastc,1) <= 0)
		return(0);
	return(lastc&0177);
}

/* get the condition for this breakpoint */
getcond(aidx)
{

	register idx,i;

	idx=aidx;	/* breakpoint table index */
	if(*lp == '\n') {	/* unconditional */
		brktab[idx].cond[0] = '\0';
		return;
	}
	while(*lp == 'b') lp++;		/* ignore blanks */
	i=0;
	while(*lp!='\n' && i<CONDLEN-2)
		brktab[idx].cond[i++] = *lp++;	/* save condition source */
	if(*lp != '\n') {
		printf("***condition more than %d bytes long ignored\n",CONDLEN-2);
		brktab[idx].cond[0] = '\0';
		return;
	}
	brktab[idx].cond[i++] = '\n';
	brktab[idx].cond[i++] = '\0';
}
