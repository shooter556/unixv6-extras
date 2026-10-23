#include "old.h"

struct	file_stk	{
	int	fd;
	int	line_no;
	char	**argp;
	int	arg_no;
	char	*fdb;
} file_stk[NFILES];

struct	file_stk *file_pnt file_stk;

char	toke_byte	-1;
char	act_tok[ACTSZ];


/*	The command input to the linker
 *	is converted to a tree-structured
 *	invocation description.
 *	The syntax is described elsewhere.
 */

parse(argc, argv)
char **argv;
{
	register int rr1;
	register char *rr2;
	register int quit;

	rr2 = file_pnt;
	rr2->fd = -1;
	rr2->argp = ++argv;
	rr2->line_no = argc;
	if(p_toke() == NIL)
		syntax("No arguments\n");

	u_root.s_num = -1;
	u_pool.s_num = -2;

	quit = NIL;

	while(!quit)
		switch(rr1 = g_toke()) {

		case NIL:
			quit++;
			break;

		case 'X':
			xflag = 2; break;
		case 'Y':
			yflag = 2; break;
		case 'x':
			if(rr1 = act_tok[2])
				xflag = rr1 - '0';
			break;
		case 'y':
			if(rr1 = act_tok[2])
				yflag = rr1 - '0';
			break;

		case 'h':
			debug++;
			break;

		case 'o':
			if((rr1 = g_toke()) != STR)
				syntax("Overlay name missing\n");
			copy(act_tok, (overname = getcore(len(act_tok))));
			break;

		case 'n':
			nflag++;
			break;

		case 'm':
			mflag++;
			break;

		case 'R':
			if(u_root.s_files)
				syntax("ROOT already defined\n");
			attr(&u_root);
			if(u_root.s_autos)
				syntax("AUTO specified for ROOT\n");
			break;

		case 'U':
			if(u_pool.s_files)
				syntax("POOL already defined\n");
			attr(&u_pool);
			if(u_pool.s_autos)
				syntax("AUTO specified for POOL\n");
			if(u_pool.s_calls)
				syntax("CALLS specified for POOL\n");
			u_pool.s_ansc = &u_root;
			break;

		case 'S':
			if((rr1 = g_toke()) != STR || (rr1 = cvt_num(act_tok)) < 0)
				syntax("Segment syntax %s\n",act_tok);
			for(rr2 = seg_list; rr2; rr2 = rr2->s_lseg)
				if(rr2->s_num == rr1)
					syntax("Segment %d redefined\n", rr1);
			rr2 = getcore(sizeof u_root);
			rr2->s_num = rr1;
			attr(rr2);
			e_segs++;
			break;

		default:
			syntax("Horrible syntax error\n");

		}

	if(u_root.s_files == NIL)
		syntax("No ROOT specified\n");

	linkup(&u_root);

	u_pool.s_flags =| ANCHORED;

	for(rr2 = seg_list; rr2; rr2 = rr2->s_lseg)
		if((rr2->s_flags & ANCHORED) == NIL)
			syntax("Segment %d is unanchored\n", rr2->s_num);

}


attr(p)
struct segment *p;
{

	struct	files	*filelist;
	struct	common	*commlist;
	struct	files	**filetail;
	struct	autos	*autolist;
	struct	calls	*calllist;

	register int rr1;
	register char *rp;

	int	quit;
	int	t;

	filetail = &filelist;
	filelist = commlist = autolist = calllist = NIL;

	quit = NIL;
	while(!quit)
		switch(rr1 = p_toke()) {

		default:
			rp = p;
			rp->s_files = filelist;
			rp->s_lseg  = seg_list;
			rp->s_common = commlist;
			rp->s_calls = calllist;
			rp->s_autos = autolist;

			seg_list = rp;
			quit++;
			break;

		case 'l':
			rr1 = act_tok[2];
			copy("/lib/liba.a", act_tok);
			if(rr1)
				act_tok[8] = rr1;
			toke_byte = STR;

		case STR:
			rp = getcore(S_FILES);
			g_toke();
			t = FILE;
			goto strcom;

		case 'i':
			t = INCL;
			goto incexc;

		case 'e':
			t = EXCL;

		incexc:
			g_toke();
			rp = getcore(S_FILES);
			if(g_toke() != STR)
				syntax("Bad INCLUDE/EXCLUDE\n");

		strcom:
			*filetail = rp;
			filetail = &rp->f_next;
			rp->f_type = t;
			copy(act_tok, (rp->f_name = getcore(len(act_tok))));
			break;

		case 'a':
			g_toke();
			while(p_toke() == STR) {
				g_toke();
				rp = getcore(S_AUTOS);
				rp->a_next = autolist;
				autolist = rp;
				copy8(act_tok, rp->a_name);
				e_autos++;
			}
			break;

		case 'c':
			g_toke();
			while(p_toke() == STR) {
				g_toke();
				rp = getcore(S_CALLS);
				rp->c_next = calllist;
				calllist = rp;
				if((rp->c_pnt = cvt_num(act_tok)) <= 0)
					syntax("Bad CALL number %s\n", act_tok);
			}
			break;

		case 'p':
			g_toke();
			while(p_toke() == STR) {
				g_toke();
				rp = getcore(S_COMMON);
				rp->n_next = commlist;
				commlist = rp;
				copy8(act_tok, rp->n_name);
			}

		}
}


linkup(p)
struct segment *p;
{
	register struct segment *p1;
	register struct segment *q;
	register int found;

	p->s_flags =| ANCHORED;

	for(p1 = p->s_calls; p1; p1 = p1->c_next) {
		found = 0;
		for(q = seg_list; q; q = q->s_lseg)
			if(q->s_num == p1->c_pnt) {
				if(q->s_ansc)
					syntax("Segment %d calls segment %d, already called by %d\n",
							p->s_num,
							p1->c_pnt,
							q->s_ansc->s_num);
				found++;
				p1->c_pnt = q;
				q->s_ansc = p;
				break;
			}

		if(!found)
			syntax("Segment %d calls non-existent segment %d\n",
					p->s_num,
					p1->c_pnt);

		linkup(q);
	}
}




p_toke()
{
	register struct file_stk *fp;

	if(toke_byte < 0) {
		for(;;) {

			while((toke_byte = get_raw()) == NIL) {
				if(file_pnt->fd == -1)
					return(NIL);
				close(file_pnt->fd);
				--file_pnt;
			}

			if(act_tok[0] == '-' && act_tok[1] == 'd') {
				if(get_raw() == NIL)
					syntax("No indirect file\n");
				fp = ++file_pnt;
				if((fp->fd = open(act_tok, 0)) == -1)
					syntax("Cannot open indirect file\n");
				fp->line_no = 1;
				fp->arg_no = 0;
				copy(act_tok, (fp->fdb = getcore(len(act_tok))));
			} else
				break;
		}

		if(act_tok[0] == '-')
			toke_byte = act_tok[1];
		else
			toke_byte = STR;

	}

	return(toke_byte);
}

g_toke()
{
	register char r;
	r = p_toke();
	toke_byte = -1;
	return(r);
}

get_raw()
{
	register char *rp;
	register int igspc;
	register struct file_stk *fp;

	char c;

	fp = file_pnt;
	if(fp->fd == -1) {
		if(--fp->line_no == 0)
			return(NIL);
		copy(*fp->argp++, act_tok);
		fp->arg_no++;
		return(STR);
	}

	rp = act_tok;
	igspc = NIL;

	while(read(fp->fd, &c, 1) > 0) {
		if(c == '\n') {
			fp->line_no++;
			fp->arg_no = 0;
		}
		if((c == '\n' || c== ' ') && (!igspc))
			continue;
		igspc++;
		if(c == ' ' || c == '\n')
			break;
		*rp++ = c;
	}

	*rp = NIL;
	fp->arg_no++;

	return(act_tok[0]);
}

syntax(s1,s2,s3,s4)
{
	register struct file_stk *fp;

	fp = file_pnt;

	while(fp->fd != -1) {
		printf("From file %s, line %d, arg # %d\n",fp->fdb,fp->line_no,fp->arg_no);
		--fp;
	}

	printf("From command, arg # %d : ", fp->arg_no);

	printf(s1,s2,s3,s4);
	exit(1);
}


cvt_num(s)
char *s;
{
	register char *r;
	register int n,i;

	r = s;
	n = 0;
	while(i = *r++) {
		if(i > '9' || i < '0')
			return(-1);
		n = n * 10 + i - '0';
	}
	return(n);
}

