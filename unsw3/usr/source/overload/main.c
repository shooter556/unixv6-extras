#include "old.h"
char	*tname	"/tmp/oxyyyyy";


/* Skeletal outline of linking process:
 *	parse_____	parses the linker command, and
 *		produces the chained segment tree
 *		which drives the rest of the process
 *	link1_____	reads each file, recording definitions.
 *		Determination of library inclusion is
 *		done here. The result is a symbol table
 *		for each segment, and a list of library
 *		modules.
 *	address_______ Assigns absolute core address to the
 *		various symbols, computes the prototype
 *		segment tables and autoload vectors, and
 *		allocates common.
 *	map___	is an optional phase which prints an
 *		allocation map.
 *	link2_____	re-reads files and included library modules,
 *		generating a UNIX executable file "a.out",
 *		an overlay file "ov.out", and optionally,
 *		an overlay symbolic file, "ov.sym"
 */


main(argc, argv)
char **argv;
{
	register int pid;
	register char *cp;
	extern int d_exit();

	fout = 2;

	memlow = memhigh = sbrk(0);


	if((signal(SIGINT, 1) & 01) == 0)
		signal(SIGINT, d_exit);

	copy("ov.out", (overname = getcore(8)));

	pid = getpid();
	for(cp = &tname[12]; cp > &tname[7]; pid =>> 3)
		*--cp = (pid & 07) + '0';

	parse(argc, argv);
	link1();
	address();
	if(mflag)
		map();
	link2();

	chmod("a.out", 0755);
	chmod("ov.out", 0644);
	chmod("ov.sym", 0644);

}

ferror(s,s1,s2,s3)
char *s;
{
	printf(s,s1,s2,s3);
	exit(1);
}

d_exit()
{
	unlink("a.out");
	unlink("ov.out");
	unlink("ov.sym");

	tname[6] = 't'; unlink(tname);
	tname[6] = 'd'; unlink(tname);
	tname[6] = 's'; unlink(tname);

	exit(1);
}

/*
 *	getcore_______ dynamically allocates storage,
 *	causing the core image of the linker to grow.
 *	No size restriction other than the total amount
 *	of core available is imposed by the linker.
 */

getcore(n)
{
	register char *rp;
	while(memlow + n >= memhigh)
		if(brk(memhigh =+ GOBBLE) == -1)
			ferror("No more core\n");
	rp = memlow;
	memlow =+ n;
	return(rp);
}

copy(s1, s2)
char *s1,*s2;
{
	register char*r1,*r2;
	r1 = s1; r2 = s2;
	while(*r2++ = *r1++);
}

copy8(s1,s2)
char *s1,*s2;
{
	register char *r1,*r2;
	register int n;
	n = 8;
	r1 = s1; r2 = s2;
	do if(*r2++ = *r1) r1++; while(--n);
}


len(s)
char *s;
{
	register char *r;
	register int n;

	n = 0; r = s;
	do n++; while(*r++);
	return(++n & ~1);
}

max(a,b)
{
	return(a>b?a:b);
}

/*
 * 	value_____ returns the run-time address for its
 *	symbol table argument. Intra-segmental references
 *	are always to the true core address. Extra-segmental
 *	references to an autload________ symbol are sent to its
 *	autoload vector.
 */

value(s)
struct symbol *s;
{
	register struct symbol *sp;
	register struct autol *ap;
	register char *cp;

	sp = s;
	if(sp->stype == EXS) {
		sp = sp->svalue;
		ap = sp->svalue;
		if(sp->sflags & AUTOL) 
			return( (ap-op_autos)*SIZE_AUT + ov_av);
		return(ap);
	}
	cp = sp->svalue;
	if(sp->sflags & AUTOL)
		return(cp->jsraddr);
	return(cp);
}

/*
 *	symget______ is a non-recursive co-routine that
 *	returns all the symbols in a segment's
 *	symbol table
 */

struct symbol *symget(head)
struct segment *head;
{
	static struct symbol *act, *anc;
	register struct symbol *tp, *active;

	if((active = act) == NIL)
		return(act = head->s_st);

	for(;;) {
		if((active->sflags & 07) == 0) {
			active->sflags++;
			if(tp = active->srson) {
				active->srson = anc;
				anc = active;
				return(act = tp);
			}
		}
		if(active->sflags & 01) {
			active->sflags++;
			if(tp = active->slson) {
				active->slson = anc;
				anc = active;
				return(act = tp);
			}
		}
		if(active->sflags & 02) {
			active->sflags =& ~07;
			tp = active;
			if((active = anc) == NIL)
				return(act = NIL);
			if(active->sflags & 01) {
				anc = active->srson;
				active->srson = tp;
			} else {
				anc = active->slson;
				active->slson = tp;
			}
		}
	}
}


/*
 *	open____, get___, seek____, and fetch_____ are variations
 *	on similar routines in the original UNIX
 *	linker. They facilitate efficient nearly-
 *	sequential access to a file.
 */


x_open(fn)
char *fn;
{
	x_stat++;
	if((x_infile = open(x_inname = fn, 0)) < 0)
		x_fail("Cannot open\n");
	page[0].bno = page[1].bno = -1;
	page[0].nuser = page[1].nuser = 0;
	text.pno = reloc.pno = &fpage;
	fpage.nuser = 2;
	x_seek(&text, 0, 0, 2);
	if(text.size <= 0)
		x_fail("Premature eof\n");
	if(x_get(&text) == ARCMAGIC) {
		x_stat++;
		arhd.aname[0] = 0;
		return(1);
	}
	return(0);
}

x_get(sp)
struct stream *sp;
{
	register struct stream *rsp;

	rsp = sp;
	if(--rsp->nibuf < 0) {
		x_seek(rsp, rsp->bno + 1, 0, -1);
		--rsp->nibuf;
	}
	if(--rsp->size <= 0) {
		if(rsp->size < 0)
			x_fail("Premature eof\n");
		++fpage.nuser;
		--rsp->pno->nuser;
		rsp->pno = &fpage;
	}
	return(*sp->ptr++);
}

x_seek(sp, blk, o, s)
struct stream *sp;
{
	register struct stream *rsp;
	register struct page *p;
	register int b;
	int n;

	rsp = sp;
	b = blk + ((o >> 8) & 0377);
	o =& 0377;

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
			x_fail("No pages\n");
	++p->nuser;
	rsp->bno = b;
	rsp->pno = p;
	rsp->ptr = p->buff + o;
	if(s != -1)
		rsp->size = (s >> 1) & 077777;
	if((rsp->nibuf = p->nibuf - o) <= 0)
		rsp->size = 0;
}

x_fetch(w, n)
int *w;
{	register int *rw, rn, *p;
	rn = n;
	rn =>> 1;
	rw = w;
	if((text.nibuf =- rn) >= 0)
		if((text.size =- rn) >= 0) {
			p = text.ptr;
			do *rw++ = *p++; while(--rn);
			text.ptr = p;
			return;
		} else
			text.size =+ rn;
	text.nibuf =+ rn;
	do *rw++ = x_get(&text); while(--rn);
}


x_close()
{
	close(x_infile);
	x_stat = 0;
}


x_fail(s)
{
	x_segid(NIL, s);
	d_exit();
}

x_segid(p, m, n)
struct segment *p;
char *m, *n;
{
	if(p != NIL)
		printf("%d ", p->s_num);
	if(x_stat)
		printf("%s ", x_inname);
	if(x_stat > 1)
		printf("(%-8.8s) ",arhd.aname);
	printf(": ");
	printf(m,n);
	cnt_err++;
}

x_readh(bno, off)
{
	x_seek(&text, bno, off, sizeof magic);
	x_fetch(&magic, sizeof magic);
	if(magic.mag_num != 0407) 
		x_fail("Bad format\n");
	if(magic.mag_rel & RELFLG) 
		x_fail("No relocation\n");

	magic.mag_txt = (magic.mag_txt + 1) & ~1;
	magic.mag_dat = (magic.mag_dat + 1) & ~1;
	magic.mag_bss = (magic.mag_bss + 1) & ~1;
}


/*
 *	The following output routines are
 *	called from the last pahse of the linker.
 */

x_creat(b, f)
struct iobuf *b;
{	register struct iobuf *rb;
	rb = b;
	if((rb->io_fd = creat(f, 0400)) == -1) {
		printf("Cannot creat %s\n", f);
		d_exit();
	}
	rb->io_un = 256;
	rb->io_nx = rb->io_bf;
}

x_clo(b)
struct iobuf *b;
{	register struct iobuf *rb;
	rb = b;
	write(rb->io_fd, rb->io_bf, (256 - rb->io_un) << 1);
	close(rb->io_fd);
}

x_copy(c, b)
struct iobuf *b;
{	register struct iobuf *rb;
	register int n,m;
	int	cbb[256];

	rb = b;
	tname[6] = c;
	if((m = open(tname, 0)) == -1) {
		printf("Cannot shuffle\n");
		d_exit();
	}
	while(n = read(m, cbb, 512))
		x_out(rb, cbb, n);
	close(m);
}

x_out(b, w, c)
struct iobuf *b;
int *w;
{	register struct iobuf *rb;
	register int *rw;
	register int rc;

	rb = b; rw = w; rc = c;
	rc =>> 1;
	while(rc--) {
		*rb->io_nx++ = *rw++;
		if(--rb->io_un == 0) {
			write(rb->io_fd, rb->io_bf, 512);
			rb->io_un = 256; rb->io_nx = rb->io_bf;
		}
	}
}

x_outw(b, w)
struct iobuf *b;
{	register struct iobuf *rb;
	rb = b;
	*rb->io_nx++ = w;
	if(--rb->io_un == 0) {
		write(rb->io_fd, rb->io_bf, 512);
		rb->io_un = 256; rb->io_nx = rb->io_bf;
	}
}

