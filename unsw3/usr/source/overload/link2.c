#include "old.h"

struct	iobuf	io_o, io_t, io_d, io_s;

link2()
{
	register struct autol *ap;
	register struct calls *cp;
	register int n;

	/*
	 * final pass of the overlay linker
	 * output is directed to:
	 * 	a.out --	UNIX executable file
	 *	ov.out --	overlay file
	 *	ov.sym --	overlay debugging file
	 */

	/*
	 * allocate maximum sized linear look-up for
	 * external symbols
	 */

	vect_s = getcore(2 * symnum);

	/* open the temps for linking the resident segment */

	tname[6] = 't'; x_creat(&io_t, tname);
	tname[6] = 'd'; x_creat(&io_d, tname);

	if(xflag) {
		tname[6] = 's'; x_creat(&io_s, tname);
	}

	size_smm = 0;

	/* first, the root */

	libact = libroot; libcount = 0;

	if(xflag)
		g_out(&u_root);
	link2_1(&u_root);

	/* next, the pool */

	libact = lib_pool.libmod; libcount = lib_pool.libind;

	if(xflag)
		g_out(&u_pool);

	link2_1(&u_pool);

	/*
	 * now output the linker produced tables to 
	 * the data segment
	 */

	x_out(&io_d, overname, len(overname));
	x_out(&io_d, op_segs, SIZE_SEG * e_segs);
	for(ap = op_autos; ap < qp_autos; ap++) {
		x_outw(&io_d, 004437);
		x_outw(&io_d, ov_lp);
		x_outw(&io_d, ap->jsrseg);
		x_outw(&io_d, ap->jsraddr);
	}

	/* adjust, if text shareable */

	if(nflag && res.tss)
		for(n = res.residue; n--;)
			x_outw(&io_t, 0);

	/* terminate the temporaries */

	x_clo(&io_t);
	x_clo(&io_d);
	if(xflag)
		x_clo(&io_s);

	/*
	 * generate magic header for a.out file 
	 */

	x_creat(&io_o, "a.out");

	magic.mag_num = (nflag && res.tss) ? 0410 : 0407;
	magic.mag_txt = res.tss;
	magic.mag_dat = res.dss;
	magic.mag_bss = res.bss;
	magic.mag_smt = size_smm;
	magic.mag_rel = 1;

	x_out(&io_o, &magic, sizeof magic);

	x_copy('t', &io_o);
	x_copy('d', &io_o);
	if(xflag)
		x_copy('s',&io_o);

	x_clo(&io_o);
	tname[6] = 't'; unlink(tname);
	tname[6] = 'd'; unlink(tname);
	if(xflag) {
		tname[6] = 's'; unlink(tname);
	}


	/*
	 * a.out file is finished. Now produce the
	 *	overlay files
	 */

	x_creat(&io_o, "ov.out");
	if(yflag) {
		tname[6] = 's'; x_creat(&io_s, tname);
	}

	libact = lib_over.libmod;
	libcount = lib_over.libind;

	for(cp = u_root.s_calls; cp; cp = cp->c_next)
		link2_2(cp->c_pnt);

	x_clo(&io_o);

	if(yflag) {
		x_clo(&io_s);
		x_creat(&io_o, "ov.sym");
		x_out(&io_o, op_sss, 2*e_segs + 2);
		x_copy('s', &io_o);
		tname[6] = 's'; unlink(tname);
		x_clo(&io_o);
	}
}


/*
 *	link an overlay segment, and append its
 *	text and data portions to the overlay file.
 *	This routines descends in the canonical
 *	recursive ordering.
 */

link2_2(p)
struct segment *p;
{
	register struct segment *rp;

	rp = p;
	size_smm = 0;
	if(yflag) g_out(rp);

	tname[6] = 't'; x_creat(&io_t, tname);
	tname[6] = 'd'; x_creat(&io_d, tname);

	link2_1(rp);

	*qp_sss++ = size_smm;

	x_clo(&io_t);
	x_clo(&io_d);
	x_copy('t', &io_o);
	x_copy('d', &io_o);
	tname[6] = 't'; unlink(tname);
	tname[6] = 'd'; unlink(tname);

	for(rp = rp->s_calls; rp; rp = rp->c_next)
		link2_2(rp->c_pnt);
}

/*
 *	link-outut a file, or the modules
 *	from a library
 */

link2_1(p)
struct segment *p;
{
	register struct files *fp;
	register struct segment *rp;
	int nbno, noff;

	rp = p;
	rp->s_ts = 0;
	rp->s_ds = 0;
	rp->s_bs = 0;

	for(fp = rp->s_files; fp; fp = fp->f_next)
		if(fp->f_type == FILE) {
			if(x_open(fp->f_name) == 0) {
				linkx(rp, 0, 0);
				x_close();
			} else {

				for(;;) {
					nbno = libact->libns[libcount].l_bno;
					noff = libact->libns[libcount++].l_off;
					if(libcount == LIBBS) {
						libcount = 0;
						libact = libact->libnext;
					}
					if(nbno == -1)
						break;
					linkx(rp, nbno, noff);
				}
			}
		}
}

/*
 *	Output the global symbolic information
 *	from a segment's symbol table
 */

g_out(p)
struct segment *p;
{
	register struct segment *rp;
	register struct symbol *sp;

	rp = p;
	while(sp = symget(rp))
		if(sp->stype != EXS && sp->stype != UND) {
			x_out(&io_s, sp, 8);
			x_outw(&io_s, EXT | (sp->stype == COM ? BSS : sp->stype));
			x_outw(&io_s, value(sp));
			size_smm =+ sizeof i_sym;
		}
}

/*
 *	The final preparation to relocate
 *	a core portion. Direct the production
 *	of the text and data images
 */

linkx(p, bno, off)
struct segment *p;
{
	register struct segment *rp;
	register int smm;
	register char *offset;
	int voff, tflag;

	rp = p;
	if(rp == &u_root || rp == &u_pool)
		tflag = xflag;
	else
		tflag = yflag;
	x_readh(bno, off);
	off =+ (sizeof magic)/2;

	/*
	 *	local offset computation
	 */

	txt_j = rp->s_so + rp->s_to + rp->s_ts;
	dat_j = rp->s_so + rp->s_do + rp->s_ds - magic.mag_txt;
	bss_j = rp->s_so + rp->s_bo + rp->s_bs - magic.mag_txt - magic.mag_dat;

	/*
	 *	re-read and index symbol table
	 */

	x_seek(&text, bno, off + magic.mag_txt + magic.mag_dat, magic.mag_smt);

	for(smm = 0; text.size > 0; smm++) {
		x_fetch(&i_sym, sizeof i_sym);
		if(i_sym.i_type & EXT)
			vect_s[smm] = find(rp, i_sym.i_name);
		else
			if(tflag == 3 || (tflag == 2 && i_sym.i_name[0] != 'L')) {
				x_out(&io_s, &i_sym, 10);
				switch(i_sym.i_type) {
				default:
				case ABS:	offset = 0; break;
				case TXT:	offset = txt_j; break;
				case DAT:	offset = dat_j; break;
				case BSS:	offset = bss_j; break;
				}
				x_outw(&io_s, i_sym.i_value + offset);
				size_smm =+ sizeof i_sym;
			}
	}

	/*
	 * Now, link the text and data segments
	 * of file
	 */

	x_seek(&text, bno, off, magic.mag_txt);
	x_seek(&reloc, bno, off + (magic.mag_txt + magic.mag_dat)/2, magic.mag_txt);
	linkxx(&io_t, txt_j);

	x_seek(&text, bno, off + (magic.mag_txt)/2, magic.mag_dat);
	x_seek(&reloc, bno, off + magic.mag_txt + magic.mag_dat/2, magic.mag_dat);
	linkxx(&io_d, dat_j);

	rp->s_ts =+ magic.mag_txt;
	rp->s_ds =+ magic.mag_dat;
	rp->s_bs =+ magic.mag_bss;
}


/*
 *	finally, the "real thing."
 *	An image is born.
 */

linkxx(ichan, cbase)
struct iobuf *ichan;
{
	register struct iobuf *rc;
	register int r;
	register int t;

	rc = ichan;
	for(;;) {
		if(--text.size <= 0) 	{if(text.size<0) break; text.size++; t = x_get(&text);}
		else if(--text.nibuf<0)	{text.nibuf++; text.size++; t = x_get(&text);}
		else			t = *text.ptr++;
		if(--reloc.size<=0)	{if(reloc.size<0)x_fail("Relocation error\n"); reloc.size++; r = x_get(&reloc);}
		else if(--reloc.nibuf<0){reloc.nibuf++; reloc.size++; r = x_get(&reloc);}
		else			r = *reloc.ptr++;

		switch(r & 016) {
			case RTXT:	t =+ txt_j; break;
			case RDAT:	t =+ dat_j; break;
			case RBSS:	t =+ bss_j; break;
			case REXT:	t =+ value(vect_s[r >> 4]);
		}
		if(r & 01)
			t =- cbase;
		x_outw(rc, t);
	}
}

