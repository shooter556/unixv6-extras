#include "old.h"

/*	this optional phase produces an
 *	allocation map.
 *	Segments are listing in the canonical order
 *	This clearly is not ideal, but without a large
 *	flat-bed plotter, no other representation
 *	seems better.
 */

map()
{

	fout = dup(1);

	printf("\n\n\nUNIX overlay linker jnr/v0.01\n\n\n");
	if(cnt_und) printf("Undefined symbols : %d\n", cnt_und);
	if(cnt_err) printf("Errors detected : %d\n");

	printf("\n\n\nRESIDENT SECTION : \n\n");

	printf("text : %6q - %6q\n", 0, res.tss);
	printf("data : %6q - %6q\n", res.tss, res.tss + res.dss);
	printf("bss  : %6q - %6q\n", res.tss + res.dss, res.tss + res.dss + res.bss);

	printf("Overlay region : %6q - %6q\n", ov_seg, ov_seg + oregion);
	printf("	  size : %d. bytes\n\n", oregion);

	printf("Segment tables : %6q - %6q\n", ov_ss, ov_se);
	printf("	  size : %d. bytes\n\n", e_segs * SIZE_SEG);

	printf("Autoload vectors : %6q - %6q \n", ov_av, ov_av + (e_autos * SIZE_AUT));
	printf("	    size : %d. bytes\n\n", e_autos * SIZE_AUT);

	describe(&u_root);
	describe(&u_pool);

	flush();
	close(fout);
	fout = 2;
	flush();
}

describe(p)
struct segment *p;
{
	register struct segment *rp;
	register struct symbol *sp;
	register char *cp;
	struct autol *ap;

	rp = p;

	printf("\n\n\nSegment # %d\n\n", rp->s_num);
	printf("text : %6q - %6q\n",
		rp->s_so + rp->s_to,
		rp->s_so + rp->s_to + rp->s_ts);
	printf("data : %6q - %6q\n",
		rp->s_so + rp->s_do,
		rp->s_so + rp->s_do + rp->s_ds);
	printf("bss  : %6q - %6q\n\n",
		rp->s_so + rp->s_do + rp->s_ds,
		rp->s_so + rp->s_bo + rp->s_bs);

	printf("files : ");
	for(cp = rp->s_files; cp; cp = cp->f_next)
		if(cp->f_type == FILE)
			printf("%s  ",cp->f_name);

	printf("\ncalls : ");
	for(cp = rp->s_calls; cp; cp = cp->c_next)
		printf("%d  ", cp->c_pnt->s_num);

	printf("\nautos : ");
	for(cp = rp->s_autos; cp; cp = cp->a_next)
		printf("%s  ", cp->a_name);

	if(rp->s_ansc)
		printf("\nAncestor : %d\n", rp->s_ansc->s_num);

	printf("\n\nSymbols : \n");

	symprint(rp->s_st);

	for(cp = rp->s_calls; cp; cp = cp->c_next)
		describe(cp->c_pnt);

}
symprint(s)
struct symbol *s;
{
	register struct symbol *sp;
	register struct autol *ap;

	for(sp = s; sp; sp = sp->srson) {
		if(sp->slson)
			symprint(sp->slson);
		if(sp->stype != EXS) {
			printf("%-8.8s   ", sp->sname);
			if(sp->stype == UND)
				printf("******  ");
			else
				printf("%6q  ",value(sp));
			switch(sp->stype) {
				case UND:	printf("undefined"); break;
				case ABS:	printf("text"); break;
				case TXT:	printf("text"); break;
				case DAT:	printf("data"); break;
				case COM:	printf("common"); break;
				case BSS:	printf("bss");
			}
			if(sp->sflags & AUTOL)
				printf(", vector %6q", ((ap = sp->svalue) - op_autos)*SIZE_AUT + ov_av);
			printf("\n");
		}
	}
}
