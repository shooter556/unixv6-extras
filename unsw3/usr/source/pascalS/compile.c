#include "nglob.h"
#include "ccglob.h"


/*
 *	keyword mapping.
 */
struct keyword {
	char	ksy;	/* internal representation */
	char	*key;	/* pointer to input representation */
} keyword[] {
		ANDSY,		"and",
		ARRAYSY,	"array",
		BEGINSY,	"begin",
		CASESY,		"case",
		CONSTSY,	"const",
		IDIV,		"div",
		DOSY,		"do",
		DOWNTOSY,	"downto",
		ELSESY,		"else",
		ENDSY,		"end",
		FORSY,		"for",
		FUNCSY,		"function",
		IFSY,		"if",
		IMOD,		"mod",
		NOTSY,		"not",
		OFSY,		"of",
		ORSY,		"or",
		PROCSY,		"procedure",
		PROGSY,		"program",
		RECORDSY,	"record",
		REPEATSY,	"repeat",
		THENSY,		"then",
		TOSY,		"to",
		TYPESY,		"type",
		UNTILSY,	"until",
		VARSY,		"var",
		WHILESY,	"while",
};


int	null[]	NULLSET;

/*
 *	sets used in error recovery. if a bit is set, it denotes
 *	the symbol with that bit number as its value.
 */
/*	sttbgsys	[BEGINSY,IFSY,CASESY,REPEATSY,WHILESY,FORSY] */
int	sttbgsys[]	{0000000, 0000000, 0003740, 0000000};
/*	blkbgsys	[CONSTSY,TYPESY,VARSY,FUNCSY,PROCSY,BEGINSY] */
int	blkbgsys[]	{0000000, 0170000, 0000041, 0000000};
/*	bkstbsys	[CONSTSY,TYPESY,VARSY,FUNCSY,PROCSY,BEGINSY,
				IFSY,CASESY,REPEATSY,WHILESY,FORSY] */
int	bkstbsys[]	{0000000, 0170000, 0003741, 0000000};
/*	conbgsys	[INTCON,REALCON,CHARCON,PLUS,MINUS,IDENT] */
int	conbgsys[]	{0000147, 0000000, 0000020, 0000000};
/*	typbgsys	[ARRAYSY,RECORDSY,IDENT] */
int	typbgsys[]	{0000000, 0000000, 0000026, 0000000};
/*	facbgsys	[INTCON,REALCON,CHARCON,NOTSY,LPARENT,IDENT] */
int	facbgsys[]	{0000027, 0000010, 0000020, 0000000};
/*	set_b		[BEGINSY] */
int	set_b[]		{0000000, 0000000, 0000040, 0000000};
/*	set_be		[EQL,BECOMES] */
int	set_be[]	{0020000, 0004000, 0000000, 0000000};
/*	set_btdd	[BECOMES,DOSY,TOSY,DOWNTOSY] */
int	set_btdd[]	{0000000, 0004000, 0100000, 0000003};
/* 	set_cc		[COMMA,COLON] */
int	set_cc[]	{0000000, 0002200, 0000000, 0000000};
/*	set_ccr		[RPARENT,COMMA,COLON] */
int	set_ccr[]	{0000000, 0002220, 0000000, 0000000};
/*	set_ci		[COMMA,IDENT] */
int	set_ci[]	{0000000, 0000200, 0000020, 0000000};
/*	set_crk		[RBRACK,COMMA] */
int	set_crk[]	{0000000, 0000300, 0000000, 0000000};
/*	set_crro	[RPARENT,RBRACK,COLON,OFSY] */
int	set_crro[]	{0000000, 0002120, 0040000, 0000000};
/*	set_crt		[RPARENT,COMMA] */
int	set_crt[]	{0000000, 0000220, 0000000, 0000000};
/*	set_d		[DOSY] */
int	set_d[]		{0000000, 0000000, 0100000, 0000000};
/*	set_e		[ELSESY] */
int	set_e[]		{0000000, 0000000, 0010000, 0000000};
/*	set_enllgg	[EQL,NEQ,GTR,GEQ,LSS,LEQ] */
int	set_enllgg[]	{0160000, 0000007, 0000000, 0000000};
/*	set_i		[IDENT] */
int	set_i[]		{0000000, 0000000, 0000020, 0000000};
/*	set_ies		[SEMICOLN,IDENT,ENDSY] */
int	set_ies[]	{0000000, 0000400, 0004020, 0000000};
/*	set_iv		[VARSY,IDENT] */
int	set_iv[]	{0000000, 0040000, 0000020, 0000000};
/*	set_lcs		[LPARENT,SEMICOLN,COLON] */
int	set_lcs[]	{0000000, 0002410, 0000000, 0000000};
/*	set_llp		[LPARENT,LBRACK,PERIOD] */
int	set_llp[]	{0000000, 0001050, 0000000, 0000000};
/*	set_occ		[COMMA,COLON,OFSY] */
int	set_occ[]	{0000000, 0002200, 0040000, 0000000};
/*	set_p		[PERIOD] */
int	set_p[]		{0000000, 0001000, 0000000, 0000000};
/*	set_pm		[PLUS,MINUS] */
int	set_pm[]	{0000140, 0000000, 0000000, 0000000};
/*	set_pmo		[PLUS,MINUS,ORSY] */
int	set_pmo[]	{0010140, 0000000, 0000000, 0000000};
/* 	set_rt		[RPARENT] */
int	set_rt[]	{0000000, 0000020, 0000000, 0000000};
/*	set_s		[SEMICOLN] */
int	set_s[]		{0000000, 0000400, 0000000, 0000000};
/*	set_sc		[SEMICOLN,COLON] */
int	set_sc[]	{0000000, 0002400, 0000000, 0000000};
/*	set_sci		[COMMA,SEMICOLN,IDENT] */
int	set_sci[]	{0000000, 0000600, 0000020, 0000000};
/*	set_se		[SEMICOLN,ENDSY] */
int	set_se[]	{0000000, 0000400, 0004000, 0000000};
/*	set_seci	[COMMA,SEMICOLN,IDENT,END] */
int	set_seci[]	{0000000, 0000600, 0004020, 0000000};
/*	set_sr		[RPARENT,SEMICOLN] */
int	set_sr[]	{0000000, 0000420, 0000000, 0000000};
/*	set_su		[SEMICOLN,UNTILSY] */
int	set_su[]	{0000000, 0000400, 0020000, 0000000};
/*	set_td		[DOSY,THENSY] */
int	set_td[]	{0000000, 0000000, 0100000, 0000004};
/*	set_tdd		[DOSY,TOSY,DOWNTOSY] */
int	set_tdd[]	{0000000, 0000000, 0100000, 0000003};
/*	set_triia	[TIMES,IDIV,RDIV,IMOD,ANDSY] */
int	set_triia[]	{0007600, 0000000, 0000000, 0000000};

/*
 *	fatal errors. correct by increasing appropriate
 *	table's size.
 */
char	*fmsg[] {
		" ",	/* no fatal(0); */
		"identifiers",
		"procedures",
		"reals",
		"arrays",
		"levels",
		"code",
		"strings",
};

/*
 *	mapping of error reports from integers
 *	to more meaningful values.
 */
char	*msg[] {
		"undef. identifier",
		"multi defined identifier",
		"identifier exp.",
		"'program' exp.",
		"')' exp.",
		"':' exp.",
		"syntax error",
		"identifier or 'var' exp.",
		"of exp.",
		"'(' exp.",
		"id, 'array' or 'record' exp.",
		"[ exp.",
		"] exp.",
		"'..' exp.",
		"';' exp.",
		"inval. func. type",
		"'=' exp.",
		"boolean expr. exp.",
		"contrl var type",
		"type conflict",
		"program param.",
		"too big",
		"'.' exp.",
		"typ (case)",
		"bad character",
		"illeg. const id",
		"index type",
		"index bound",
		"no such array",
		"type id exp.",
		"undef type",
		"no such record",
		"boolean type exp.",
		"illeg. arith. type",
		"integer exp.",
		"incompat. types",
		"param type",
		"var id exp.",
		"bad string",
		"no. of pars",
		"bad type",
		"bad type",
		"real type exp.",
		"integer exp.",
		"var, const",
		"var, proc illegal",
		"types (:=)",
		"type (case)",
		"illeg. arg type",
		"store ovfl",
		"bad constant",
		"':=' exp.",
		"'then' exp.",
		"'until' exp.",
		"'do' exp.",
		"'to'/'downto' exp.",
		"'begin' exp.",
		"'end' exp.",
		"bad factor",
};


/*
 *	everything starts to happen here.
 */
main(argc,argv)
int argc;
char **argv;
{	int extern fin;
	extern long latoi();
	register char *q;
	char *p;
	register int i;

	maxocnt = 10 * CYCLE;
	for(i = 1; i < argc; i++){
		switch( *(p = argv[i]) ){
		case '-':
			switch( *++p ){
			case 'l':
				plist = TRUE;
				p++;
				break;
			case 't':
				p++;
				maxocnt = CYCLE * latoi(&p);
				break;
			default:
				usage();
				return;
			}
			break;
		default:
			if(fin == 0){
				q = p;
				while(*q != 0)
					q++;
				q =- 2;
				if( q <= p || differ(q,".p")){
					usage();
					return;
				}
				if( (fin = open(p,0)) < 0){
					cant(p);
					return;
				}
			} else {
				usage();
				return;
			}
		}/* switch */
	}/* for */
	if( fin == 0 ){
		usage();
		return;
	}

	init();	initsfs();
	program();
	flush();
	compiling = 0;
	if(errcnt == 0)
		execute();
	flush();
}/* main */

usage()
{
	extern int fout;

	flush();
	fout = 2;
	prints("usage: psc [-l] [-tn] name.p\n",0);
	flush();
	fout = 0;
}

cant(p)
char *p;
{
	extern int fout;

	flush();
	fout = 2;
	prints("can't open:",0);
	prints(p,0);
	putchar(NL);
	flush();
	fout = 0;
}

long latoi(p)
register char **p;
{
	long val = 0;
	while( ('0' <= **p) && (**p <= '9') ){
		val = val * 10 + (**p - '0');
		(*p)++;
	}
	return(val);
}

program()
{
	register struct btab *btp;

	if(sy != PROGSY)
		error(3);
	else {
		insymbol();
		if(sy != IDENT)
			error(2);
		else {
			copystr(id,progname);
			insymbol();
			if(sy != LPARENT)
				error(9);
			else do {
				insymbol();
				if(sy != IDENT)
					error(2);
				else {
					if(!differ(id,"input"))
						iflag = 1;
					else
					  if(!differ(id,"output"))
						oflag = 1;
					     else
						error(0);
					insymbol();
				}
			} while(sy == COMMA);
			if(sy == RPARENT)
				insymbol();
			else
				error(4);
			if(!oflag)
				error(20);
		}
	}
	btp = &btab[1];
	btp->b_last = t;
	btp->b_lastpar = 1;
	btp->b_psize = 0;
	btp->b_vsize = 0;
	level = 1;
	block(bkstbsys,0);
	if(sy != PERIOD)
		error(22);
	emit(31);
	if(btab[2].b_vsize > STACKSZE)
		error(49);
/*	if(errcnt)
		errormsg();*/
	if(!differ(progname,"test0"))
		printtab();
}

init()
{
/*
 *	initialize table indices, and other globals.
 */
	compiling = 1;
	cc = lc = ll = errpos = errcnt = lineno = 0;
	copyset(null,errs);
	ch = ' ';
	insymbol();
	t = -1;
	a = 0;
	b = 1;
	sx = 0;
	c2 = 0;
	display[0] = 1;
	iflag = oflag = 0;
}
struct initsfy {
	char *stfnnm;
	int  stfnmod;
	int  stfntyp;
	int  stfnid;
} initsft[] {
	" ",		VARIABLE,	NOTYP,		0,
	"false",	KONSTANT,	BOOLS,		0,
	"true",		KONSTANT,	BOOLS,		1,
	"real",		TYPE,		REALS,		1,
	"char",		TYPE,		CHARS,		1,
	"boolean",	TYPE,		BOOLS,		1,
	"integer",	TYPE,		INTS,		1,
	"abs",		FUNCTION,	REALS,		0,
	"sqr",		FUNCTION,	REALS,		2,
	"odd",		FUNCTION,	BOOLS,		4,
	"chr",		FUNCTION,	CHARS,		5,
	"ord",		FUNCTION,	INTS,		6,
	"succ",		FUNCTION,	CHARS,		7,
	"pred",		FUNCTION,	CHARS,		8,
	"round",	FUNCTION,	INTS,		9,
	"trunc",	FUNCTION,	INTS,		10,
	"sin",		FUNCTION,	REALS,		11,
	"cos",		FUNCTION,	REALS,		12,
	"exp",		FUNCTION,	REALS,		13,
	"ln",		FUNCTION,	REALS,		14,
	"sqrt",		FUNCTION,	REALS,		15,
	"arctan",	FUNCTION,	REALS,		16,
	"eof",		FUNCTION,	BOOLS,		17,
	"eoln",		FUNCTION,	BOOLS,		18,
	"read",		PROCEDRE,	NOTYP,		1,
	"readln",	PROCEDRE,	NOTYP,		2,
	"write",	PROCEDRE,	NOTYP,		3,
	"writeln",	PROCEDRE,	NOTYP,		4,
	" ",		PROCEDRE,	NOTYP,		0,
};
initsfs()
{
/*
 *	enter all the standard functions and procedures in
 *	the symbol table.
 */
	register struct initsfy *p,*lastp1;

	lastp1 = &initsft[sizeof initsft/sizeof initsft[0]];
	for(p = initsft; p < lastp1; p++) {
		enterx(p->stfnnm, p->stfnmod, p->stfntyp, p->stfnid);
	}
}
error(n)
char n;	/* the type of error (0-ERRMAX) */
{
/*
 *	print the error number on the output file, with marker.
 *	if not standard output, print line number and message
 *	on the standard error file (file descriptor = 2).
 *
 *	modifies : errpos,errcnt,errs[].
 */
	extern fout;
	register i;

	if(errpos == 0){
		if(!plist){
			putline(line);
		}
		prints(" **",0);
	}
	if(cc > errpos) {
		for(i=errpos;i<(cc-1);i++)
			putchar(line[i]==TAB?TAB:' ');
		putchar('^');
		printd(n,2);
		errpos = (line[i]==TAB)&&((i&07)<06)?cc:cc+2;
		errcnt++;
		al(n,errs);
	}
	if(fout > 1) {
		flush();
		i = fout;
		fout = 2;
		printd(lineno,3);
		putchar(':');
		putchar(' ');
		prints(msg[n],0);
		putchar(NL);
		flush();
		fout = i;
	}
}
errormsg()
{
/*
 *	called on completion of compilation to print the
 *	mapping from error numbers to messages.
 */
	register k;

	for(k=0;k<=ERMAX;k++)
		if(in(k,errs)) {
			putchar(TAB);
			printd(k,2);
			putchar(' ');
			prints(msg[k],0);
			putchar(NL);
		}
	for(k=0;k<SETSIZE;k++)
		errs[k] = 0;
}
fatal(n)
int n;	/* type of fatal error */
{
/*
 *	called when a table has overflowed. is noted
 *	that tables should be grown dynamically.
 */
/*	errormsg();*/
	prints("\n compiler table for ",0);
	prints(fmsg[n],0);
	prints(" is too small\n",0);
	flush();
	exit(1);
}
insymbol()
{
/*
 *	the main scanning routine.
 *
 *	modifies : id[],sy,inum,rnum,stab[],sleng,sx.
 */
	int i,j;
	int k;
	int e,x;
	char bad;
	char strtstr;

 begin: while((ch == ' ') || (ch == TAB) || (ch == NL))
		nextch();
	if(ALPHA) {
		k = 0;
		do  {
			if(k < ALNG)
				id[k++] = ch;
			nextch();
		} while(ALPHA || NUMERIC);
		id[k] = '\0';
		i = 0; j = NKW - 1;
		do {
			k = (i + j) /2;
			x = differ(id,keyword[k].key);
			if(x <= 0)
				j = k - 1;
			if(x >= 0)
				i = k + 1;
		} while(i <= j);
		sy = ((i - 1) > j)?keyword[k].ksy:IDENT;
	} else
		if(NUMERIC) {
			k = bad = 0;
			inum = 0;
			sy = INTCON;
			do {
				i = ch - '0';
				if((++k <= KMAX) &&
				  (((LONGMAX-i)/10) >= inum)) {
			   		inum = inum*10;
					inum =+ i;
				}
			   	else
			   		bad = 1;
				nextch();
			} while(NUMERIC);
			if(bad) {
				if(compiling)
					error(21);
				k = 0;
				inum = 0;
			}
			if(ch == '.') {
				nextch();
				if(ch == '.')
					ch = ':';
				else {
					sy = REALCON;
					rnum = inum;
					e = 0;
					while(NUMERIC) {
					   e--;
					   rnum = rnum*10+ch-'0';
					   nextch();
					}
					if((ch == 'e')||(ch == 'E'))
					   readscle(&e);
					if(e)
					   ajstscle(k,e);
				}
			} else
				if((ch == 'e') || (ch == 'E')) {
					sy = REALCON;
					rnum = inum;
					e = 0;
					readscle(&e);
					if(e)
						ajstscle(k,e);
				}
		} else 
			switch(ch) {
			case ':' :
				nextch();
				switch(ch) {
				case '=' :
					sy = BECOMES;
					nextch();
					break;
				default  :
					sy = COLON;
				}
				break;
			case '<' :
				nextch();
				switch(ch) {
				case '=' :
					sy = LEQ;
					nextch();
					break;
				case '>' :
					sy = NEQ;
					nextch();
					break;
				default  :
					sy = LSS;
				}
				break;
			case '>' :
				nextch();
				switch(ch) {
				case '=' :
					sy = GEQ;
					nextch();
					break;
				default  :
					sy = GTR;
				}
				break;
			case '.' :
				nextch();
				switch(ch) {
				case '.' :
					sy = COLON;
					nextch();
					break;
				default  :
					sy = PERIOD;
				}
				break;
			case '"' : 
				/* error(24); */
			case '\'':
				k = 0;
				strtstr = ch;
			l2:	nextch();
				if(ch == strtstr) {
					nextch();
					if(ch != strtstr)
						goto l3;
				}
				if(sx+k == SMAX)
					fatal(7);
				stab[sx+k++] = ch;
				if(cc == 1)
					k = 0;
				else
					goto l2;
			l3:	if(k == 1) {
					sy = CHARCON;
					inum = stab[sx];
				} else if(k == 0) {
					error(38);
					sy = CHARCON;
					inum = 0;
					} else {
						sy = STRING;
						strgpos = sx;
						sleng = k;
						sx =+ k;
					  }
				break;
			case '(' :
				nextch();
				switch(ch) {
				case '*' :
					nextch();
					do {
						while(ch != '*')
							nextch();
						nextch();
					} while(ch != ')');
					nextch();
					goto begin;
				default  :
					sy = LPARENT;
				}
				break;
			case '+' :
				sy = PLUS;
				nextch();
				break;
			case '-' :
				sy = MINUS;
				nextch();
				break;
			case '*' :
				sy = TIMES;
				nextch();
				break;
			case '/' :
				sy = RDIV;
				nextch();
				break;
			case ')' :
				sy = RPARENT;
				nextch();
				break;
			case '=' :
				sy = EQL;
				nextch();
				break;
			case ',' :
				sy = COMMA;
				nextch();
				break;
			case '[' :
				sy = LBRACK;
				nextch();
				break;
			case ']' :
				sy = RBRACK;
				nextch();
				break;
			case '{' :
				do
					nextch();
				while(ch != '}');
				nextch();
				goto begin;
			case '#' :
				sy = NEQ;
				nextch();
				break;
			case '&' :
				sy = ANDSY;
				nextch();
				break;
			case ';' :
				sy = SEMICOLN;
				nextch();
				break;
			case EOR :
			case EOF :
				prints("\n ** program incomplete\n",0);
/*				errormsg();*/
				flush();
				exit(2);
			default :
				error(24);
				nextch();
				goto begin;
			}
}
readscle(e)
int *e;	/* the exponent */
{
/*
 *	reads in the exponent of a real number.
 */
	register s,sign;

	nextch();
	sign = 1; s = 0;
	if(ch == '+')
		nextch();
	else if(ch == '-') {
		nextch();
		sign = -1;
	     }
	while(NUMERIC) {
		s = s * 10 + ch - '0';
		nextch();
	}
	*e =+ s * sign;
}
ajstscle(k,e)
int k;	/* length of mantissa */
int e;	/* exponent */
{
/*
 *	converts the number to normalized form.
 *
 *	modifies : rnum.
 */
	register s;
	PASREAL d,tt;

	if((k + e) > EMAX) 
		error(21);
	else if((k + e) < EMIN)
		rnum = 0;
	else {
		s = abs(e);
		tt = 1.0;
		d = 10.0;
		do {
			while(!(s & 1)) {
				s =/ 2;
				d =* d;

			}
			tt =* d;
		} while(--s);
		if(e >= 0)
			rnum =* tt;
		else
			rnum =/ tt;
	}
}
getline()
{
/*
 *	fetches the next line from the input.
 *
 *	modifies : errpos,lineno,ll,cc,line[].
 */
	register char *l;
	register gotline;

	if(errpos) {
		putchar(NL);
		errormsg();
		errpos = 0;
	}
	cc = 0;
	l = line; gotline = 0;
	do {
		*l = getchar();
		*(l+1) = NL;
		if(*l == EOR || *l == EOF) {
			*(l+1) = NL;	/* terminator for putline */
			gotline = 1;
		}
	} while((*l++ != NL) && !gotline);
	ll = (l-line);
	lineno++;
	if(compiling&plist){
		putline(line);
	}
}

putline(l)
register char *l;
{
	printd(lc,3);
	putchar(' ');
	do {
		putchar(*l);
	} while(*l++ != NL);
}
 
enterx(x0,x1,x2,x3)
char *x0;	/* pointer to string to be entered */
char x1;	/* nature of entry */
char x2;	/* type of entry */
int x3;		/* address value */
{
/*
 *	enters the standard functions into the identifier table.
 *
 *	modifies : tab[],t.
 */
	register struct tab *p;

	p = &tab[++t];

	copystr(x0,p->t_name);
	p->t_link = t - 1;
	p->t_obj = x1;
	p->t_typ = x2;
	p->t_ref = 0;
	p->t_nrm = (x1 == KONSTANT ? 0:1);
	p->t_lev = 0;
	p->t_adr = x3;
}
entarray(tp,lp,hp)
int tp;		/* index type */
long *lp;	/* lower bound */
long *hp;	/* upper bound */
{
/*
 *	creates an entry in the array table for the given array.
 *
 *	modifies : atab[],a.
 */
	register struct atab *p;
	register long *l,*h;
	extern long labs();

	l = lp; h = hp;
	if(*l > *h)
		error(27);
	if((labs(*l) > XMAX) || (labs(*h) > XMAX)) {
		error(27);
		*l = 1;		*h = 0;
	}
	if (a == AMAX)
		fatal(4);
	else {
		p = &atab[++a];
		p->a_inxtyp = tp;
		p->a_low = *l;
		p->a_high = *h;
	}
}
enterblk()
{
/*
 *	sets up the start of a new block.
 *
 *	modifies : btab[],b;
 */
	register struct btab *p;

	if(b == BMAX)
		fatal(2);
	else {
		p = &btab[++b];
		p->b_last = p->b_lastpar = 0;
	}
}
int entconst(conrecp, conrslt)
struct conrec *conrecp;
int *conrslt;
{	/* either
		a. constant will fit into a single word, or
		b. constant requires more.
	conrecp - pointer to constant structure
	conrslt - returns :
		a. value of the constant, or
		b. index of constant in table "rconst"
	entconst - returns :
		a. 0 - small constant
		b. 1 - large constant
				*/
	register struct conrec *xcp;
	register PASREAL *xcc;
	register int xx;
	extern long labs();

	xcp = conrecp;
	switch(xcp->c_typ) {
	case BOOLS:
	case CHARS:	*conrslt = xcp->c_i;
			return(0);
	case INTS:	if(labs(xcp->c_i) < NMAX) {
				*conrslt = xcp -> c_i;
				return(0);
			} else {
				if(c2 >= C2MAX)
					fatal(3);
				else {
					xcc = rconst;
					rconst[c2 + 1].r_i = xcp->c_i;
					while(xcc->r_i != xcp->c_i)
						xcc++;
					xx = xcc - rconst;
					if(xx > c2)
						c2 = xx;
					*conrslt = xx;
					return(1);
				}
			}
	case REALS :	if(c2 >= C2MAX)
				fatal(3);
			else {
				xcc = rconst;
				rconst[c2 + 1] = xcp->c_r;
				while(*xcc != xcp ->c_r)
					xcc++;
				xx = xcc - rconst;
				if(xx > c2)
					c2 = xx;
				*conrslt = xx;
				return(1);
			}
	}
}
emit(fct)
char fct;	/* opcode */
{
/*
 *	emits an instruction with no parameters.
 *
 *	modifies : code[],lc.
 */

	if(lc == CMAX)
		fatal(6);
	else 
		code[lc++].o_f = fct;
}
emit1(fct,yp)
char fct;	/* opcode */
int yp;		/* jump address, or integer value */
{
/*
 *	emit an instruction with 1 parameter.
 *
 *	modifies : code[],lc.
 */
	register struct order *p;

	if(lc == CMAX)
		fatal(6);
	else {
		p = &code[lc++];
		p->o_f = fct;
		p->o_y = yp;
	}
}
emit2(fct,xp,yp)
char fct,xp;	/* opcode and level */
int yp;		/* address */
{
/*
 *	emit an instruction with 2 parameters.
 *
 *	modifies : code[],lc.
 */
	register struct order *p;

	if(lc == CMAX)
		fatal(6);
	else {
		p = &code[lc++];
		p->o_f = fct;
		p->o_x = xp;
		p->o_y = yp;
	}
}
block(fsys,isfun)
int *fsys;	/* the error recovery set */
char isfun;	/* 1=function, 0=procedure */
{
/*
 *	processes a block. invokes procedures to handle
 *	declarations, and then calls 'statment()'.
 *
 *	modifies : tab[],display[],btab[].
 */
	int dx;
	char x,prb,prt;
	int set[SETSIZE];

	dx = 5; prt = t;
	if(level > LMAX)
		fatal(5);
	test(set_lcs,fsys,7);
	enterblk();
	tab[prt].t_ref = display[level] = prb = b;
	tab[prt].t_typ = NOTYP;
	if(sy == LPARENT)
		parmlist(fsys,&dx);
	btab[prb].b_lastpar = t; btab[prb].b_psize = dx;
	if(isfun)
		if(sy == COLON) {
			insymbol();
			if(sy == IDENT) {
				x = loc(id);
				insymbol();
				if(x)
   				   if(tab[x].t_obj != TYPE)
					error(29);
				   else
				   if(tab[x].t_typ <= CHARS)
				      tab[prt].t_typ = tab[x].t_typ;
				   else
				      error(15);
			} else
			    skip(al(SEMICOLN,copyset(fsys,set)),2);
		} else
			error(5);
	if(sy == SEMICOLN)
		insymbol();
	else
		error(14);
	do {
		if(sy == CONSTSY)
			constdcl(fsys);
		if(sy == TYPESY)
			typedcl(fsys);
		if(sy == VARSY)
			vardcl(fsys,&dx);
		btab[prb].b_vsize = dx;
		while((sy == PROCSY) || (sy == FUNCSY))
			procdcl(fsys);
		test(set_b,bkstbsys,56);
	} while(!in(sy,sttbgsys));
	tab[prt].t_adr = lc; 
	insymbol();
	statment(as(set_se,copyset(fsys,set)));
	while(in(sy,al(SEMICOLN,copyset(sttbgsys,set)))) {
		if(sy == SEMICOLN)
			insymbol();
		else
			error(14);
		statment(as(set_se,copyset(fsys,set)));
	}
	if(sy == ENDSY)
		insymbol();
	else
		error(57);
	test(al(PERIOD,copyset(fsys,set)),null,6);
}
skip(fsys,n)
int *fsys;
char n;	/* error code */
{
/*
 *	invokes 'error()' and then finds the next acceptable
 *	recovery symbol.
 */
	error(n);
	while(!in(sy,fsys))
		insymbol();
}
test(s1,s2,n)
int *s1,*s2;	/* recovery sets */
char n;		/* error code */
{
/*
 *	if the current symbol is not acceptable, skip
 *	until a member of s1|s2 is found.
 */
	int set[SETSIZE];

	if(!in(sy,s1))
		skip(as(s1,copyset(s2,set)),n);
}
testscln(fsys)
int *fsys;
{
/*
 *	called when a semicolon is expected. if not found,
 *	prints error, and skips till ident or block starter
 *	is found.
 */
	int set[SETSIZE];

	if(sy == SEMICOLN)
		insymbol();
	else {
		error(14);
		if((sy == COMMA) || (sy == COLON))
			insymbol();
	}
	test(al(IDENT,copyset(blkbgsys,set)),fsys,6);
}
enter(idp,k)
char *idp;	/* pointer to string */
char k;		/* nature of entry */
{
/*
 *	enters the id in the table, with appropriate description.
 *	search is linear within a given level.
 *
 *	modifies : tab[],t,btab[].
 */
	register struct tab *p;
	register char j,l;

	if(t == TMAX)
		fatal(1);
	else {
		p = tab;
		copystr(idp,p->t_name);
		l = j = btab[display[level]].b_last;
		while(differ((p+j)->t_name,idp))
			j = (p+j)->t_link;
		if(j)
			error(1);
		else {
			p = &tab[++t];
			copystr(idp,p->t_name);
			p->t_link = l;
			p->t_obj = k;
			p->t_typ = NOTYP;
			p->t_ref = p->t_adr = 0;
			p->t_lev = level;
			btab[display[level]].b_last = t;
		}
	}
}
loc(idp)
char *idp;	/* pointer to string */
{
/*
 *	locate the given id in tab[]. if not there, then error.
 *	otherwise, return the index.
 *	again, search is linear within a given level.
 *
 *	modifies : tab[]. (scratch entry only).
 */
	register struct tab *p;
	register char i,j;

	i = level;
	p = tab;
	copystr(idp,p->t_name);
	do {
		j = btab[display[i]].b_last;
		while(differ((p+j)->t_name,idp))
			j = (p+j)->t_link;
	} while((--i >= 0) && (j == 0));
	if(j == 0)
		error(0);
	return(j);
}
entervar()
{
	if(sy == IDENT) {
		enter(id,VARIABLE);
		insymbol();
	} else
		error(2);
}
constant(fsys,p)
int *fsys;
struct conrec *p;	/* return address for constant descrip. */
{
/*
 *	finds and processes a constant.
 */
	register struct conrec *c;
	register char x;
	register sign;

	c = p; c->c_typ = NOTYP; c->c_i = 0;
	test(conbgsys,fsys,50);
	if(in(sy,conbgsys)) {
		switch(sy) {
		case CHARCON :
			c->c_typ = CHARS;
			c->c_i = inum;
			insymbol();
			break;
		default :
			sign = 1;
			switch(sy) {
			case MINUS :
				sign = -1;
			case PLUS :
				insymbol();
			}
			switch(sy) {
			case IDENT :
				if(x = loc(id))
				   if(tab[x].t_obj != KONSTANT)
					error(25);
				   else {
					c->c_typ = tab[x].t_typ;
					if(c->c_typ == REALS)
						c->c_r = sign *
						rconst[tab[x].t_adr];
					else
						c->c_i = sign *
						tab[x].t_adr;
				   }
				insymbol();
				break;
			case INTCON :
				c->c_typ = INTS;
				c->c_i = sign * inum;
				insymbol();
				break;
			case REALCON :
				c->c_typ = REALS;
				c->c_r = sign * rnum;
				insymbol();
				break;
			default :
				skip(fsys,50);
			}
		}
		test(fsys,null,6);
	}
}
typ(fsys,tp,rf,sz)
int *fsys;
char *tp,*rf;	/* type and reference pointers */
int *sz;	/* size pointer */
{
/*
 *	processes part of a type declaration. invokes 
 *	itself recursively for record declarations.
 *
 *	modifies : display[],tab[],btab[],level.
 */
	register struct tab *p;
	register char t0,t1;
	int offset,elsz;
	char x,eltp,elrf;
	int set[SETSIZE];
	
	*tp = NOTYP;
	*rf = *sz = 0;
	test(typbgsys,fsys,10);
	switch(sy) {
	case IDENT :
		if(x = loc(id)) {
			p = &tab[x];
			if(p->t_obj != TYPE)
				error(29);
			else {
				*tp = p->t_typ;
				*rf = p->t_ref;
				*sz = p->t_adr;
				if(*tp == NOTYP)
					error(30);
			}
		}
		insymbol();
		break;
	case ARRAYSY :
		insymbol();
		if(sy == LBRACK)
			insymbol();
		else {
			error(11);
			if(sy == LPARENT)
				insymbol();
		}
		*tp = ARRAYS;
		arraytyp(fsys,rf,sz);
		break;
	case RECORDSY :
		insymbol();
		enterblk();
		*tp = RECORDS;
		*rf = b;
		if(level++ >= LMAX)
			fatal(5);
		display[level] = b;
		offset = 0;
		while(sy != ENDSY) {
			if(sy == IDENT) {
				t0 = t;
				entervar();
				while(sy == COMMA) {
					insymbol();
					entervar();
				}
				if(sy == COLON)
					insymbol();
				else
					error(5);
				t1 = t;
				typ(as(set_seci,
				    copyset(fsys,set)),
				    &eltp,&elrf,&elsz);
				while(t0 < t1) {
					p = &tab[++t0];
					p->t_typ = eltp;
					p->t_ref = elrf;
					p->t_nrm = 1;
					p->t_adr = offset;
					offset =+ elsz;
				}
			}
			if(sy != ENDSY) {
				if(sy == SEMICOLN)
					insymbol();
				else {
					error(14);
					if(sy != IDENT)
						insymbol();
				}
				test(set_ies,fsys,6);
			}
		}
		btab[*rf].b_vsize = *sz = offset;
		btab[*rf].b_psize = 0;
		insymbol();
		level--;
		break;
	default :
		return;
	}
	test(fsys,null,6);
}
arraytyp(fsys,aref,arsz)
int *fsys;
char *aref;
int *arsz;
{
/*
 *	processes part of an array declaration, recursively
 *	if need be. calls 'typ()' to get the types
 *	of array elements.
 *
 *	modifies : atab[].
 */
	register struct conrec *l,*h;
	register struct atab *p;
	char eltp,elrf;
	int elsz;
	struct conrec scl,sch;
	int set[SETSIZE];

	l = &scl;
	constant(as(set_crro,copyset(fsys,set)),l);
	if(l->c_typ == REALS) {
		error(27);
		l->c_typ = INTS;
		l->c_i  = 0;
	}
	if(sy == COLON)
		insymbol();
	else
		error(13);
	h = &sch;
	constant(as(set_crro,copyset(fsys,set)),h);
	if(h->c_typ != l->c_typ) {
		error(27);
		h->c_i = l->c_i;
	}
	entarray(l->c_typ,&(l->c_i),&(h->c_i));
	*aref = a;
	switch(sy) {
	case COMMA :
		insymbol();
		eltp = ARRAYS;
		arraytyp(fsys,&elrf,&elsz);
		break;
	default :
		switch(sy) {
		case RBRACK :
			insymbol();
			break;
		default :
			error(12);
			if(sy == RPARENT)
				insymbol();
		}
		if(sy == OFSY)
			insymbol();
		else
			error(8);
		typ(fsys,&eltp,&elrf,&elsz);
	}
	p = &atab[*aref];
	*arsz = (p->a_high - p->a_low + 1) * elsz;
	p->a_size = *arsz;
	p->a_eltyp = eltp;
	p->a_elref = elrf;
	p->a_elsize = elsz;
 }
parmlist(fsys,dx)
int *fsys;
int *dx;	/* pointer to space required */
{
/*
 *	processes the formal parameter list to a procedure.
 *
 *	modifies : tab[].
 */
	register struct tab *p;
	register char t0;
	char tp,rf,x,valpar;
	int sz;
	int set[SETSIZE];

	insymbol();
	tp = NOTYP; rf = sz = 0;
	test(set_iv,al(RPARENT,copyset(fsys,set)),7);
	while((sy == IDENT) || ( sy == VARSY)) {
		if(sy != VARSY)
			valpar = 1;
		else {
			insymbol();
			valpar = 0;
		}
		t0 = t; entervar();
		while(sy == COMMA) {
			insymbol();
			entervar();
		}
		if(sy == COLON) {
			insymbol();
			if(sy != IDENT)
				error(2);
			else {
				x = loc(id);
				insymbol();
				if(x) {
					p = &tab[x];
					if(p->t_obj != TYPE)
						error(29);
					else {
					    tp = p->t_typ;
					    rf = p->t_ref;
					    sz = valpar?p->t_adr:1;
					}
				}
			}
			test(set_sr,as(set_ci,copyset(fsys,set)),14);
		} else
			error(5);
		while(t0 < t) {
			p = &tab[++t0];
			p->t_typ = tp;
			p->t_ref = rf;
			p->t_nrm = valpar;
			p->t_adr = *dx;
			p->t_lev = level;
			*dx =+ sz;
		}
		if(sy != RPARENT) {
			if(sy == SEMICOLN)
				insymbol();
			else {
				error(14);
				if(sy == COMMA)
					insymbol();
			}
			test(set_iv,al(RPARENT,copyset(fsys,set)),6);
		}
	}
	if(sy == RPARENT) {
		insymbol();
		test(set_sc,copyset(fsys,set),6);
	} else
		error(4);
}
constdcl(fsys)
int *fsys;
{
/*
 *	processes a constant declaration.
 *
 *	modifies : tab[].
 */
	register struct conrec *c;
	register struct tab *p;
	struct conrec sc;
	int set[SETSIZE];

	c = &sc;
	insymbol();
	test(set_i,blkbgsys,2);
	while(sy == IDENT) {
		enter(id,KONSTANT);
		insymbol();
		if(sy == EQL)
			insymbol();
		else {
			error(16);
			if(sy == BECOMES)
				insymbol();
		}
		constant(as(set_sci,copyset(fsys,set)),c);
		p = &tab[t];
		p->t_typ = c->c_typ;
		p->t_ref = 0;
		p->t_nrm = entconst(c,&(p->t_adr));
		testscln(fsys);
	}
}
typedcl(fsys)
int *fsys;
{
/*
 *	processes a type decalaration.
 *
 *	modifies : tab[].
 */
	register struct tab *p;
	register char t1;
	char tp,rf;
	int sz;
	int set[SETSIZE];

	insymbol();
	test(set_i,blkbgsys,2);
	while(sy == IDENT) {
		enter(id,TYPE);
		t1 = t;
		insymbol();
		if(sy == EQL)
			insymbol();
		else {
			error(16);
			if (sy == BECOMES)
				insymbol();
		}
		typ(as(set_sci,copyset(fsys,set)),&tp,&rf,&sz);
		p = &tab[t1];
		p->t_typ = tp;
		p->t_ref = rf;
		p->t_adr = sz;
		testscln(fsys);
	}
}
vardcl(fsys,dx)
int *fsys;
int *dx;	/* pointer to space required for variables */
{
/*
 *	processes a variable declaration.
 *
 *	modifies : tab[].
 */
	register struct tab *p;
	register char t0,t1;
	char tp,rf;
	int sz;
	int set[SETSIZE];

	insymbol();
	while(sy == IDENT) {
		t0 = t;
		entervar();
		while(sy == COMMA) {
			insymbol();
			entervar();
		}
		if(sy == COLON)
			insymbol();
		else
			error(5);
		t1 = t;
		typ(as(set_sci,copyset(fsys,set)),
			&tp,&rf,&sz);
		while(t0 < t1) {
			p = &tab[++t0];
			p->t_typ = tp;
			p->t_ref = rf;
			p->t_lev = level;
			p->t_adr = *dx;
			p->t_nrm = 1;
			*dx =+ sz;
		}
		testscln(fsys);
	}
}
procdcl(fsys)
int *fsys;
{
/*
 *	processes a procedure or function declaration.
 *	invokes 'block()' to compile the next block.
 *
 *	modifies : tab[],level,id[].
 */
	register isfun;
	int set[SETSIZE];

	isfun = (sy == FUNCSY);
	insymbol();
	if(sy != IDENT) {
		error(2);
		id[0] = ' '; id[1] = '\0';
	}
	enter(id,isfun?FUNCTION:PROCEDRE);
	tab[t].t_nrm = 1;
	insymbol();
	level++;
	block(al(SEMICOLN,copyset(fsys,set)),isfun);
	level--;
	if(sy == SEMICOLN)
		insymbol();
	else
		error(14);
	emit(32+isfun);
}
statment(fsys)
int *fsys;
{
/*
 *	decides which procedure to invoke to handle the
 *	next statment, by doing a switch on the current
 *	symbol.
 */
	register char i;
	register struct tab *p;

	switch(sy) {
	case IDENT :
		i = loc(id);
		insymbol();
		if(i) {
			p = &tab[i];
			switch(p->t_obj) {
			case KONSTANT :
			case TYPE     :
				error(45);
				break;
			case VARIABLE :
				assgnmnt(i,p->t_lev,p->t_adr,fsys);
				break;
			case PROCEDRE :
				if(p->t_lev)
					call(fsys,i);
				else
					stndproc(p->t_adr,fsys);
				break;
			case FUNCTION :
				if(p->t_ref == display[level])
				    assgnmnt(i,p->t_lev +1,0,fsys);
				else
				    error(45);
			}
		}
		break;
	case BEGINSY :
		cmpdstat(fsys);
		break;
	case IFSY    :
		ifstat(fsys);
		break;
	case CASESY  :
		casestat(fsys);
		break;
	case WHILESY :
		whilstat(fsys);
		break;
	case REPEATSY:
		reptstat(fsys);
		break;
	case FORSY   :
		forstat(fsys);
		break;
	default      :
		break;
	}
	test(fsys,null,14);
}
selector(fsys,v)
int *fsys;
struct item *v;
{
/*
 *	generates the code to access fields of records
 *	and elements of arrays.
 *
 *	modifies : tab[]. (scratch entry only).
 */
	register i,j;
	register struct tab *p;
	struct item x;
	int set[SETSIZE];

	do {
		switch(sy) {
		case PERIOD :
			insymbol();
			if(sy != IDENT)
				error(2);
			else {
				if(v->i_typ != RECORDS)
					error(31);
				else {
				   j = btab[v->i_ref].b_last;
				   p = tab;
				   copystr(id,p->t_name);
				   while(differ((p+j)->t_name,id))
					   j = (p+j)->t_link;
				   if(j == 0)
					   error(0);
				   v->i_typ = (p+j)->t_typ;
				   v->i_ref = (p+j)->t_ref;
				   if(i = (p+j)->t_adr)
					   emit1(9,i);
				}
				insymbol();
			}
			break;
		case LPARENT :
		case LBRACK  :
			if(sy != LBRACK)
				error(11);
			do {
			   insymbol();
			   expressn(as(set_crk,copyset(fsys,set)),&x);
			   if(v->i_typ != ARRAYS)
			   	error(28);
			   else {
			     i = v->i_ref;
			     if(atab[i].a_inxtyp != x.i_typ)
			     	error(26);
			     else
				emit1(20+(!(atab[i].a_elsize==1)),i);
			     v->i_typ = atab[i].a_eltyp;
			     v->i_ref = atab[i].a_elref;
			   }
			} while(sy == COMMA);
			if(sy == RBRACK)
				insymbol();
			else {
				error(12);
				if(sy == RPARENT)
					insymbol();
			}
			break;
		}
	} while((sy == LBRACK)||(sy == LPARENT)||(sy == PERIOD));
	test(fsys,null,6);
}
call(fsys,ip)
int *fsys;
char ip;	/* pointer into tab[] for called routine */
{
/*
 *	handles procedure and function calls.
 */
	register struct tab *p;
	register char lastp,cp;
	struct item x;
	char k;
	int set[SETSIZE];

	emit1(18,ip);
	lastp = btab[tab[ip].t_ref].b_lastpar;
	cp = ip;
	if(sy == LPARENT) {
	   do {
	      insymbol();
	      if(cp >= lastp)
	      	error(39);
	      else {
     	         p = &tab[++cp];
     	         switch(p->t_nrm) {
		 case 1 : /* value parameter */
     	         	expressn(as(set_ccr,copyset(fsys,set)),&x);
     	         	if(x.i_typ == p->t_typ) {
     	         		if(x.i_ref != p->t_ref)
					error(36);
     	         		else if(x.i_typ == ARRAYS)
     	         		     emit1(22,atab[x.i_ref].a_size);
		   	        else if(x.i_typ == RECORDS)
				     emit1(22,btab[x.i_ref].b_vsize);
		   	} else
		   	   if(x.i_typ == INTS && p->t_typ == REALS)
		   	   	emit1(26,0);
		   	   else if(x.i_typ != NOTYP)
		   	   	error(36);
		   	break;
		 case 0 : /* var parameter */
		   	if(sy != IDENT)
		   	   error(2);
		   	else {
		   	   k = loc(id);
		   	   insymbol();
		   	   if(k) {
		   	   	p = &tab[k];
		   	   	if(p->t_obj != VARIABLE)
		   	   		error(37);
		   	   	x.i_typ = p->t_typ;
		   	   	x.i_ref = p->t_ref;
	   	   		emit2(!(p->t_nrm),p->t_lev,p->t_adr);
				if(sy == LBRACK || sy == LPARENT
					|| sy == PERIOD)
		   	   	   selector(as(set_ccr,
					    copyset(fsys,set)),&x);
		   	   	if(x.i_typ != tab[cp].t_typ 
		   	   		|| x.i_ref != tab[cp].t_ref)
		   	   		error(36);
			   }
			}
			break;
		}
	      }
   	      test(set_crt,fsys,6);
	   } while(sy == COMMA);
	   if(sy == RPARENT)
		insymbol();
	   else
		error(4);
	}
	if(cp < lastp)
		error(39);
	p = &tab[ip];
	emit1(19,btab[p->t_ref].b_psize - 1);
	if(p->t_lev < level)
		emit2(3,p->t_lev,level);
}
reslttyp(ap,bp)
char ap,bp;	/* argument types to be resolved into one type */
{
	register char a1,b1;

	a1 = ap; b1 = bp;
	if((a1 > REALS) || (b1 > REALS)) {
		error(33);
		return(NOTYP);
	} 
	if((a1 == NOTYP) || (b1 == NOTYP))
		return(NOTYP);
	if(a1 == INTS)
		if(b1 == INTS)
			return(INTS);
		else {
			emit1(26,1);
			return(REALS);
		}
	else {
		if(b1 == INTS)
			emit1(26,0);
		return(REALS);
	}
}
expressn(fsys,xp)
int *fsys;
struct item *xp;
{
	register struct item *x,*y;
	register char op;
	struct item siy;
	int set[SETSIZE];

	x = xp; y = &siy;
	simpexpn(as(set_enllgg,copyset(fsys,set)),x);
	if(in(sy,set_enllgg)) {
		op = sy;
		insymbol();
		simpexpn(fsys,y);
		if(x->i_typ <= CHARS && x->i_typ != REALS
			&& x->i_typ == y->i_typ)
			switch(op) {
			case EQL :
				emit(45); break;
			case NEQ :
				emit(46); break;
			case LSS :
				emit(47); break;
			case LEQ :
				emit(48); break;
			case GTR :
				emit(49); break;
			case GEQ :
				emit(50); break;
			}
		else {
			if(x->i_typ == INTS) {
				x->i_typ = REALS;
				emit1(26,1);
			} else if(y->i_typ == INTS) {
					y->i_typ = REALS;
					emit1(26,0);
				}
			if(x->i_typ == REALS && y->i_typ == REALS)
				switch(op) {
				case EQL :
					emit(39); break;
				case NEQ :
					emit(40); break;
				case LSS :
					emit(41); break;
				case LEQ :
					emit(42); break;
				case GTR :
					emit(43); break;
				case GEQ :
					emit(44); break;
				}
			else
				error(35);
		}
		x->i_typ = BOOLS;
	}
}
simpexpn(fsys,xp)
int *fsys;
struct item *xp;
{
	register struct item *x,*y;
	register char op;
	struct item siy;
	int set[SETSIZE];

	x = xp; y = &siy;
	if(in(sy,set_pm)) {
		op = sy;
		insymbol();
		term(as(set_pm,copyset(fsys,set)),x);
		if(x->i_typ > REALS)
			error(33);
		else if(op == MINUS)
			emit1(36,x->i_typ);
	} else
		term(as(set_pmo,copyset(fsys,set)),x);
	while(in(sy,set_pmo)) {
		op = sy;
		insymbol();
		term(as(set_pmo,copyset(fsys,set)),y);
		if(op == ORSY) {
			if(x->i_typ == BOOLS && y->i_typ == BOOLS)
				emit(51);
			else {
			   if(x->i_typ != NOTYP && y->i_typ != NOTYP)
				   error(32);
			   x->i_typ = NOTYP;
			}
		} else 
			switch(x->i_typ =
				 reslttyp(x->i_typ,y->i_typ)) {
			case INTS :
				emit(52 + (op != PLUS));
				break;
			case REALS :
				emit(54 + (op != PLUS));
				break;
			}
	}
}
term(fsys,xp)
int *fsys;
struct item *xp;
{
	register struct item *x,*y;
	register char op;
	struct item siy;
	int set[SETSIZE];

	x = xp; y = &siy;
	factor(as(set_triia,copyset(fsys,set)),x);
	while(in(sy,set_triia)) {
		op = sy;
		insymbol();
		factor(as(set_triia,copyset(fsys,set)),y);
		switch(op) {
		case TIMES :
			switch(x->i_typ =
				 reslttyp(x->i_typ,y->i_typ)) {
			case INTS :
				emit(57); break;
			case REALS:
				emit(60);
			}
			break;
		case RDIV  :
			if(x->i_typ == INTS) {
				emit1(26,1);
				x->i_typ = REALS;
			}
			if(y->i_typ == INTS) {
				emit(26,0);
				y->i_typ = REALS;
			}
			if(x->i_typ == REALS && y->i_typ == REALS)
				emit(61);
			else {
			   if(x->i_typ != NOTYP && y->i_typ != NOTYP)
				   error(32);
			   x->i_typ = NOTYP;
			}
			break;
		case ANDSY:
			if(x->i_typ == BOOLS && y->i_typ == BOOLS)
				emit(56);
			else {
			   if(x->i_typ != NOTYP && y->i_typ != NOTYP)
				   error(32);
			   x->i_typ = NOTYP;
			}
			break;
		case IDIV :
		case IMOD :
			if(x->i_typ == INTS && y->i_typ == INTS)
				emit(58 + (op != IDIV));
			else {
			   if(x->i_typ != NOTYP && y->i_typ != NOTYP)
				   error(34);
			   x->i_typ = NOTYP;
			}
			break;
		}
	}
}
factor(fsys,xp)
int *fsys;
struct item *xp;
{
	register struct item *x;
	register struct tab *p;
	register char i;
	char f;
	int set[SETSIZE];
	struct conrec sc;
	int c1,cn;

	x = xp;
	x->i_typ = NOTYP; x->i_ref = 0;
	test(facbgsys,fsys,58);
	for(;;) {
		switch(sy) {
		case IDENT :
			i = loc(id);
			insymbol();
			p = &tab[i];
			switch(p->t_obj) {
			case KONSTANT :
				x->i_typ = p->t_typ;
				x->i_ref = 0;
				if( p->t_nrm == 0 )
					emit1(24, p->t_adr);
				else
					emit1(25, p->t_adr);
				break;
			case VARIABLE :
				x->i_typ = p->t_typ;
				x->i_ref = p->t_ref;
				switch(sy) {
				case LBRACK :
				case LPARENT:
				case PERIOD :
				   emit2(!p->t_nrm,p->t_lev,p->t_adr);
				   selector(fsys,x);
				   if(x->i_typ <= CHARS)
					   emit(34);
				   break;
				default :
				   if(x->i_typ <= CHARS)
					   f = (p->t_nrm)?1:2;
				   else
					   f = (p->t_nrm)?0:1;
				   emit2(f,p->t_lev,p->t_adr);
				   break;
				}
				break;
			case TYPE :
			case PROCEDRE :
				error(44);
				break;
			case FUNCTION :
				x->i_typ = p->t_typ;
				if(p->t_lev)
					call(fsys,i);
				else
					stndfunc(p,x,fsys);
				break;
			}
			break;
		case CHARCON :
		case INTCON :
			x->i_typ = sc.c_typ = (sy == CHARCON)?CHARS:INTS;
			sc.c_i = inum;
			cn = entconst(&sc, &c1);
			emit1(24 + cn, c1);
			x->i_ref = 0;
			insymbol();
			break;
		case REALCON :
			x->i_typ = sc.c_typ = REALS;
			sc.c_r = rnum;
			cn = entconst(&sc, &c1);
			emit1(24 + cn, c1);
			x->i_ref = 0;
			insymbol();
			break;
		case LPARENT :
			insymbol();
			expressn(al(RPARENT,copyset(fsys,set)),x);
			if(sy == RPARENT)
				insymbol();
			else
				error(4);
			break;
		case NOTSY :
			insymbol();
			factor(fsys,x);
			switch(x->i_typ) {
			case NOTYP :
				break;
			case BOOLS :
				emit(35);
				break;
			default :
				error(32);
			}
			break;
		default :
			return;
		}
	test(fsys,facbgsys,6);
	}
}
stndfunc(pp,xp,fsys)
struct tab *pp;	/* pointer to tab[] entry for function */
struct item *xp;
int *fsys;
{
/*
 *	generate the code to invoke the built-in function.
 */
	register struct item *x;
	register struct tab *p;
	register char ts;
	int n;
	int set[SETSIZE];

	x = xp; p = pp;
	n = p->t_adr;
	if(sy == LPARENT)
		insymbol();
	else
		error();
	if(n < 17) {
		expressn(al(RPARENT,copyset(fsys,set)),x);
		switch(n) {
/* abs */	case 0 :
/* sqr */	case 2 :
			ts = INTS | REALS;
			p->t_typ = x->i_typ;
			if(x->i_typ == REALS)
				n++;
			break;
/* ord */	case 4 :
/* chr */	case 5 :
			ts = INTS;
			break;
/* ord */	case 6 :
			ts = INTS | BOOLS | CHARS;
			break;
/*succ */	case 7 :
/*pred */	case 8 :
			ts = CHARS;
			break;
/*round*/	case 9 :
/*trunc*/	case 10:
/* sin */	case 11:
/* cos */	case 12:
/* exp */	case 13:
/* ln  */	case 14:
/*sqrt */	case 15:
/*atan */	case 16:
			ts = INTS | REALS;
			if(x->i_typ == INTS)
				emit1(26,0);
			break;
		}
		if(x->i_typ & ts)
			emit1(8,n);
		else if(x->i_typ != NOTYP)
			error(48);
	} else {
		if(sy != IDENT)
			error(2);
		else if(differ(id,"input"))
			error(0);
		     else
			insymbol();
/*eof,eoln*/	emit1(8,n);
	  }
	x->i_typ = p->t_typ;
	if(sy == RPARENT)
		insymbol();
	else
		error(4);
}
assgnmnt(ip,lv,ad,fsys)
char ip,lv;
int ad;
int *fsys;
{
	register struct tab *p;
	register struct item *x,*y;
	struct item six,siy;
	int set[SETSIZE];

	x = &six; y = &siy;
	p = &tab[ip];
	x->i_typ = p->t_typ;
	x->i_ref = p->t_ref;
	emit2(!(p->t_nrm),lv,ad);
	if((sy == LBRACK) || (sy == LPARENT) || (sy == PERIOD))
		selector(as(set_be,copyset(fsys,set)),x);
	if(sy == BECOMES)
		insymbol();
	else {
		error(51);
		if(sy == EQL)
			insymbol();
	}
	expressn(fsys,y);
	if(x->i_typ == y->i_typ)
		if(x->i_typ <= CHARS)
			emit(38);
		else if(x->i_ref != y->i_ref)
			error(46);
		     else if(x->i_typ == ARRAYS)
				emit1(23,atab[x->i_ref].a_size);
			  else
				emit1(23,btab[x->i_ref].b_vsize);
	else 
	if(x->i_typ == REALS && y->i_typ == INTS) {
		emit1(26,0);
		emit(38);
	} else
		if(x->i_typ != NOTYP && y->i_typ != NOTYP)
			error(46);
}
cmpdstat(fsys)
int *fsys;
{
	int set[SETSIZE];

	insymbol();
	statment(as(set_se,copyset(fsys,set)));
	while((sy == SEMICOLN) || in(sy,sttbgsys)) {
		if(sy == SEMICOLN)
			insymbol();
		else
			error(14);
		statment(as(set_se,copyset(fsys,set)));
	}
	if(sy == ENDSY)
		insymbol();
	else
		error(57);
}
ifstat(fsys)
int *fsys;
{
/*
 *	modifies : code[].
 */
	register lc1,lc2;
	struct item x;
	int set[SETSIZE];

	insymbol();
	expressn(as(set_td,copyset(fsys,set)),&x);
	if(!(x.i_typ & (BOOLS | NOTYP)))
		error(17);
	lc1 = lc;
	emit(11);
	if(sy == THENSY)
		insymbol();
	else {
		error(52);
		if(sy == DOSY)
			insymbol();
	}
	statment(al(ELSESY,copyset(fsys,set)));
	if(sy == ELSESY) {
		insymbol();
		lc2 = lc;
		emit(10);
		code[lc1].o_y = lc;
		statment(fsys);
		code[lc2].o_y = lc;
	} else
		code[lc1].o_y = lc;
}
casestat(fsys)
int *fsys;
{
/*
 *	modifies : code[].
 */
	register k,lc1;
	int i,j;
	struct item x;
	int set[SETSIZE];

	insymbol();
	i = j = 0;
	expressn(as(set_occ,copyset(fsys,set)),&x);
	if((x.i_typ == REALS) || (x.i_typ >= ARRAYS))
		error(23);
	lc1 = lc;
	emit(12);
	if(sy == OFSY)
		insymbol();
	else
		error(8);
	onecase(&i,&j,&x,fsys);
	while(sy == SEMICOLN) {
		insymbol();
		onecase(&i,&j,&x,fsys);
	}
	code[lc1].o_y = lc;
	for(k=1;k<=i;k++) {
		emit1(13,casetab[k].c_val);
		emit1(13,casetab[k].c_lc);
	}
	emit1(10,0);
	for(k=1;k<=j;k++) 
		code[exittab[k]].o_y = lc;
	if(sy == ENDSY)
		insymbol();
	else
		error(57);
}
caselabl(i,x,fsys)
int *i;
struct item *x;
int *fsys;
{
/*
 *	modifies : casetab[].
 */
	register struct conrec *lab;
	register k;
	struct conrec sc;
	int set[SETSIZE];
	extern long labs();

	lab = &sc;
	constant(as(set_cc,copyset(fsys,set)),lab);
	if(lab->c_typ != x->i_typ)
		error(47);
	else if(*i == CSMAX)
		fatal(6);
	     else if(labs(lab->c_i) > NMAX)
		error(21);
	     else {
		casetab[++(*i)].c_val = lab->c_i;
		casetab[*i].c_lc = lc;
		k = 0;
		while(casetab[++k].c_val != lab->c_i);
		if(k < *i)
			error(1);
	     }
}
onecase(i,j,x,fsys)
int *i,*j;
struct item *x;
int *fsys;
{
/*
 *	modifies : exittab[].
 */
	int set[SETSIZE];

	if(in(sy,conbgsys)) {
		caselabl(i,x,fsys);
		while(sy == COMMA) {
			insymbol();
			caselabl(i,x,fsys);
		}
		if(sy == COLON)
			insymbol();
		else
			error(5);
		statment(as(set_se,copyset(fsys,set)));
		exittab[++(*j)] = lc;
	  	emit(10);
	}
}
reptstat(fsys)
int *fsys;
{
	register lc1;
	struct item x;
	int set[SETSIZE];

	lc1 = lc;
	insymbol();
	statment(as(set_su,copyset(fsys,set)));
	while(in(sy,al(SEMICOLN,copyset(sttbgsys,set)))) {
		if(sy == SEMICOLN)
			insymbol();
		else
			error(14);
		statment(as(set_su,copyset(fsys,set)));
	}
	if(sy == UNTILSY) {
		insymbol();
		expressn(fsys,&x);
		if(!(x.i_typ & (BOOLS | NOTYP)))
			error(17);
		emit1(11,lc1);
	} else
		error(53);
}
whilstat(fsys)
int *fsys;
{
/*
 *	modifies : code[].
 */
	register lc1,lc2;
	struct item x;
	int set[SETSIZE];

	insymbol();
	lc1 = lc;
	expressn(al(DOSY,copyset(fsys,set)),&x);
	if(!(x.i_typ & (BOOLS | NOTYP)))
		error(17);
	lc2 = lc;
	emit(11);
	if(sy == DOSY)
		insymbol();
	else
		error(54);
	statment(fsys);
	emit1(10,lc1);
	code[lc2].o_y = lc;
}
forstat(fsys)
int *fsys;
{
/*
 *	modifies : code[].
 */
	register struct tab *p;
	register struct item *x;
	register char cvt;
	struct item six;
	char i,f;
	int lc1,lc2;
	int set[SETSIZE];

	x = &six;
	insymbol();
	switch(sy) {
	default :
		skip(as(set_btdd,copyset(fsys,set)),2);
		break;
	case IDENT :
		i = loc(id);
		insymbol();
		if(i == 0)
			cvt = INTS;
		else  {
			p = &tab[i];
			if(p->t_obj == VARIABLE) {
				cvt = p->t_typ;
				emit2(0,p->t_lev,p->t_adr);
				if(!(cvt&(NOTYP|INTS|BOOLS|CHARS)))
					error(18);
			} else {
				error(37);
				cvt = INTS;
			  }
		}
	}
	switch(sy) {
	default :
		skip(as(set_tdd,copyset(fsys,set)),51);
		break;
	case BECOMES :
		insymbol();
		expressn(as(set_tdd,copyset(fsys,set)),x);
		if(x->i_typ != cvt)
			error(19);
	}
	f = 14;
	switch(sy) {
	default :
		skip(al(DOSY,copyset(fsys,set)),55);
		break;
	case DOWNTOSY :
		f = 16;
	case TOSY :
		insymbol();
		expressn(al(DOSY,copyset(fsys,set)),x);
		if(x->i_typ != cvt)
			error(19);
	}
	lc1 = lc;
	emit(f);
	if(sy == DOSY)
		insymbol();
	else
		error(54);
	lc2 = lc;
	statment(fsys);
	emit1(f + 1,lc2);
	code[lc1].o_y = lc;
}
stndproc(n,fsys)
int n;	/* key to which procedure */
int *fsys;
{
/*
 *	processes read,readln,write,writeln.
 */
	register struct tab *p;
	register struct item *x,*y;
	struct item six,siy;
	char i;
	int set[SETSIZE];

	x = &six; y = &siy;
	switch(n) {
	case 1 :	/* read */
	case 2 :	/* readln */
	   if(!iflag) {
	   	error(20);
	   	iflag = 1;
	   }
	   if(sy == LPARENT) {
		do {
		   insymbol();
		   if(sy != IDENT)
		   	error(2);
		   else {
		     i = loc(id);
		     insymbol();
		     if(i) {
			p = &tab[i];
			if(p->t_obj != VARIABLE)
				error(37);
			else {
			   x->i_typ = p->t_typ;
			   x->i_ref = p->t_ref;
			   emit2(!p->t_nrm,p->t_lev,p->t_adr);
			   switch(sy) {
			   case LBRACK :
			   case LPARENT:
			   case PERIOD :
			   	selector(as(set_crt,
			   		 copyset(fsys,set)),x);
			   }
			   if(x->i_typ&(INTS|REALS|CHARS|NOTYP))
			   	emit1(27,x->i_typ);
			   else
			   	error(40);
			}
		     }
		   }
		   test(set_crt,fsys,6);
		} while(sy == COMMA);
		if(sy == RPARENT)
			insymbol();
		else
			error(4);
	   } 
	   if(n == 2)
		emit(62);
	   break;
	case 3 :	/* write */
	case 4 :	/* writeln */
	   if(sy == LPARENT) {
		do {
		   insymbol();
		   if(sy == STRING) {
			emit1(24,sleng);
			emit1(28,strgpos);
			insymbol();
		   } else {
			expressn(as(set_ccr,
				 copyset(fsys,set)),x);
			if(x->i_typ >= ARRAYS)
				error(41);
			if(sy == COLON) {
				insymbol();
				expressn(as(set_ccr,
					 copyset(fsys,set)),y);
				if(y->i_typ != INTS)
					error(43);
				if(sy == COLON) {
					if(x->i_typ != REALS)
						error(42);
					insymbol();
					expressn(as(set_crt,
					   copyset(fsys,set)),y);
					if(y->i_typ != INTS)
						error(43);
					emit(37);
				} else
					emit1(30,x->i_typ);
			} else
				emit1(29,x->i_typ);
		     }
		} while(sy == COMMA);
		if(sy == RPARENT)
			insymbol();
		else
			error(4);
	   }
	   if(n == 4)
		emit(63);
	   break;
	}
}
