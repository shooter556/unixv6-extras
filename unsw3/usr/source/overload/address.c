#include "old.h"

/*	This phase is the computational work-horse
 *	of the linker. Segments are properly positioned
 *	in core, and symbols that were previously relative 
 *	to the base of their region (text, data, bss)
 *	are given true core addresses. Common is allocated,
 *	and the overlay hole is partitioned. Finally,
 *	prototype segment tables and autoload vectors
 *	are produced.
 */

address()
{

	register struct symbol *sp;
	register struct segment *p1;


	/* 
	 *	sum up common areas for each segment
	 */

	for(p1 = seg_list; p1; p1 = p1->s_lseg)
		while(sp = symget(p1))
			if(sp->stype == COM)
				p1->s_cs =+ (sp->svalue = ((sp->svalue + 1) & ~1));

	/*
	 *	maximum child tree size is size of overlay
	 *	region
	 */

	for(p1 = u_root.s_calls; p1; p1 = p1->c_next)
		oregion = max(oregion, set_child(p1->c_pnt));

	/*
	 *	establish sizes for the three resident 
	 *	regions
	 */

	res.tss = u_root.s_ts + u_pool.s_ts ;
	res.dss = u_root.s_ds + u_pool.s_ds + SIZE_SEG*e_segs + SIZE_AUT*e_autos + len(overname);
	res.bss = u_root.s_bs + u_pool.s_bs + u_root.s_cs + u_pool.s_cs + oregion;

	u_root.s_to = 0;
	u_pool.s_to = u_root.s_ts;

	if(nflag) {
		res.residue = ((res.tss + 077) & ~077) - res.tss;
		res.tss =+ res.residue;
	}

	if(res.tss && nflag)
		u_root.s_do = (res.tss + 017777) & ~017777;
	else
		u_root.s_do = res.tss;

	u_pool.s_do = u_root.s_do + u_root.s_ds;
	u_root.s_co = u_root.s_do + res.dss + oregion;
	u_pool.s_co = u_root.s_co + u_root.s_cs;
	u_root.s_bo = u_pool.s_co + u_pool.s_cs;
	u_pool.s_bo = u_root.s_bo + u_root.s_bs;

	/*
	 *	allocate prototypes
	 */

	qp_segs  = op_segs  = getcore(SIZE_SEG * e_segs);
	qp_autos = op_autos = getcore(4 * e_autos);

	if(yflag) {
		qp_sss = op_sss = getcore(2 * e_segs + 2);
		*qp_sss++ = e_segs;
	}

	/*
	 *	assign address for linker generated objects
	 */

	ov_seg = u_root.s_do + res.dss;
	ov_nm = u_root.s_do + u_root.s_ds + u_pool.s_ds;
	ov_ss = ov_nm + len(overname);
	ov_se = ov_ss + SIZE_SEG * e_segs;
	ov_av = ov_se;

	/*
	 *	now assign address to all segments,
	 *	and fill in segment tables and
	 *	autoload vector prototypes.
	 */

	set_up_seg(&u_pool);
	set_up_seg(&u_root);

	/*
	 *	insert values for linker-defined symbols
	 */

	find(&u_root, "__o_name")->svalue = ov_nm;
	find(&u_root, "__o_segs")->svalue = ov_ss;
	find(&u_root, "__o_sege")->svalue = ov_se;
	find(&u_root, "__o_oreg")->svalue = ov_seg;

	if(eflag)
		find(&u_pool, "_end")->svalue = u_pool.s_bs +u_pool.s_bo;

	if(e_autos)
		ov_lp = value(find(&u_root, "__o_aut"));
}

set_child(p)
struct segment *p;
{
	static int disk_b, disk_c;
	register struct segment *rp;
	register int n;
	register struct calls *rrp;

	rp = p;

	rp->s_dks = rp->s_ts + rp->s_ds;
	rp->s_to = 0;
	rp->s_do = rp->s_ts;
	rp->s_co = rp->s_do + rp->s_ds;
	rp->s_bo = rp->s_co + rp->s_cs;

	rp->s_crs = rp->s_bo + rp->s_bs;

	rp->s_dkb = disk_b;
	rp->s_dkc = disk_c;

	disk_c =+ rp->s_dks;
	disk_b =+ (disk_c >> 9) & 0777;
	disk_c =& 0777;

	n = 0;
	for(rrp = rp->s_calls; rrp; rrp = rrp->c_next)
		n = max(set_child(rrp->c_pnt), n);

	return(n + rp->s_crs);
}



set_up_seg(p)
struct segment *p;
{
	register struct segment *rp;
	register struct symbol *sp;
	register char *ap;

	rp = p;

	if(rp == &u_root || rp == &u_pool)
		rp->s_so = 0;
	else
		if((ap = rp->s_ansc) == &u_root)
			rp->s_so = ov_seg;
		else
			rp->s_so = ap->s_so + ap->s_crs;

	if(rp != &u_root && rp != &u_pool) {
		ap = qp_segs++;
		ap->q_by = rp->s_dkc;
		ap->q_bl = rp->s_dkb;
		ap->q_rs = rp->s_dks;
		ap->q_lc = rp->s_so;
		ap->q_hc = rp->s_so + rp->s_crs;
		ap->q_sn = rp->s_num;
	}

	for(ap = rp->s_autos; ap; ap = ap->a_next)
		if((sp = find(rp, ap->a_name)) == NIL || sp->stype == UND || sp->stype == EXS)
			x_segid(rp, "cannot autoload <%-8.8s>\n", ap->a_name);
		else
			sp->sflags =| AUTOL;

	while(sp = symget(rp))
		switch(sp->stype) {

		case UND:	if(rp != &u_pool || sp->svalue == NIL) {
					x_segid(rp, "Undefined <%-8.8s>\n", sp->sname);
					cnt_und++;
				}
				break;

		case TXT:	sp->svalue =+ rp->s_so + rp->s_to; goto cka;
		case DAT:	sp->svalue =+ rp->s_so + rp->s_do; goto cka;
		case BSS:	sp->svalue =+ rp->s_so + rp->s_bo; goto cka;
		case COM:	ap = sp->svalue; sp->svalue = rp->s_so + rp->s_co; rp->s_co =+ ap;

		     cka:	if(sp->sflags & AUTOL) {
					ap = qp_autos++;
					ap->jsrseg = ((qp_segs - op_segs) - 1)*SIZE_SEG + ov_ss;
					ap->jsraddr = sp->svalue;
					sp->svalue = ap;
				}

		}

	for(rp = rp->s_calls; rp; rp = rp->c_next)
		set_up_seg(rp->c_pnt);

}

