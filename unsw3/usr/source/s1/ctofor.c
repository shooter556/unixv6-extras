#
/*
 *	Convert a unix object module to a format acceptable
 *	to the fortran linker.  A strictly interim solution.
 *
 *	conv infile outfile [st]
 *	st is an optional integer which specifies how much of the
 *		local symbol table is to be output:
 *		st = 0		all
 *		st = 1		omit compiler symbols 'L'
 *		st = 2		omit all symbols
 *
 *	no other arguments or defaults.
 */


#define	NIL	0
#define	GOBBLE	1024
#define	BINSIZ	124

#define	MDN	0
#define	CSN	1
#define	ISN	2
#define	TRA	3
#define	GSN	4
#define	PSN	5
#define	PVI	6

#define	GSD	001
#define	ESD	002
#define	TXS	003
#define	RLD	004
#define	ISD	005
#define	EMOD	006
#define	LMOD	007
#define	PSD	017
#define	SDR	022
#define	EDR	021

#define	SHR	01
#define	INS	02
#define	BSS	04
#define	DEF	010
#define	REL	040
#define	OVR	020
#define	GBL	0100
#define	COM	0200
#define	SPC	0400
#define	CGN	01000

long	rxin();
long	itol();

#define	N_ABS	0
#define	N_TXT	1
#define	N_DAT	2
#define	N_BSS	3

struct	{
	int	hiword;
	int	loword;
};

long	fixnames[4];
char	*fixinits[]	{
	" abs.",
	"text.",
	"data.",
	"bss.",
	NIL
};

struct	sym	{
	long	gname;
	int	gtype;
	char	*gval;
};

struct	sym	*gv;
struct	sym	*ge;

char	*memlo	&end;
char	*memhi	&end;

#define	RABS	000
#define	RTXT	002
#define	RDAT	004
#define	RBSS	006
#define	REXT	010

#define	EXT	040
#define	UND	00
#define	ABS	01
#define	TXT	02
#define	DAT	03
#define	BSS	04

#define	PCREL	01

struct	magic	{
	int	mag_num;
	char	*mag_txt;
	char	*mag_dat;
	char	*mag_bss;
	char	*mag_smt;
	int	mag_uns;
	int	mag_ent;
	int	mag_rel;
} magic;

struct	usym	{
	char	uname[8];
	int	uflags;
	int	uval;
} usym;


struct	page	{
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[256];
} page[2];

struct	{
	int	nuser;
	int	bno;
} fpage;

struct	stream	{
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	struct	page *pno;
};

int	x_infile;

char	outbuf[520];
char	binbuf[BINSIZ];
char	rldbuf[BINSIZ];
char	*rldpnt;
char	*binpnt;
int	bincode;
int	rld_dsp;
int	rld_add;
int	rld_rad;
int	rld_cnt;
struct	stream text;
struct	stream reloc;

int	stflag;

main(argc, argv)
char **argv;
{
	register long *lp;
	register char **cp;
	if(argc > 3)
		stflag = atoi(argv[--argc]);
	if(argc != 3) 
		fail("Syntax: conv infile outfile\n");

	x_open(argv[1]);
	o_cre(argv[2]);

	gv = getcore(ldiv(0, magic.mag_smt, 12)*8);
	for(lp = fixnames, cp = fixinits; *cp; )
		*lp++ = rxin(*cp++);

	pass1();
	pass2();

	x_close();
	o_close();
	return(0);
}

getcore(n)
{
	register char *w;

	if(memlo + n >= memhi) 
		if(brk(memhi = memlo + max(n+GOBBLE/2, GOBBLE)) == -1) 
			fail("Out of core\n");
	w = memlo;
	memlo =+ n;
	return(w);
}

max(a,b)
char *a, *b;
{
	return(a>b?a:b);
}

/*
 *	Routines to handle UNIX file on input
 */

o_cre(fn)
{
	if(fcreat(fn, outbuf) == -1) 
		fail("Cannot open output file\n");
}

o_close()
{
	fflush(outbuf);
}

x_close()
{}

o_purge()
{
	register int chksum, *p;

	p = binbuf;
	chksum = binpnt - binbuf + 6;
	putw(1, outbuf);
	putw(chksum, outbuf);
	putw(bincode, outbuf);
	chksum =+ (chksum >> 8) + 1 + bincode + (bincode >> 8);
	while(p < binpnt) {
		chksum =+ *p + (*p >> 8);
		putw(*p++, outbuf);
	}
	putw(-chksum & 0377, outbuf);
	binpnt = binbuf;
}

r_purge()
{
	register int chksum, *p;

	p = rldbuf;
	chksum = rldpnt - rldbuf + 6;
	putw(1, outbuf);
	putw(chksum, outbuf);
	putw(RLD, outbuf);
	chksum =+ (chksum >> 8) + 1 + RLD + (RLD >> 8);
	while(p < rldpnt) {
		chksum =+ *p + (*p >> 8);
		putw(*p++, outbuf);
	}
	putw(-chksum & 0377, outbuf);
	rldpnt = rldbuf;
}


o_igsd()
{
	bincode = GSD;
	binpnt = binbuf;
}

o_egsd()
{
	o_purge();
	bincode = ESD;
	o_purge();
}

o_isdr()
{
	bincode = SDR;
}

o_esdr()
{
	o_purge();
}

o_fin()
{
	bincode = EMOD;
	o_purge();
}

o_pgsd(gt, gn1, gn2, gf, gw)
{
	register int *r;
	if(binpnt - binbuf > 64)
		o_purge();
	r = binpnt;
	*r++ = gn1;
	*r++ = gn2;
	*r++ = (gt << 8) | (gf & 0377);
	*r++ = gw;
	binpnt = r;
}

o_psdr(gn1, gn2, gp, gf, gw)
{
	register int *r;
	if(binpnt - binbuf > 64)
		o_purge();
	r = binpnt;
	*r++ = gn1;
	*r++ = gn2;
	*r++ = (gp << 8) | (gf & 0377);
	*r++ = gw;
	binpnt = r;
}

x_open(fn)
char *fn;
{
	if((x_infile = open(fn, 0)) < 0) 
		fail("Cannot open input file\n");
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = &fpage;
	fpage.nuser = 2;
	x_seek(&text, itol(0,0), sizeof magic);
	if(text.size <= 0) 
		fail("Premature eof\n");
	x_fetch(&magic, sizeof magic);
	if(magic.mag_num != 0407) 
		fail("Input file not in proper format\n");
	if(magic.mag_rel) 
		fail("Input file has no relocation\n");
}

x_seek(sp, off, s)
struct stream *sp;
long off;
{
	register struct stream *rsp;
	register struct page *p;
	register int b;
	int n, o;

	rsp = sp;
	b = off>>8;
	o = off&0377;

	--rsp->pno->nuser;
	if((p = &page[0])->bno != b && (p = &page[1])->bno != b)
		if(p->nuser == 0 || (p = &page[0])->nuser == 0) {
			if(page[0].nuser == 0 && page[1].nuser == 0)
				if(page[0].bno < page[1].bno)
					p = &page[0];
			p->bno = b;
			seek(x_infile, b, 3);
			if((n = read(x_infile, p->buff, 512) >> 1) < 0)
				n = 0;
			p->nibuf = n;
		} else 
			fail("Internal page botch\n");
	++p->nuser;
	rsp->bno =b;
	rsp->pno = p;
	rsp->ptr = p->buff + o;
	if(s != -1)
		rsp->size = (s >> 1) & 077777;
	if((rsp->nibuf = p->nibuf - o) <= 0)
		rsp->size = 0;
}

x_fetch(w, n)
int *w;
{
	register int *rw, rn, *p;
	rn = n;
	rn =>> 1;
	rw = w;
	if((text.nibuf =- rn) >= 0) {
		if((text.size =- rn) >= 0) {
			p = text.ptr;
			do *rw++ = *p++; while(--rn);
			text.ptr = p;
			return;
		}
		text.size =+ rn;
	}
	text.nibuf =+ rn;
	do *rw++ = x_get(&text); while(--rn);
}

x_get(sp)
struct stream *sp;
{
	register struct stream *rsp;

	rsp = sp;
	if(--rsp->nibuf < 0) {
		x_seek(rsp, itol(0,256)*(rsp->bno+1),-1);
		--rsp->nibuf;
	}
	if(--rsp->size <= 0) {
		if(rsp->size < 0) 
			fail("Premature eof\n");
		++fpage.nuser;
		--rsp->pno->nuser;
		rsp->pno = &fpage;
	}
	return(*sp->ptr++);
}

/*
 *	Pass 1.
 *	read in all the symbols and do the name conversion.
 *	Then produce the gsd.
 */

pass1()
{
	register struct sym *gp;
	long now;

	x_seek(&text, itol(0, magic.mag_txt)+itol(0, magic.mag_dat)+(sizeof magic)/2, magic.mag_smt);

	o_igsd();
	
	time(&now);

	o_pgsd(MDN, now, NIL, NIL);
	o_pgsd(PVI, now, NIL, NIL);

	for(gp = gv; text.size ; gp++) {
		x_fetch(&usym, sizeof usym);
		if(((usym.uflags & EXT) == 0) && (stflag&2 || stflag && usym.uname[0]== 'L'))
			continue;
		gp->gval = usym.uval;
		gp->gtype = usym.uflags;
		gp->gname = rxin(usym.uname);
	}
	ge = gp;

	for(gp = gv; gp < ge; gp++)
		if(gp->gtype == EXT+UND && gp->gval != 0) {
			o_pgsd(PSN, gp->gname, DEF|REL|OVR|GBL|COM|BSS, gp->gval);
			o_pgsd(GSN, gp->gname, DEF|REL|COM, 0);
		}

	o_pgsd(PSN, fixnames[N_ABS], DEF|GBL|OVR, NIL);
	for(gp = gv; gp < ge; gp++)
		if(gp->gtype == EXT+ABS)
			o_pgsd(GSN, gp->gname, DEF, gp->gval);
		else if(gp->gtype == EXT+UND)
			o_pgsd(GSN, gp->gname, NIL, NIL);

	o_pgsd(PSN, fixnames[N_TXT], DEF|GBL|SHR|REL, magic.mag_txt);
	for(gp = gv; gp < ge; gp++) 
		if(gp->gtype == EXT+TXT)
			o_pgsd(GSN, gp->gname, DEF|REL, gp->gval);

	o_pgsd(PSN, fixnames[N_DAT], DEF|GBL|REL, magic.mag_dat);
	for(gp = gv; gp < ge; gp++)
		if(gp->gtype == EXT+DAT)
			o_pgsd(GSN, gp->gname, DEF|REL, gp->gval - magic.mag_txt);

	o_pgsd(PSN, fixnames[N_BSS], DEF|GBL|REL|BSS, magic.mag_bss);
	for(gp = gv; gp < ge; gp++)
		if(gp->gtype == EXT+BSS)
			o_pgsd(GSN, gp->gname, DEF|REL, gp->gval - magic.mag_txt - magic.mag_dat);

	o_egsd();
	o_isdr();
	for(gp = gv; gp < ge; gp++)
		if((gp->gtype & EXT) == 0)
			switch(gp->gtype & 037) {

			case ABS:	o_psdr(gp->gname, 0, NIL, gp->gval);
					break;
			case TXT:	o_psdr(gp->gname, 1, REL, gp->gval);
					break;
			case DAT:	o_psdr(gp->gname, 2, REL, gp->gval - magic.mag_txt);
					break;
			case BSS:	o_psdr(gp->gname, 3, REL, gp->gval - magic.mag_txt - magic.mag_dat);

			}

	o_esdr();
}

long rxin(s)
char *s;
{
	register int r50, cc;
	register c;
	int *p;
	static long zz;

	p = &zz;
	cc = 6;
	r50 = 0;
	if(*s == '_')
		s++;
	else {
		r50 = 28;
		cc--;
	}
	do {
		if(c = *s)
			s++;
		r50 =* 050;
		if(c == 0 || c == ' ')
			goto xx;
		if(c >= 'a' && c <= 'z') {
			r50 =+ c - 'a' + 1;
			goto xx;
		}
		if(c >= 'A' && c <= 'Z') {
			r50 =+ c - 'A' + 1;
			goto xx;
		}
		if(c == '$' || c == '_') {
			r50 =+ 27;
			goto xx;
		}
		if(c == '.') {
			r50 =+ 28;
			goto xx;
		}
		r50 =+ c - '0' + 30;
xx:
		if(cc == 4) {
			*p++ = r50;
			r50 = 0;
		}
	} while(--cc);
	*p = r50;
	return(zz);
}

pass2()
{
	x_seek(&text, itol(0, (sizeof magic)/2), magic.mag_txt);
	x_seek(&reloc, (itol(0, magic.mag_txt) + itol(0, magic.mag_dat) + sizeof magic) >> 1, magic.mag_txt);
	lnkobj(N_TXT, 0);

	x_seek(&text, (itol(0, magic.mag_txt) + sizeof magic) >> 1, magic.mag_dat);
	x_seek(&reloc, itol(0, magic.mag_txt) + ((itol(0, magic.mag_dat) + sizeof magic) >> 1), magic.mag_dat);
	lnkobj(N_DAT, magic.mag_txt);

	o_fin();
}

fail(s,x)
{
	printf(s, x);
	exit(1);
}

lnkobj(seg, base)
{
	register r, w, *p;

	bincode = RLD;
	p = binpnt;
	*p++ = 7;
	*p++ = fixnames[seg].hiword;
	*p++ = fixnames[seg].loword;
	binpnt = p;
	rldpnt = rldbuf;
	o_purge();

	bincode = TXS;
	rld_add = 0;
	rld_rad = base + 2;
	rld_cnt = 0;
	rld_dsp = 4;
	p = binpnt;
	*p++ = 0;
	binpnt = p;

	while(text.size) {
		w = x_get(&text);
		r = x_get(&reloc);
		if(r&PCREL)
			w =+ rld_rad;
		switch(r & 016) {

		case RABS:	if((r&PCREL)==0)
					o_rld(w, 0);
				else
					o_rld(0, 1, 3, w);
				break;

		case RTXT:	if(seg == N_TXT)
					if(r&PCREL)
						o_rld(w - rld_rad, 0);
					else
						o_rld(0, 1, 1, w);
				else
					o_rld(0, 3, r&PCREL? 14:13, fixnames[N_TXT], w);
				break;

		case RDAT:	if(seg == N_DAT)
					if(r&PCREL)
						o_rld(w - rld_rad, 0);
					else
						o_rld(0, 1, 1, w - magic.mag_txt);
				else
					o_rld(0, 3, r&PCREL? 14:13, fixnames[N_DAT], w - magic.mag_txt);
				break;

		case RBSS:	o_rld(0, 3, (r&PCREL)==0? 13:14, fixnames[N_BSS], w - magic.mag_txt - magic.mag_dat);
				break;

		case REXT:	p = &gv[r >> 4];
				o_rld(0, 3, (r&PCREL)==0? 5:6, p->gname, w);
		}
	}

	if(rld_cnt) {
		o_purge();
		r_purge();
	}
}

o_rld(w, n, t, x)
{
	register *p, *q, *s;

	p = binpnt;
	q = rldpnt;
	s = &x;

	*p++ = w;
	if(n) {
		*q++ = t | (rld_dsp << 8);
		do
			*q++ = *s++;
		while(--n);
	}
	binpnt = p;
	rldpnt = q;
	rld_dsp =+ 2;
	rld_add =+ 2;
	rld_rad =+ 2;
	if(++rld_cnt == 16) {
		rld_cnt = 0;
		o_purge();
		r_purge();
		p = binpnt;
		*p++ = rld_add;
		binpnt = p;
		rld_dsp = 4;
	}
}
