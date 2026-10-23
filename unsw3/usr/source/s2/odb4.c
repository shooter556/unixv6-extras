#include	"odb7.h"
#define	NOADDR	0
#define	SINGLE	1
#define	DOUBLE	2
#define	BRANCH	3
#define	MULDIV	4
#define	JSRXOR	5
#define	RTSINS	6
#define	EMTINS	7
#define	TRAPIN	8
#define	SOBINS	10
#define	OCTVAL	11
#define	ASHINS	12


struct	oplist
	{
	char	*name;
	int	optype;
	int	mask;
	int	opcode;
};


struct	scalls
	{
	int argcnt;
	char *sysnam;
};


int	*aap;
int	pcntr;
struct scalls systab[]
	{
	0,	"indir",
	0,	"exit",
	0,	"fork",
	2,	"read",
	2,	"write",
	2,	"open",
	0,	"close",
	0,	"wait",
	2,	"creat",
	2,	"link",
	1,	"unlink",
	2,	"exec",
	1,	"chdir",
	0,	"time",
	3,	"mknod",
	2,	"chmod",
	2,	"chown",
	1,	"break",
	2,	"stat",
	2,	"seek",
	0,	"getpid",
	3,	"mount",
	1,	"umount",
	0,	"setuid",
	0,	"getuid",
	0,	"stime",
	4,	"ptrace",
	0,	"?27",
	1,	"fstat",
	0,	"?29",
	1,	"smdate",
	1,	"stty",
	1,	"gtty",
	0,	"?33",
	0,	"nice",
	0,	"sleep",
	0,	"sync",
	1,	"kill",
	0,	"switch",
	0,	"?39",
	0,	"?40",
	0,	"dup",
	0,	"pipe",
	1,	"times",
	4,	"prof",
	0,	"?45",
	0,	"setgid",
	0,	"getgid",
	2,	"signal",
};



struct oplist optab[]
	{
	"adc",	SINGLE,	0000077,	0005500,
	"adcb",	SINGLE,	0000077,	0105500,
	"add",	DOUBLE,	0007777,	0060000,
	"ash",	ASHINS,	0000777,	0072000,
	"ashc",	ASHINS,	0000777,	0073000,
	"asl",	SINGLE,	0000077,	0006300,
	"aslb",	SINGLE,	0000077,	0106300,
	"asr",	SINGLE,	0000077,	0006200,
	"asrb",	SINGLE,	0000077,	0106200,
	"bcc",	BRANCH,	0000377,	0103000,
	"bcs",	BRANCH,	0000377,	0103400,
	"beq",	BRANCH,	0000377,	0001400,
	"bge",	BRANCH,	0000377,	0002000,
	"bgt",	BRANCH,	0000377,	0003000,
	"bhi",	BRANCH,	0000377,	0101000,
	"bhis",	BRANCH,	0000377,	0103000,
	"bic",	DOUBLE,	0007777,	0040000,
	"bicb",	DOUBLE,	0007777,	0140000,
	"bis",	DOUBLE,	0007777,	0050000,
	"bisb",	DOUBLE,	0007777,	0150000,
	"bit",	DOUBLE,	0007777,	0030000,
	"bitb",	DOUBLE,	0007777,	0130000,
	"ble",	BRANCH,	0000377,	0003400,
	"blo",	BRANCH,	0000377,	0103400,
	"blos",	BRANCH,	0000377,	0101400,
	"blt",	BRANCH,	0000377,	0002400,
	"bmi",	BRANCH,	0000377,	0100400,
	"bne",	BRANCH,	0000377,	0001000,
	"bpl",	BRANCH,	0000377,	0100000,
	"bpt",	NOADDR,	0000000,	0000003,
	"br",	BRANCH,	0000377,	0000400,
	"bvc",	BRANCH,	0000377,	0102000,
	"bvs",	BRANCH,	0000377,	0102400,
	"ccc",	NOADDR,	0000000,	0000257,
	"clc",	NOADDR,	0000000,	0000241,
	"cln",	NOADDR,	0000000,	0000250,
	"clr",	SINGLE,	0000077,	0005000,
	"clrb",	SINGLE,	0000077,	0105000,
	"clv",	NOADDR,	0000000,	0000242,
	"clz",	NOADDR,	0000000,	0000244,
	"cmp",	DOUBLE,	0007777,	0020000,
	"cmpb",	DOUBLE,	0007777,	0120000,
	"cnz",	NOADDR,	0000000,	0000254,
	"com",	SINGLE,	0000077,	0005100,
	"comb",	SINGLE,	0000077,	0105100,
	"dec",	SINGLE,	0000077,	0005300,
	"decb",	SINGLE,	0000077,	0105300,
	"div",	MULDIV,	0000777,	0071000,
	"emt",	EMTINS,	0000377,	0104000,
	"fadd",	RTSINS,	0000007,	0075000,
	"fdiv",	RTSINS,	0000007,	0075030,
	"fmul",	RTSINS,	0000007,	0075020,
	"fsub",	RTSINS,	0000007,	0075010,
	"halt",	NOADDR,	0000000,	0000000,
	"inc",	SINGLE,	0000077,	0005200,
	"incb",	SINGLE,	0000077,	0105200,
	"iot",	NOADDR,	0000000,	0000004,
	"jmp",	SINGLE,	0000077,	0000100,
	"jsr",	JSRXOR,	0000777,	0004000,
	"mfpi",	SINGLE,	0000077,	0006500,
	"mov",	DOUBLE,	0007777,	0010000,
	"movb",	DOUBLE,	0007777,	0110000,
	"mtpi",	SINGLE,	0000077,	0006600,
	"mul",	MULDIV,	0000777,	0070000,
	"neg",	SINGLE,	0000077,	0005400,
	"negb",	SINGLE,	0000077,	0105400,
	"nop",	NOADDR,	0000000,	0000240,
	"reset",NOADDR,	0000000,	0000005,
	"rol",	SINGLE,	0000077,	0006100,
	"rolb",	SINGLE,	0000077,	0106100,
	"ror",	SINGLE,	0000077,	0006000,
	"rorb",	SINGLE,	0000077,	0106000,
	"rti",	NOADDR,	0000000,	0000002,
	"rts",	RTSINS,	0000007,	0000200,
	"rtt",	NOADDR,	0000000,	0000006,
	"sbc",	SINGLE,	0000077,	0005600,
	"sbcb",	SINGLE,	0000077,	0105600,
	"scc",	NOADDR,	0000000,	0000277,
	"sec",	NOADDR,	0000000,	0000261,
	"sen",	NOADDR,	0000000,	0000270,
	"sev",	NOADDR,	0000000,	0000262,
	"sez",	NOADDR,	0000000,	0000264,
	"sob",	SOBINS,	0000777,	0077000,
	"sub",	DOUBLE,	0007777,	0160000,
	"swab",	SINGLE,	0000077,	0000300,
	"trap",	TRAPIN,	0000377,	0104400,
	"tst",	SINGLE,	0000077,	0005700,
	"tstb",	SINGLE,	0000077,	0105700,
	"wait",	NOADDR,	0000000,	0000001,
	"xor",	JSRXOR,	0000777,	0074000,
	".word",OCTVAL,	0177777,	0000000
};


putins(v,x)
	{
	register struct oplist *p;
	register insval, *svp;
	if ( dot&1 )
		dot--;
	svp = &symbuf[10];
	insval = v;
	seekoff();
	while(getsym(symbuf))
		if (*svp == dot)
			{
			symbuf[8] = '\0';
			putlp(symbuf);
			putlp(":");
			break;
		}
	putlp("\t");
	bytwrd = 2;
	for (p=optab;  ;p++)
		if ((insval & ~p->mask) == p->opcode)
				break;
	putlp(p->name);
	switch(p->optype)
		{
		case ASHINS :
		case MULDIV :
			insval = ((insval>>6) & 07) |		/* swap */
				 ((insval<<6) & 07700);		/* fields */
			goto dblab;
		case JSRXOR :
			insval =& 0777;
		case DOUBLE :
		dblab:
			putlp("\t");
			putad(insval>>6);
			putlp(",");
			putad(insval);
		case NOADDR :
			return;
		case RTSINS :
			insval =& 07;
		case SINGLE :
			putlp("\t");
			putad(insval);
			return;
		case SOBINS :
			putlp("\t");
			putad((insval>>6) & 07);
			putlp(",");
			insval = -(insval&077);
			goto branch;
		case BRANCH :
			putlp("\t");
		branch:
			insval =& 0377;
			if (insval&0200)
				insval =| 0177400;
			insval = dot+2+(insval<<1);
			putoff(insval);
			return;
		case TRAPIN :
			syscal(insval, x);
			return;
		case EMTINS :
			insval =& 0377;
			putlp("\t");
			putdec(insval);
			return;
		default     :
			putlp("\t");
			putoct(insval);
		}
}



putad(a)
	{
	register mode,reg,ccont;
	mode = a;
	reg = mode&07;
	mode = (mode>>3)&07;
	if (reg == 7 && mode&2)		/* pc addressing */
			{
			ccont = get(dot+bytwrd);
			if (mode&1)
				putlp("@");
			if (mode&4)
			 	putoff(ccont+dot+2+bytwrd);
			else
				{
				putlp("#");
				putoff(ccont);
				}
			bytwrd =+ 2;
			return;
			}
	reg = regs[reg];
	switch (mode)			/* general register addressing */
		{
		case 0 :
			putlp(reg);
			return;
		case 1 :
			pbreg(reg);
			return;
		case 3 :
			putlp("@");
		case 2 :
			pbreg(reg);
			putlp("+");
			return;
		case 5 :
			putlp("@");
		case 4 :
			putlp("-");
			pbreg(reg);
			return;
		case 7 :
			putlp("@");
		case 6 :
			putoff(get(dot+bytwrd));
			bytwrd =+ 2;
			pbreg(reg);
		}
}

pbreg(rr)
	{
	putlp("(");
	putlp(rr);
	putlp(")");
}

putoff(v)
char *v;
	{
	register char  *ofset;
	char i, *d;
	register int *p, *q;
	q = &symbuf[10];
	p = q--;
	i = d = -1;
	seekoff();
	while(getsym(symbuf))
		if ( (*q&040) && (ofset=v-*p) >= 0 &&
			ofset <= d && ofset < 02000 )
			{
			d = ofset;
			for (i=0; i<12; i++)
				symbol[i] = symbuf[i];
		}
	if (i== -1 || (ofset==1 && d != 0))
		{
		putoct(v);
		return;
	}
	symbol[8] = '\0';
	putlp(symbol);
	if (d)
		{
		putlp("+");
		putoct(d);
	}
}


syscal(v,x)
	{
	register inst,cnt;
	inst = v;
	putlp("\t");
	if ((cnt = inst&077) > 48)
			{
			putoct(inst);
			return;
			}
	putlp(systab[inst=cnt].sysnam);
	if (!inst && !x)
		{
		newline();
		linput();
		putins(get(dot+2),1);
		bytwrd = 4;
		return;
		}
	cnt = systab[inst].argcnt;
	while (cnt--)
		{
		newline();
		putlp("\t\t");
		putoff(get(dot+bytwrd));
		bytwrd =+ 2;
		}
}
colon()
	{
	int adrbuf[3];
	register struct oplist *p;
	register int *ap, i;
	i = 0;
	rch();
	skip();
	loop:
		symbuf[i++] = ch;
		switch(ch)
		{
		case ' '   :
		case '\t' :
		case '\n' :	symbuf[--i] = '\0';
				break;
		default	:
				if (i<7)
					{
					rch();
					goto loop;
				}
				error();
		}
	for (p=optab; p->mask != 0177777; p++)
		if (cmpst(symbuf,p->name))
			{
			aap = ap = adrbuf;
			pcntr = dot+2;
			aap++;
			*ap = p->opcode;
			valfnd = svalfnd = 0;
			if (p->optype == NOADDR)
				goto nomore;
			rch();
			skip();
			i = 06;
		switch(p->optype)
			{
			case SINGLE :
			case DOUBLE : case MULDIV :
			case ASHINS :
				aexp();
				break;
			case RTSINS :
			case JSRXOR : case SOBINS :
				getreg();
				break;
			case BRANCH :
				i = 010;
				aexp1(1);
				adrval = (adrval-pcntr) >> 1;
				if (adrval>0377 || adrval<0177401)
						error();
				svalfnd = adrval;
				break;
			case EMTINS : case TRAPIN :
				aexp1(1);
				svalfnd = adrval;
		}
		skip();
		switch(p->optype)
			{
			case DOUBLE : case ASHINS : case JSRXOR :
			case MULDIV : case SOBINS :
				if (ch != ',')
					error();
				rch();
				skip();
		}
		patch(i);
		svalfnd = 0;
		switch(p->optype)
			{
			case DOUBLE : case JSRXOR :
				aexp();
				break;
			case SOBINS :
				aexp1(1);
				if (adrval > pcntr)
					error();
				svalfnd = adrval >> 1;
				break;
			case MULDIV : case ASHINS :
				getreg();
				patch(i);
				valfnd = ((valfnd>>6) & 077) |
					 ((valfnd<<6) & 0700);
			default :
				goto nomore;
		}
		patch(i);
	nomore:
		if (ch != '\n')
			error();
		*ap =| valfnd;
		pcntr = dot;
		valfnd = 1;
		for (ap = adrbuf; ap < aap; ap++)
			{
			adrval = *ap;
			closloc();
			dot =+ 2;
		}
		dot = pcntr - 2;
		valfnd = 0;
		return;
	}
	error();
}


aexp2()
	{
	if (digit(ch) || alpha(ch))
			term(0);
	else	adrval = 0;
}


aexp()
	{
	if (ch=='@')
		{
		svalfnd =| 010;
		if (rch() == '(')
			{
			aexplb();
			if (ch == '+')
				{
				svalfnd =| 020;
				rch();
			}
			else
				{
				adrval = 0;
				svalfnd =| 060;
				incpc();
			}
		}
		else
			nbraket();
	}
	else
		direct();
}


nbraket()
	{
	switch(ch)
		{
		case '%' : case 'r' :
		case 'R' :
			rch();
			getreg();
			return;
		case '#' :
			svalfnd =| 027;
			rch();
			aexp1(1);
			incpc();
			return;
		case '-' :
			rch();
			if (ch == '(')
				{
				svalfnd =| 040;
				aexplb();
				return;
			}
			aexp1(0);
			goto def;
		default :
			aexp1(1);
		   def:
			if (ch != '(')
				{
				svalfnd =| 067;
				adrval =- (pcntr+2);
				incpc();
			}
			else
				{
				svalfnd =| 060;
				incpc();
				aexplb();
			}
	}
}


direct()
	{
	if (ch=='(')
		{
		if (svalfnd != 0)
			error();
		svalfnd =| 010;
		aexplb();
		if (ch=='+')
			{
			rch();
			svalfnd =+ 010;
		}
	}
	else
		nbraket();
}


aexp1(k)
	{
	aexp2();
	if (!k)
		adrval = -adrval;
	aexp3();
}


aexp3()
	{
	register t1;
	t1 = 0;
	for (;;)
		{
		t1 =+ adrval;
		switch(ch)
			{
			case '+' :
				rch();
				aexp2();
				continue;
			case '-' :
				rch();
				aexp2();
				adrval = -adrval;
				continue;
			default :
				adrval = t1;
				return;
		}
	}
}


aexplb()
	{
	rch();
	getreg();
	if (ch != ')')
		error();
	rch();
}


getreg()
	{
	if (match(ch, "rR%"))
		rch();
	if ((ch =- '0') > 7)
		error();
	svalfnd =| ch;
	rch();
}


incpc()
	{
	*aap++ = adrval;
	pcntr =+ 2;
}


patch(c)
	{
	valfnd = (valfnd << c) | (svalfnd & 077);
}
