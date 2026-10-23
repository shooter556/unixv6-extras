#
/*

	Dynamic Debugging Tool
		Bill Allen
			Naval Postgraduate School
			March, 1975

*/

#include "optab.h"
#include "syscall.h"


#define	REG		0
#define	AUTOI	1
#define	AUTOD	2
#define	INDEX	3
#define	DOUBLE	0
#define	SINGLE	1
#define	SINGLW	2
#define	MULDIV	3
#define	BRANCH	4
#define	NOADDR	5
#define	FLTREV	6
#define	FLTNOR	7
#define	SPECL1	11
#define	SPECL2	12
#define	SPECL3	13
#define	SPECL4	14
#define	SPECL5	15
#define	SPECL6	16
#define	SPECL7	17
#define	SPECL8	18
#define	SPECL9	19
#define	FLTPNR	21
#define	FLTPRV	22

extern int dot,sdot,dotinc,errflg;
extern status;
extern char *lp;
extern char *lp;
extern int regbuf[];
extern char *regname[];
extern int coroffset,fcore,wcore,brktx;
#define BRKLEN 20
#define CONDLEN 82
struct {
	int value;		/* old value of breakpoint */
	int addr;		/* address of breakpoint */
	char cond[CONDLEN];	/* breakpoint condition */
} brktab[BRKLEN];
extern int callev, entpt[];

#define	BREAK	3
extern int r0;
extern int r1;
extern int r2;
extern int r3;
extern int r4;
extern int r5;
extern int sp;
extern int pc;
extern int ps;
extern char **uregs;

#define PC 7

#define NUMSS 19	/* number of special symbols */
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
#define	SSF	17	/* current function */
#define SSSL	18	/* lowest address on stack */

int ssval[NUMSS];		/* special symbol values */

char *ssname[]			/* special symbol names */
{
	"r0",		/* 0 user reg 0 */
	"r1",		/* 1 */
	"r2",		/* 2 */
	"r3",		/* 3 */
	"r4",		/* 4 */
	"r5",		/* 5 */
	"sp",		/* 6 */
	"pc",		/* 7 */
	"ps",		/* 8 users processor status */
	"8",		/* 9 search lower limit */
	"9",		/* 10 search upper limit */
	"l",		/* 11 low address for symbols */
	"m",		/* 12 search mask value */
	"q",		/* 13 last value typed to user */
	"a",			/* 14 have assembler symbols (no _) if set */
	"d",			/* 15 D space symbol offset */
	"r",				/* 16 symbol range value */
	"f",				/* current function for local symbols */
	"sl"				/* lowest address on stack */
};

char *termstr[] {	/* termination status strings */
	"normal exit",	/* 0 */
	"hangup",		/* 1 */
	"interrupt",	/* 2 */
	"quit",			/* 3 */
	"illegal instruction",	/* 4 */
	"trace trap",	/* 5 */
	"IOT instruction",	/* 6 */
	"EMT instruction",	/* 7 */
	"floating point exception",	/* 8 */
	"killed",		/* 9 */
	"bus error",	/* 10 */
	"segmentation violation",	/* 11 */
	"bad argument to system call"	/* 12 */
};

int pbkptc;
int tempbra[2],tempbrv[2];

/*  print an instruction in assembler format */

pinstr()
{

	register int i,instr;
	register int reg;

	sdot=dot+2;		/* next word address */
	dotinc = 2;
	instr=cget(dot);	/* instruction in binary */
	for(i=0;;i=+OPENT)	/* search opcode table */
		if((instr&~optab[i])==optab[i+1])
			goto found;
	printf("** opcode not recognized\n");
	return;

found:
	printf("%s",optab[i+3]);	/* print mneumonic */
	switch(optab[i+2]){			/* type of instruction */

		case SINGLE:
		case DOUBLE:
			if(instr&0100000)	/* byte instruction */
				putchar('b');
	}
	putchar('\t');
	switch(optab[i+2]){		/* type of instruction */

		case DOUBLE:
			prtop((instr&07700)>>6);	/* print first operand */
			putchar(',');

		case JUMP:
		case SINGLE:
		case SINGLW:
oneop:
			prtop(instr&077);		/* last operand */

		case NOADDR:
fin:
			putchar('\t');
			return;

		case BRANCHU:
		case BRANCH:
			instr =<< 8;
			instr =>> 7;	/* convert to signed offset */
			psym(dot+instr+2);		/* branch address */
			goto fin;

		case MULDIV:
			prtop(instr&077);	/* source operand */
			putchar(',');
			prtop((instr&0700)>>6);	/* dest register */
			goto fin;

		case SPECL0:		/* xor */
		case SPECL1:		/* jsr */
			reg = (instr&0700)>>6;
			printf("%s,",regname[reg]);
			goto oneop;

		case SPECL2:		/* rts */
			printf("%s\t",regname[instr&07]);
			return;

		case SPECL3:		/* cflg, sflg */
			if(instr&010)
				putchar('n');
			if(instr&04)
				putchar('z');
			if(instr&02)
				putchar('v');
			if(instr&01)
				putchar('c');
			goto fin;

		case SPECL5:	/* trap - system call */
			instr =& 0377;
			if(instr<=63){	/* valid UNIX system call */
				printf("%.8s\t",syscall[instr]);
				return;
			}

		case SPECL7:	/* mark */
		case SPECL4:	/* emt */
			printf("%.1o\t",instr&0377);
			return;

		case SPECL8:		/* sob */
			reg = (instr&0700)>>6;
			printf("%s,",regname[reg]);
			psym(sdot-((instr&077)<<1));
			goto fin;

		case SPECL9:		/* spl */
			printf("%o\t",instr&07);
			return;

		case SPECL6:	/* octal */
			printf("%.1o\t",instr);
			return;

		case FLTREV:	/* floating point reverse format */
			prfpop(instr&077);
fprev:
			putchar(',');
			prfpop((instr&0300)>>6);
			putchar('\t');
			return;

		case FLTNOR:	/* fp normal format */
			prfpop((instr&0300)>>6);
			putchar(',');
			prfpop(instr&077);
			putchar('\t');
			return;

		case FLTPNR:
			prfpop((instr&0300)>>6);
			putchar(',');
			prtop(instr&077);
			putchar('\t');
			return;

		case FLTPRV:
			prtop(instr&077);
			goto fprev;

	}
	printf("?\t");
	return;
}

/* print a floating point operand */

prfpop(type)
{

	register int reg,mode;

	reg = type&07;
	mode = (type&070)>>3;
	if(mode==0){	/* fp accumulator */
		printf("fr%o",reg);
		return;
	}
	prtop(type);
}

/*  print an operand symbolically */

prtop(adrtype)
{

	register int reg,mode,defer;

	reg = adrtype&07;
	mode = (adrtype&060)>>4;
	defer = adrtype&010;
	if(reg != PC){	/* not pc relative */
nopcrel:
		if(defer)	/* deferred mode addressing */
			putchar('*');
		switch(mode){

			case REG:
				printf("%s",regname[reg]);
				return;

			case AUTOI:
				printf("(%s)+",regname[reg]);
				return;

			case AUTOD:
				printf("-(%s)",regname[reg]);
				return;

			case INDEX:
				if(reg == 5)		/* probably local symbol */
					plocsym(cget(sdot));
				else {
					psym(cget(sdot));	/* print index value */
					printf("(%s)",regname[reg]);
				}
incrtn:
				sdot =+ 2;
				dotinc =+ 2;
				return;
		}
	}
	else {		/* pc relative addressing */
		switch(mode){

			case REG:
			case AUTOD:
				goto nopcrel;		/* no special case */

			case AUTOI:
				if(defer){		/* absolute address */
					printf("*$");
					psym(cget(sdot));
					goto incrtn;
				}
				printf("$%o",cget(sdot));	/* immediate data */
				goto incrtn;

			case INDEX:
				if(defer)		/* relative deferred */
					putchar('*');
				psym(cget(sdot)+sdot+2);
				goto incrtn;

		}
	}
}

/* get the value of a special symbol  */

getspsym()
{

	return(ssval[spsymidx()]);
}

/* store a value into a special symbol */

putspsym(value)
{
	register t1,t2;

	t1 = spsymidx();
	if(t1==SSF) {	/* current function */
		for(t2=0; t2<callev; t2++)	/* make sure its active */
			if(entpt[t2] == value)
				goto putit;
		printf("*** not an active function\n");
		return;
	}
putit:
	ssval[t1] = value;
	return;
}

/* return the index in ssval of a special symbol. */
/*   the special symbol name is pointed to by lp */

spsymidx()
{

	register int i;
	register char *p,*tlp;

	for(i=0;i<NUMSS;i++){
		tlp = lp;	/* points to special symbol name */
		p = ssname[i];	/* list of correct names */
		while(*p){
			if(*p++ != *tlp++)	/* not this one */
				goto nextss;
		}
		lp = tlp;	/* point past name */
		return(i);	/* found it */
nextss:
		continue;
	}
	printf("** no such special symbol name\n");
	return(0);
}

/* initialize te default special symbol values */

initssv()
{

	ssval[SSL] = 0100;	/* low symbolic address */
	ssval[SS9] = 020000;	/* high search address */
	ssval[SSM] = 0177777;	/* search mask */
	ssval[SSRG] = 01000;	/* symbol range */
	ssval[SSSL] = 0177000;	/* lowest address on stack */
}

/*  set a temporary breakpoint */
/*   flag == 0  => called from ;s */
/*   flag == 1  => called from ;p */
stempbpt(flag)
{

	register int tpc,i,j;

/*  if pc points to a permanent breakpoint, clear it temporarily */

	pbkptc = 0;
	tpc = ssval[SSPC];
	for(i=0; i<=brktx; i++)
		if(brktab[i].addr == tpc){
			pbkptc = tpc;		/* remember who we cleared */
			dot = tpc;
			cfput(brktab[i].value);
		}
	if(pbkptc==0 && flag)	/* ;p really desnt need a temp bkpt */
		return(0);
	ssval[SSPS] =| 020;		/* turn on T bit */
	return(1);
}


/* remove a temporary breakpoint */

rtempbpt()
{

	register int i;

	if(pbkptc) {		/* must reset a permanent one */
		dot = pbkptc;
		cfput(BREAK);
		pbkptc = 0;
	}
}

/* print termination status */
prtermst()
{

	register int i;

	i = status&077;		/* status code */
	if(i>12) {
		printf("Unknown termination status\n");
		return;
	}
	printf("Termination status: %o  %s\n",status,termstr[i]);
}
