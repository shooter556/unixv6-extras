#

		/*	core dumper for local batch-
			Unix level 6.
			copyright GC inc 1976.
		*/

#include <defines.h>
#include <param.h>

	/*	useful symbolic equates..	*/

#define	usersiot	0377

#define	in	&
#define	lowbyte	0377


#define MAGIC 0407		/* magic word for checking */

#define lastdigit 7
#define of &

#define	iottrap	6

struct SYM {
	char	name[8];
	int	type;
	int	addr;  
};



struct {
	int	magic;
	int	stext;
	int	sdata;
	int	sbss;
	int	ssymtab;
	int	blanks[2];
	int	relocn;
} hdr;



struct REGS {
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	j[2];
	int	r6;
	int	r1;
	int	j1;
	int	r0;
	int	r7;
	int	psw;
}	regs;
int	*oregs[]	/* ordered register table */
{
	&regs.r0,
	&regs.r1,
	&regs.r2,
	&regs.r3,
	&regs.r4,
	&regs.r5
};



char tms[] "\n\n\txx sec execution time\n\n";
int	monstatus;	/*	current monitor status when terminated	*/

int	cpsec;
int	cored;
int	aoutd;


char	duplicated[] "\t\t\t. . . DUPLICATED . . .\t\t\t. . . DUPLICATED . . .\n";



char	nchard[]	"nchar$";
char	obuffd[]	"obuff$";
char	errord[]	"error$";
char	*magics[]	{
	"start$",
	"end$"
};


char	*errmsgs[]
{
	0,
	0,
	"quit",
	"illegal instruction",
	"bpt trap",
	0,
	"emt trap",
	"floating point error",
	0,
	"odd address for word",
	"address out of range",
	"trap trap",
	0,
	0,
	"system closedown",
	0,
	"time limit"
};


char	*monmsgs[]
{
	"too much output",
	"line too long",
	"tried to read past eof",
};




char	*begbuf;
char	*op;	/* pointer to output buffer */
char	*endbuf;


octal(w)
int	w;
{
	register char	*c;
	register int	j,i;
	i=w;
	c = (op =+ 6);
	for(j=0;j<5;j++,i=>>3) *--c = ((lastdigit of i)+'0');
	*--c = (i&1) + '0';
	if(op > endbuf) writit();
}


character(w)
int	w;
{
	register char c,*cp;
	register int i;
	i = w;
	for (w=0; w<2; w++)
	{
		c = i;
		if ( c>=040 && c < 0177 )
			*op++ = c;
		else if ( c == '\n' )
			{
				*op++ = '\\';
				*op++ = 'n';
			}
			else *op++ = '?';
		i =>> 8;
	}
	if (op >= endbuf) writit();
}


writit()
{
	register char *j,*k;
	if(op>=endbuf)
	{
		write(1,begbuf,512);
		j = begbuf; k = endbuf;
		while(k<op) *j++ = *k++;
		op = j;
	}
	else
	{
		write(1,begbuf,op-begbuf);
		op = begbuf;
	}
}


int	address	0;

dumpto ( where )
int	where;
{
	int	inbuf[8];
	int	lastline[8];
	register int i,*j,*k;
	int dupcount, same;
	dupcount = 0;
	read(cored,inbuf,020);
	k = inbuf;
	octal(address);
	*op++ = '\t';
	*op++ = '\t';
			for( k = lastline, j = inbuf;
				j < inbuf+8;
				j++ , k++ )
	{
		octal(*j);
		*op++ = '\t';
		*k = *j;	/* initialises the lastline. */
	}
	*op++ = '\t';
	for(k=inbuf; k<inbuf+8; k++) character(*k);
	/*	first line done. now dump the rest...	*/
	address =+ 020 /* bytes */;
	*op++ = '\n';
	while( address < where )
	{
		read(cored,inbuf,020);
		/* check for the line being the same */
		i = 0;
		k = inbuf;
		j = lastline;
		while(*k++ == *j++ && i < 8) i++;
		if ( i == 8 ) dupcount++;
			else dupcount = 0;
		if ( dupcount == 1 )
			writes(duplicated);
		else if ( dupcount == 0 )
		{
			octal(address);
			*op++ = '\t';
			*op++ = '\t';
			for( k = lastline, j = inbuf;
				j < inbuf+8;
				j++ , k++ )
			{
				*k = *j;	/* reinitialise lastline */
				octal(*j);
				*op++ = '\t';
			}
			*op++ = '\t';
			for (j=inbuf; j<inbuf+8; j++) character(*j);
			*op++ = '\n';
		}
		address =+ 020;
	}
}


crapout(s)
char	*s;
{
	writes(s);
	ext();
}


find(s)
char	*s;
/*	finds a symbol in the a.out file, and returns its
	address relative to the start of the core image		*/
{
	struct SYM sym;
	register char *cp1,*cp2;
	seek(aoutd,(020+(hdr.relocn ? 1 : 2)*(hdr.sdata+hdr.stext)),0);
	/* now at start of symbol table */
	while( read(aoutd,&sym,sizeof sym))
	{
		cp1 = s;
		cp2 = &sym;
		while( *cp1 == *cp2 ) if ( *cp1 ) cp1++,cp2++;
					else return(sym.addr + 02000);
	}
	crapout("sdump:symbol not found");
}


dumptrace(why)
int	why;
{
	int word;
	register char *c;
	register int i,corrupt;
	char monbuf[512];
	corrupt = 0;
	for(i=0; i<2; i++)
	{
		seek(cored,find(magics[i]),0);
		if(( read(cored,&word,2) != 2) || ( word != MAGIC )) corrupt++;
	}
	if(corrupt)
		writes("\014monitor obliterated.\n");
	else
	{
		seek(cored,find(errord),0);
		read(cored,&monstatus,2);
		seek(cored,find(obuffd),0);
		read(cored,monbuf,512);
		seek(cored,find(nchard)+2,0);
		read(cored,&word,2);
		monbuf[word] = '\0';
		writes(monbuf);	/* if there are any nulls,
					the user put them 
					there, not the monitor	*/
		/*	now the tracing output	*/
		i = 15 - cpsec;	/*time left*/
		tms[4] = i % 10 + '0';
		tms[3] = (i =/ 10) ? i + '0' : ' ';
		writes(tms);
		*op++ = '\014';	/*	form feed	*/
	}
	word = lowbyte of monstatus;
	if(why == iottrap)
		if(word == usersiot) writes("user generated iot?");
		else writes(monmsgs[word-1]);
	else
		writes(errmsgs[why-1]);
	*op++ = '\n';
}




dumpregs()
{
	register char n,r;
	register char rno;
	r = 'R';
	n = '\n';
	writes("\ncontents of registers\n======== == =========\n\n");
	for(rno='0'; rno < '6'; rno++)
	{
		*op++ = r;
		*op++ = rno;
		*op++ = '	';
		octal(*oregs[rno-'0']);
		*op++ = '	';
		character(*oregs[rno-'0']);
		*op++ = n;
	}
	writes("stack pointer	");
	octal(regs.r6);
	*op++ = '	';
	writes("\nprogram counter	");
	octal(regs.r7);
	writes("\nstatus word	");
	octal(regs.psw);
	*op++ = n;
	*op++ = n;
}



main(argc,argv)
char	**argv;
int	argc;
{
#include <user.h>
	char	buffer[532];	/* output buffer, with 20 extra chars provided */
	op = buffer;
	endbuf = buffer + 512;
	begbuf = buffer;
	if((cored = open("core",0)) == -1)
				crapout("sdump: cant open core\n");
	if((aoutd = open("m.out",0)) == -1)
				crapout("sdump: cant open m.out");
	read(cored,&u,sizeof u);
	cpsec = u.u_cpusec;
	seek(cored, 01750, 0);	/* address of register info */
	read(cored,&regs,sizeof regs);
	read(aoutd,&hdr,sizeof hdr);
	dumptrace(u.u_arg[0]);
	dumpregs();
	seek(cored,02000,0);	/* address of beginning of core image */
	writes("contents of core:\n======== == ====\n\n");
	dumpto(u.u_dsize<<6);
	writes("\ncontents of stack\n======== == =====\n\n");
	*op++ = '\n';
	address = - u.u_ssize<<6;
	dumpto(0);
	writit();
}




writes(s)
char	*s;
{
	register char *c,*d;
	c = s;
	d = op;
	while(*c)
	{
		*d++ = *c++;
		if (d < endbuf) continue;
		op = d;
		writit();
		d = op;
	}
	op = d;
}


ext()
{
	writit();
	exit(0);
}
