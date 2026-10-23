#include "old.h"

/*
 *	Symbol definition pass of the linker
 */

char	*ldefs[]	{"__o_name", "__o_segs", "__o_sege", "__o_oreg", 0};

/*	The special overlay linker symbols are
 *	defined. (__o_name, etc.) If any autoloads
 *	are specified, "__o_aut" is also entered as undefined. 
 *	Then, each segment is searched for symbol
 *	definitions. Libraries are also examined for
 *	selective inclusion. At the end, all undefined
 *	symbols are re-incarnated in the "pool,"
 *	where a final attempt is made to satisfy them.
 */

link1()
{
	register struct segment *p1;
	register struct symbol *sp;
	register char **rc;

	libact = libroot = getcore(4*LIBBS + 2);

	insym(&u_root);

	lib_over.libmod = libact;
	lib_over.libind = libcount;

	for(rc = ldefs; *rc; rc++) {
		copy8(*rc, i_sym.i_name);
		i_sym.i_type = DAT;
		i_sym.i_value = NIL;
		install(&u_root);
	}

	if(e_autos) {
		copy8("__o_aut", i_sym.i_name);
		i_sym.i_type = UND;
		install(&u_root);
	}

	for(p1 = u_root.s_calls; p1; p1 = p1->c_next)
		link1p(p1->c_pnt);

	vector(&u_root);
	lib_pool.libmod = libact;
	lib_pool.libind = libcount;

	insym(&u_pool);

	if((sp = find(&u_pool, "_end")) && sp->stype == UND) {
		copy8("_end", i_sym.i_name);
		i_sym.i_type = ABS;
		i_sym.i_value = 0;
		install(&u_pool);
		eflag++;
	}

}

link1p(p)
struct segment *p;
{
	register struct segment *p1;

	insym(p);
	for(p1 = p->s_calls; p1; p1 = p1->c_next)
		link1p(p1->c_pnt);
}

vector(p)
struct segment *p;
{
	register struct segment *p1;
	register struct symbol *sp, *up;

	p1 = p;
	while(sp = symget(p1)) 
		if(sp->stype == UND) {
			if((up = find(&u_pool,sp->sname)) == NIL)
				up = add(sp->sname);
			sp->svalue = up->svalue;
			up->svalue = sp;
		}
	for(p1 = p1->s_calls; p1; p1 = p1->c_next)
		vector(p1->c_pnt);
}

insym(p)
struct segment *p;
{
	register struct common *cp;
	register struct files *fp;
	register struct symbol *sp;

	for(cp = p->s_common; cp; cp = cp->n_next) {
		copy8(cp->n_name,i_sym.i_name);
		i_sym.i_type = COM;
		i_sym.i_value = 0;
		install(p);
	}

	for(fp = p->s_files; fp; fp = fp->f_next) 
		switch(fp->f_type) {

		case INCL:	copy8(fp->f_name, i_sym.i_name);
				i_sym.i_type = UND;
				if((sp = install(p))->stype == UND)
					sp->sflags =& ~ EXCLF;
				break;
		case EXCL:	copy8(fp->f_name, i_sym.i_name);
				i_sym.i_type = UND;
				if((sp = install(p))->stype != UND)
					x_segid(p, "cannot exclude <%s>\n", fp->f_name);
				else
					sp->sflags =| EXCLF;
				break;
		case FILE:	insymf(p, fp->f_name);
		}
}

insymf(p, fn)
char *fn;
struct segment *p;
{
	register int noff, nbno;

	if(x_open(fn) == 0) {
		insymff(p, 0, 0, 0);
		x_close();
		return;
	}

	nbno = 0;
	noff = 1;

	for(;;) {
		x_seek(&text, nbno, noff, sizeof arhd);
		if(text.size <= 0) {
			libnote(-1, 0);
			x_close();
			return;
		}
		x_fetch(&arhd, sizeof arhd);
		if(insymff(p, 1, nbno, noff + (sizeof arhd)/2)) {
			libnote(nbno, noff + (sizeof arhd)/2);
			insymff(p, 0, nbno, noff + (sizeof arhd)/2);
		}
		noff =+ (arhd.asize + sizeof arhd) >> 1;
		nbno =+ (noff >> 8) & 0377;
		noff =& 0377;
	}
}

insymff(p, libflag, bno, off)
struct segment *p;
{
	register struct segment *rp;
	register struct symbol *sp;
	register int smm;

	rp = p;

	x_readh(bno, off);
	off =+ (sizeof magic)/2 + magic.mag_txt + magic.mag_dat;
	x_seek(&text, bno, off, magic.mag_smt);

	if(libflag) {
		while(text.size > 0) {
			x_fetch(&i_sym, sizeof i_sym);
			if((i_sym.i_type & EXT) && (i_sym.i_type & 07))
				for(rp = p; rp; rp = rp->s_ansc)
						if( (sp = find(rp, i_sym.i_name)) &&
						    sp->stype == UND &&
						    !(sp->sflags & EXCLF))
								return(1);
		}
		return(0);
	}
	for(smm = 1; text.size > 0; smm++) {
		x_fetch(&i_sym, sizeof i_sym);
		if(i_sym.i_type & EXT) {
			symnum = max(symnum, smm);
			i_sym.i_type =& ~EXT;
			if( i_sym.i_type == UND && i_sym.i_value)
				i_sym.i_type = COM;
			install(rp);
		}
	}
	rp->s_ts =+ magic.mag_txt;
	rp->s_ds =+ magic.mag_dat;
	rp->s_bs =+ magic.mag_bss;
}


/*
 *	This routine does most of the work in this phase.
 *	The symbol on entry is in i_sym.
 *	A pointer to the symbol's symbol table entry
 *		is returned.
 */

struct symbol *install(p)
struct segment *p;
{
	register char *alpha, *rs, *rp;
	char *offset;

	if((alpha=find(p,i_sym.i_name)) == NIL) {
		alpha = add(i_sym.i_name);
		alpha->stype = UND;
		alpha->svalue = NIL;

		for(rp = p->s_ansc; rp; rp = rp->s_ansc)
			if(rs = find(rp,i_sym.i_name)) {
				if(rs->stype != UND && rs->stype != EXS) {
					alpha->stype = EXS;
					alpha->svalue = rs;
					break;
				}
				if(rs->stype == UND) {
					alpha->svalue = rs;
					break;
				}
			}
	}
	switch(i_sym.i_type) {

	case COM:	if(alpha->stype == COM) {
				alpha->svalue = max(alpha->svalue,i_sym.i_value);
				break;
			}
			if(alpha->stype == EXS && alpha->svalue->stype == COM) {
				alpha->svalue->svalue = max(alpha->svalue->svalue, i_sym.i_value);
				break;
			}
			if(alpha->stype != UND)
				break;
	case ABS:	offset = 0;
			goto def;
	case TXT:	offset = p->s_ts;
			goto def;
	case DAT:	offset = p->s_ds - magic.mag_txt;
			goto def;
	case BSS:	offset = p->s_bs - magic.mag_txt - magic.mag_dat;

	     def:	switch(alpha->stype) {

			case UND:	rs = alpha->svalue;
					alpha->svalue = offset + i_sym.i_value;
					alpha->stype = i_sym.i_type;
					for(;rs; rs = rp)
						if(rs->stype == UND) {
							rp = rs->svalue;
							rs->svalue = alpha;
							rs->stype = EXS;
						} else {
							x_segid(p, "ambiguous def. of <%-8.8s>\n", alpha->sname);
							break;
						}
					break;
			case COM:	alpha->svalue = i_sym.i_value;
					alpha->stype = i_sym.i_type;
					break;
			case ABS:
			case TXT:
			case DAT:
			case BSS:	x_segid(p, "mult. def. of <%-8.8s>\n", alpha->sname);
					break;
			case EXS:	if(alpha->svalue->stype == COM) {
						for(rp = p->s_ansc; rp; rp = rp->s_ansc)
							if((rs = find(rp,i_sym.i_name)) &&
								rs->stype == EXS &&
								rs->svalue == alpha->svalue )
								rs->svalue = alpha;
						alpha->svalue = offset + i_sym.i_type;
						alpha->stype = i_sym.i_type;
					} else 
						x_segid(p, "mult. def. of <%-8.8s>\n", alpha->sname);
			}
	}
	return(alpha);
}

libnote(bno, off)
{
	libact->libns[libcount].l_bno = bno;
	libact->libns[libcount++].l_off = off;
	if(libcount == LIBBS) {
		libact = libact->libnext = getcore(4*LIBBS + 2);
		libcount = 0;
	}
}
