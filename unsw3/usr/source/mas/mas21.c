#


#include	"mas0.h"


char	*symtmp, *fbtmp, *itmp, *ltmp;
int	sfile, fbfile, ifile;

struct	iform *iptr;
int	icount;
extern	struct iform ibuf[];

extern	char listbuf[], line[];
char	*lbp, *linep;
int	lfile;

int	obfile;

extern	char inbuf[];
char	tbuf[];		/* mod011 */
int	*intptr;		/* mod005 */
char	tch;
extern	struct symbol symtab[];
extern	char maddr[];
extern	int mcode[];
extern	hashtab[];
extern	struct fb fbtable[];
extern	curfb[];
extern	char symbuf[];
extern	bytec;		/* mod001 */

int	gargc;
char	**gargv;


int	numsym;
int	lab, undef;

struct	segment segopt;

int	nextfb;

char	*fins;
int	qcompar();


int	npc, spc, pc;
int	lc;
int	operand;




mexit()				/* mod002 */
{
	signal(2,1);
	signal(3,1);
	unlink(ltmp);
	unlink(symtmp);
	unlink(fbtmp);
	unlink(itmp);
	exit();
}


main(argc, argv)
char **argv;
{
	register char *s, c;
	register n;

	signal(2,mexit);
	signal(3,mexit);	/* mod002 */

	ltmp = "/tmp/motmx.5";
	c = *(ltmp+9) = argv[0][0];	/* temp file char */
	gargc = --argc;
	gargv = ++argv;
	intptr = inbuf; *intptr = -1;			/* mod007 */

	symtmp = "/tmp/motmx.0"; *(symtmp+9) = c;
	fbtmp = "/tmp/motmx.1"; *(fbtmp+9) = c;
	itmp = "/tmp/motmx.2"; *(itmp+9) = c;

	if ((sfile = open(symtmp,0)) == -1 ||
	     (fbfile = open(fbtmp,0)) == -1 ||
	     (ifile = open(itmp, 0)) == -1)  help();
	read(sfile, hashtab, 256);

	s = symtab;
	do
	{
		n = read(sfile, s, 512);
		s =+ n; numsym =+ n;
	} while (n == 512);
	numsym =/ sizeof symtab[0];		/* mod006 */

	s = fbtable;
	do
	{
		n = read(fbfile, s, 512);
		s =+ 512;
	} while (n == 512);

	close(sfile); unlink(symtmp);
	close(fbfile); unlink(fbtmp);

	icount = read(ifile, ibuf, 512);
	iptr = ibuf;
	icount =>> 2;

	argproc();

	nextfb = 20;
	for (n=0; n<20; n++) curfb[n] = n;

	write(obfile, "L\n", 2);
	newrec();
	if (newfile() == 0) help();
	lbp = listbuf;
	lcopy(&line);

	if ((c = getc(inbuf)) == '#')
	{
		line[l_LAB] = c;
		lc++;
		getsym(getnonbl());
		lmnem();
		linep = &line[l_OPR];
		if ((c = getnonbl()) == MINUS)
		{
			*linep++ = c;
			c = getc(inbuf);
			segopt.segtype = 2;
		}
		else segopt.segtype = 1;
		segopt.segchar = *linep++ = c;
		segopt.segcount++;
		tch = getc(inbuf);
		lcomma();
	}
	else
	{
		*(intptr+1) =+ 1;		/* mod005 */
		*(intptr+2) =- 1;
	}

	for (;;)		/* main loop */
	{
		spc = pc = npc;
		undef = lab = 0;
		lc++;
		linep = line;
		if ((c = tch = getc(inbuf)) == EOF)
		{
			if (newfile()) continue;
			break;
		}

		switch (c)
		{
	    case NL:	lcomma(); continue;

	    case SP:
	    case TAB:	tch = c; break;

	    case STAR:	lcoml(); continue;

	    default:	getsym(c);
			if (DIGIT(c)) deffb(c-'0'); else lab = lookup();
			llabel();
		}

		if ((c = tch) != NL) c = getnonbl();
		if ((tch = c) == NL)
		{
			lcomm();
			continue;
		}
		if ((n = getmnem(c)) == 0)	/* get op code */
			continue;		/* zero means done */
		lcode1(n);
		outbyte(n);
		if ((n = (pc-spc)) == 2)
		{
			lcode2(operand);
			outbyte(operand);
		}
		else if (n == 3)
		{
			lcode3(operand);
			outword(operand);
		}
		lcomm();
	}

	if (iins(i_REL) == 0) syserr();
	if (bytec > 3) outrec();		/* mod001 */
	close(obfile);				/* mod000 */
	if (n = segopt.segcount) llocals(n);
	lglobals();
	unlink(itmp);
	unlink(ltmp);
	exit();
}





deffb(ind)
{
	register i;

	i = ind;
	curfb[i] = curfb[i+10];
	curfb[i+10] = nextfb++;
}


argproc()	/* mod003 */
{
	register char *of,*lf;
	register char **i;

	of = "m.out"; lf = 0;
	i = gargv;
	while (*i != -1)
	{
		if (**i == '-')
		{
			if (i[0][1] == 'l') if (i[0][2]) lf = i[0]+2; else lf = "m.lst";
			if(i[0][1] == 'o') if( i[0][2]) of = i[0]+2; else of = "/dev/null";
			*i = 0;
		}
		i++;
	}
	if ((obfile = creat(of,0600)) == -1) help();
	if (lf && (lfile = creat(lf,0600)) == -1) help();
}



newfile()
{
	register *ip, *tp;

	close(*(ip = inbuf));				/* mod005, mod011 */
	if (*(tp = tbuf))
	{
		read(*ip++ = *tp, inbuf+6, 512);
		*tp++ = 0;
		*ip++ = *tp++; *ip++ = *tp++;
		lc = *tp++; fins = *tp++;
		return(1);
	}
	if (gargc--)
	{
		fins = *gargv++;
		if(!fins) return(newfile());
		if (fopen(fins,inbuf) == -1) return(newfile());	/* mod005 */
		lc = 0;
		return(1);
	}
	return(0);
}



getmnem(ch)
char ch;
{
	register char acc;
	register opc, n;
	int adrm;

	if (getsym(ch) == 4) acc = (symbuf[3] | 040) - 0140; else acc = 0;
	lmnem();
	symbuf[3] = 0;
	n = mnemlook();
	adrm = maddr[n];
	opc = mcode[n];
	switch (adrm)
	{
    case o_INH:		pc++; break;

    case o_DUAL:	if (acc == 0) acc = getacc();
			if (acc == 2) opc =+ 0100;
			opc =+ getimm(2);
			break;

    case o_STA:		if (acc == 0) acc = getacc();
			if (acc == 2) opc =+ 0100;
			opc =+ getdirec(0);
			break;

    case o_AEI:		if (acc)
			{
				pc++;
				if (acc == 2) opc =+ 020;
			}
			else opc =+ getind(0);
			break;

    case o_PUSH:	if (acc == 0) acc = getacc();
			if (acc == 2) opc++; 
			pc++;
			break;

    case o_IDEI:	opc =+ getimm(3); break;

    case o_DEI:		opc =+ getdirec(0); break;

    case o_JMPS:	opc =+ getind(0); break;

    case o_BNCH:	operand = n = getexpr(0);
			pc =+ 2;
			if (undef) break;
			n =- pc;
			if (n < -128 || n > 127) { brerr(); n = 0; }
			operand = n & 0377;
			break;

    case o_JBRS:	operand = getexpr(0);
			if (iins(i_LONG))
			{
				opc = mcode[n+1];
				pc =+ 3;
				break;
			}
			pc =+ 2;
			n = operand - pc;
			if (n < -128 || n > 127) syserr();
			operand = n & 0377;
			break;

    case o_JEQS:	if (iins(i_LONG))
			{
				n = getexpr(0);
				operand = 03;	/* inverse jump 2 bytes */
				outbyte(opc);
				outbyte(operand);
				lcode1(opc);
				lcode2(operand);
				lcomm();
				spc =+ 2;
				outbyte(0176);	/* jmp in extended mode */
				outword(n);
				lcode1(0176);
				lcode3(n);
				tch = NL;
				linep = &line[l_C2+4];
				lcomma();
				return(0);	/* no opc zero */
			}
			symbuf[0] = 'b';
			opc = mcode[mnemlook()];
			n = getexpr(0);
			pc =+ 2;
			n =- pc;
			if ((n < -128) || (n > 127)) syserr();
			operand = n & 0377;
			break;

    default:		pseudop(adrm);
			return(0);
	}

	return(opc);
}



help()
{
	printf("help - temp files lost\n"); mexit();		/* mod002 */
}




iins(i)
char i;
{
	if (iptr->i_def == i && spc == iptr->i_pc)
	{
		if (--icount) { iptr++; return(1); }
		icount = read(ifile, ibuf, 512);
		icount =>> 2;
		iptr = ibuf;
		return(1);
	}
	return(0);
}



syserr()
{
	error("assembler error");
}


error(s)
char *s;
{
	printf("%5.d\t%s\n", lc, s);
}


brerr()
{
	error("branch error");
	line[l_UND] = 'B';
}


fbund()
{
	printf("%5.d\tUfb %s\n", lc, symbuf);
}


underr()
{
	printf("%5.d\tU %s\n", lc, symbuf);
}
