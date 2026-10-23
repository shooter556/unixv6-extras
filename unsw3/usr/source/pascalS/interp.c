#include "nglob.h"
#include "iglob.h"
#define TOS		(s+ti)	/* top-of-stack */
#define PUSH		(++ti)	/* push-down the stack */
#define POP		ti =-	/* pop-up the stack variably */
execute()
{
/*
 *	this routine is only really needed when a
 *	runtime error occurs. it prints the reason
 *	for termination, and performs a traceback over
 *	the last 10 blocks dumping local variables which
 *	are unstructured.
 *	modifies : cc,ll,blkcnt.
 */
	register h1,h2,h3;
	struct tab *p;
	struct stack_r *s;
	extern fin,fout;

	if(iflag) {
		while(ch != EOR && ch != EOF)
			nextch();
		if(ch != EOF) {
			do nextch(); 
			while((ch != EOF) && (ch != NL));
			if(ch != EOF) 
				nextch();
		} else {
			close(fin);
			fin = 0;
			cc = ll;
			nextch();
		  }
	}
	flush();
	fout = 1;
	seek(1,0,2);
	s = stack;
	if(h1 = interp()) {
	  prints("\n\nhalt at",0); printd(pc,5);
	  prints(" because of ",0);
	  switch(h1) {
	  case CASCHK:	prints("undefined case\n",0);	break;
	  case DIVCHK:	prints("division by zero\n",0);	break;
	  case INXCHK:	prints("invalid index\n",0);	break;
	  case STKCHK:	prints("storage overflow\n",0);	break;
	  case LINCHK:	prints("too much output\n",0);	break;
	  case LNGCHK:	prints("line too long\n",0);	break;
	  case REDCHK:	prints("read past eof\n",0);	break;
	  case DATCHK:	prints("invalid data\n",0);	break;
	  case CNTCHK:	prints("infinite loop ???\n",0);break;
	  }
	  h1 = bs; blkcnt = 10;
	  do {
	     putchar(NL);
	     if(--blkcnt == 0)
	     	h1 = 0;
	     h2 = (s+h1+4)->s_i;
	     if(h1) {
	     	prints(tab[h2].t_name,11);
	     	prints(" called at ",0);
	     	printl((s+h1+1)->s_i,5);
	     	putchar(NL);
	     }
	     h2 = btab[tab[h2].t_ref].b_last;
	     while(h2 != 0) {
	     	p = &tab[h2];
	     	if(p->t_obj == VARIABLE && p->t_typ <= CHARS) {
	     		prints(p->t_name,15);
	     		prints(" = ",0);
	     		h3 = p->t_nrm?h1+p->t_adr
				     :(s+h1+p->t_adr)->s_i;
			pwrite(p->t_typ, (s+h3), fld[p->t_typ], 0, 'e');
	  		putchar(NL);
	  	}
	  	h2 = p->t_link;
	     }
	     h1 = (s+h1+3)->s_i;
	  } while(h1 >= 0);
	}
	putchar(NL);
	printl(ocnt,0);	prints(" steps\n",0);
}
interp()
{
/*
 *	the interpreting machine. returns a value to its
 *	caller ('execute()') indicating the state of
 *	the machine at termination of execution.
 *
 *	modifies : pc,ocnt,lncnt,chrcnt,fld[],string[],stack[],bs
 */
	extern long labs();
	extern double fabs();
	extern double sin();
	extern double cos();
	extern double exp();
	extern double log();
	extern double sqrt();
	extern double atan();

	register struct stack_r *s;	/* ptr to base of stack */
	register ti;			/* index into stack */
	register struct order *ir;	/* current instruction */
	int h1, h2, h3, h4;		/* general counters */
	long lh1;			/* long temps */
	int sign; char gotsign;		/* used in read */
	char eof;			/* end-of-file flag */
	int displ[LMAX+1];

	s = stack; eof = 0;
	(s+1)->s_i = 0;
	(s+2)->s_i = 0;
	(s+3)->s_i = -1; (s+4)->s_i = btab[1].b_last;
	bs = displ[1] = 0;
	ti = btab[2].b_vsize - 1;
	pc = tab[(s+4)->s_i].t_adr;
	lncnt = chrcnt = 0;
	ocnt = 0;
	fld[INTS] = 10; fld[REALS] = 22;
	fld[BOOLS] = 10; fld[CHARS] = 1;
	if(maxocnt == 0)
		return(FINISH);
	for(;;) {
		ir = &code[pc++];
		if(++ocnt > maxocnt)
			return(CNTCHK);
		switch(ir->o_f) {
		case  0: /* load address */
			if(PUSH >= STACKSZE)
				return(STKCHK);
			TOS->s_a = displ[ir->o_x]+ir->o_y;
			break;
		case  1: /* load value */
			if(PUSH >= STACKSZE)
				return(STKCHK);
			h1 = (s+displ[ir->o_x]+ir->o_y);
			TOS->s_i = (h1)->s_i;
#ifdef DOUBLE
			TOS->s_dmy = (h1)->s_dmy;	/* double */
#endif
			break;
		case  2: /* load indirect */
			if(PUSH >= STACKSZE)
				return(STKCHK);
			h1 = (s+((s+displ[ir->o_x]+ir->o_y)->s_a));
			TOS->s_i = (h1)->s_i;
#ifdef DOUBLE
			TOS->s_dmy = (h1)->s_dmy;	/* double */
#endif
			break;
		case  3: /* update display */
			h1 = ir->o_y;
			h2 = ir->o_x;
			h3 = bs;
			do {
				displ[h1] = h3;
				h3 = (s+h3+2)->s_a;
			} while(--h1 != h2);
			break;
		case  8: /* built in functions */
			switch(ir->o_y) {
			case  0: /* abs(integer) */
			    TOS->s_i = labs(TOS->s_i);	break;
			case  1: /* abs(real) */
			    TOS->s_r = fabs(TOS->s_r);	break;
			case  2: /* sqr(integer) */
			    TOS->s_i =* TOS->s_i;	break;
			case  3: /* sqr(real) */
			    TOS->s_r =* TOS->s_r;	break;
			case  4: /* odd(integer) */	
			    TOS->s_b = TOS->s_i%2;	break;
			case  5: /* chr(integer) */
			    if(TOS->s_i < 0 || TOS->s_i > 63)
					return(INXCHK);	break;
			case  6: /* ord */
							break;
			case  7: /* succ */
			    TOS->s_i++;			break;
			case  8: /* pred */
			    TOS->s_i--;			break;
			case  9: /* round(real) */
			    TOS->s_i = (TOS->s_r+0.5);	break;
			case 10: /* trunc(real) */
			    TOS->s_i = TOS->s_r;	break;
			case 11: /* sin(real) */
			    TOS->s_r = sin(TOS->s_r);	break;
			case 12: /* cos(real) */
			    TOS->s_r = cos(TOS->s_r);	break;
			case 13: /* exp(real) */
			    TOS->s_r = exp(TOS->s_r);	break;
			case 14: /* ln(real) */
			    TOS->s_r = log(TOS->s_r);	break;
			case 15: /* sqrt(real) */
			    TOS->s_r = sqrt(TOS->s_r);	break;
			case 16: /* atan(real) */
			    TOS->s_r = atan(TOS->s_r);	break;
			case 17: /* eof() */
			    if(PUSH >= STACKSZE)
					return(STKCHK);
			    TOS->s_b = (ch == EOF);	break;
			case 18: /* eoln() */
			    if(PUSH >= STACKSZE)
					return(STKCHK);
			    TOS->s_b = (ch == NL);	break;
			}
			break;
		case  9: /* offset in record */
			TOS->s_a =+ ir->o_y;
			break;
		case 10: /* jump */
			pc = ir->o_y;
			break;
		case 11: /* cond. jump */
			if(!TOS->s_b)
				pc = ir->o_y;
			POP 1;
			break;
		case 12: /* switch */
			h1 = TOS->s_i; POP 1;
			h2 = ir->o_y;
			h3 = 0;
			do {
				if(code[h2].o_f != 13) {
					return(CASCHK);
				}
				if(code[h2].o_y == h1) {
					h3 = 1;
					pc = code[h2+1].o_y;
				} else h2 =+ 2;
			} while(h3 == 0);
			break;
		case 14: /* for1up */
			lh1 = (TOS-1)->s_i;
			if(lh1 <= TOS->s_i)
				(s+((TOS-2)->s_a))->s_i = lh1;
			else {
				POP 3; pc = ir->o_y;
			}
			break;
		case 15: /* for2up */
			h2 = (TOS-2)->s_a;
			lh1 = (s+h2)->s_i+1;
			if(lh1 <= TOS->s_i) {
				(s+h2)->s_i = lh1;
				pc = ir->o_y;
			} else POP 3;
			break;
		case 16: /* for1down */
			lh1 = (TOS-1)->s_i;
			if(lh1 >= TOS->s_i)
				(s+((TOS-2)->s_a))->s_i = lh1;
			else {
				pc = ir->o_y; POP 3;
			}
			break;
		case 17: /* for2down */
			h2 = (TOS-2)->s_a;
			lh1 = (s+h2)->s_i-1;
			if(lh1 >= TOS->s_i) {
				(s+h2)->s_i = lh1;
				pc = ir->o_y;
			} else POP 3;
			break;
		case 18: /* mark stack */
			h1 = btab[tab[ir->o_y].t_ref].b_vsize;
			if(ti+h1 >= STACKSZE)
				return(STKCHK);
			ti =+ 5; (TOS-1)->s_a = h1-1;
			TOS->s_a = ir->o_y;
			break;
		case 19: /* call */
			h1 = ti-(ir->o_y);
			h2 = (s+h1+4)->s_a;
			h3 = tab[h2].t_lev;
			displ[h3+1] = h1;
			h4 = (s+h1+3)->s_a+h1;
			(s+h1+1)->s_a = pc;
			(s+h1+2)->s_a = displ[h3];
			(s+h1+3)->s_a = bs;
			for(h3=ti+1;h3<=h4;h3++)
				(s+h3)->s_i = 0;
			bs = h1; ti = h4; pc = tab[h2].t_adr;
			break;
		case 20: /* index1 */
			h1 = ir->o_y;
			h2 = atab[h1].a_low;
			if(labs(TOS->s_i) > XMAX) {
				return(INXCHK);
			}
			else
				h3 = TOS->s_i;
			if(h3 < h2)
				return(INXCHK);
			if(h3 > atab[h1].a_high)
				return(INXCHK);
			POP 1;
			TOS->s_a =+ (h3-h2);
			break;
		case 21: /* index */
			h1 = ir->o_y;
			h2 = atab[h1].a_low;
			if(labs(TOS->s_i) > XMAX)
				return(INXCHK);
			else
				h3 = TOS->s_i;
			if(h3 < h2)
				return(INXCHK);
			if(h3 > atab[h1].a_high)
				return(INXCHK);
			POP 1;
			TOS->s_a =+ (h3-h2)*atab[h1].a_elsize;
			break;
		case 22: /* load block */
			h1 = TOS->s_a; POP 1;
			h2 = (ir->o_y)+ti;
			if(h2 >= STACKSZE)
				return(STKCHK);
			while(ti < h2) {
				PUSH;
				TOS->s_i = (s+h1)->s_i;
#ifdef DOUBLE
				TOS->s_dmy = (s+h1)->s_dmy;	/* double */
#endif
				h1++;
			}
			break;
		case 23: /* copy block */
			h1 = (TOS-1)->s_a;
			h2 = TOS->s_a;
			h3 = h1+(ir->o_y);
			while(h1 < h3) {
				(s+h1)->s_i = (s+h2)->s_i;
#ifdef DOUBLE
				(s+h1)->s_dmy = (s+h2)->s_dmy; /* double */
#endif
				h1++; h2++;
			}
			POP 2;
			break;
		case 24: /* literal */
			if(PUSH >= STACKSZE)
				return(STKCHK);
			TOS->s_i = ir->o_y;
			break;
		case 25: /* load real */
			if(PUSH >= STACKSZE)
				return(STKCHK);
			TOS->s_i = rconst[ir->o_y].s_i;
#ifdef DOUBLE
			TOS->s_dmy = rconst[ir->o_y].s_dmy;
#endif
			break;
		case 26: /* float */
			h1 = ti-(ir->o_y);
			(s+h1)->s_r = (s+h1)->s_i;
			break;
		case 27: /* read */
			sign = 1; gotsign = 0;
		readon:	if(eof)
				return(REDCHK);
			if(ir->o_y & (INTS|REALS))
				while(ch==' ' || ch==TAB || ch==NL)
					nextch();
			if(ch == EOF) {
				if(gotsign)
					return(DATCHK);
				eof = 1;
			}
			else switch(ir->o_y) {
			case INTS: 
				insymbol();
				switch(sy) {
				case MINUS:
					sign = -1;
				case PLUS :
					if(gotsign)
						return(DATCHK);
					gotsign = 1;
					goto readon;
				}
				if(sy != INTCON)
					return(DATCHK);
				(s+(TOS->s_a))->s_i = inum*sign;
				break;
			case REALS: 
				insymbol();
				switch(sy) {
				case MINUS:
					sign = -1;
				case PLUS :
					if(gotsign)
						return(DATCHK);
					gotsign = 1;
					goto readon;
				}
				switch(sy) {
				case INTCON :
				  (s+(TOS->s_a))->s_r = inum*sign;
					break;
				case REALCON:
				  (s+(TOS->s_a))->s_r = rnum*sign;
					break;
				default	    :
					return(DATCHK);
				}
				break;
			case CHARS: 
				(s+(TOS->s_a))->s_i =
					(ch==NL || ch==EOF)?' ':ch;
				nextch();
				break;
			}
			POP 1;
			break;
		case 28 : /* write string */
			h1 = TOS->s_i;
			h2 = ir->o_y; POP 1;
			if((chrcnt =+ h1) > LINELENG)
				return(LNGCHK);
			do putchar(stab[h2++]);while (--h1);
			break;
		case 29: /* write1 */
			h1 = fld[ir->o_y];
			if((chrcnt =+ h1) > LINELENG)
				return(LNGCHK);
			pwrite(ir->o_y, TOS, h1, 0, 'e');
			POP 1;
			break;
		case 30: /* write2 */
			h1 = TOS->s_i;
			POP 1;
			if((chrcnt =+ h1) > LINELENG)
				return(LNGCHK);
			pwrite(ir->o_y, TOS, h1, 0, 'e');
			POP 1;
			break;
		case 31: /* normal termination */
			return(FINISH);
		case 32: /* exit proc. */
			ti = bs-1; pc = (s+bs+1)->s_a;
			bs = (s+bs+3)->s_a;
			break;
		case 33: /* exit func. */
			ti = bs; pc = (s+bs+1)->s_a;
			bs = (s+bs+3)->s_a;
			break;
		case 34: /* indirect reference into stack */
			h1 = (s+(TOS->s_a));
			TOS->s_i = (h1)->s_i;
#ifdef DOUBLE
			TOS->s_dmy = (h1)->s_dmy;	/* double */
#endif
			break;
		case 35: /* logical not */
			TOS->s_b = !TOS->s_b;
			break;
		case 36: /* negate */
			if(ir->o_y == REALS)
				TOS->s_r = -TOS->s_r;
			else
				TOS->s_i = -TOS->s_i;
			break;
		case 37: /* write3 */
			h1 = (TOS - 1)->s_i;
			if((chrcnt =+h1) > LINELENG)
				return(LNGCHK);
			pwrite(REALS, TOS -2, h1, TOS->s_i.loword, 'f');
			POP 3;
			break;
		case 38: /* store */
			h1 = (s+((TOS-1)->s_a));
			(h1)->s_i = TOS->s_i; 
#ifdef DOUBLE
			(h1)->s_dmy = TOS->s_dmy; 	/* double */
#endif
			POP 2;
			break;
		case 39: /* real = real */
			POP 1; TOS->s_b = TOS->s_r == (TOS+1)->s_r;
			break;
		case 40: /* real <> real */
			POP 1; TOS->s_b = TOS->s_r != (TOS+1)->s_r;
			break;
		case 41: /* real < real */
			POP 1; TOS->s_b = TOS->s_r <  (TOS+1)->s_r;
			break;
		case 42: /* real <= real */
			POP 1; TOS->s_b = TOS->s_r <= (TOS+1)->s_r;
			break;
		case 43: /* real > real */
			POP 1; TOS->s_b = TOS->s_r >  (TOS+1)->s_r;
			break;
		case 44: /* real >= real */
			POP 1; TOS->s_b = TOS->s_r >= (TOS+1)->s_r;
			break;
		case 45: /* integer = integer */
			POP 1; TOS->s_b = TOS->s_i == (TOS+1)->s_i;
			break;
		case 46: /* integer <> integer */
			POP 1; TOS->s_b = TOS->s_i != (TOS+1)->s_i;
			break;
		case 47: /* integer < integer */
			POP 1; TOS->s_b = TOS->s_i <  (TOS+1)->s_i;
			break;
		case 48: /* integer <= integer */
			POP 1; TOS->s_b = TOS->s_i <= (TOS+1)->s_i;
			break;
		case 49: /* integer > integer */
			POP 1; TOS->s_b = TOS->s_i >  (TOS+1)->s_i;
			break;
		case 50: /* integer >= integer */
			POP 1; TOS->s_b = TOS->s_i >= (TOS+1)->s_i;
			break;
		case 51: /* Boolean or Boolean */
			POP 1; TOS->s_b =| (TOS+1)->s_b; break;
		case 52: /* integer + integer */
			POP 1; TOS->s_i =+ (TOS+1)->s_i; break;
		case 53: /* integer - integer */
			POP 1; TOS->s_i =- (TOS+1)->s_i; break;
		case 54: /* real + real */
			POP 1; TOS->s_r =+ (TOS+1)->s_r; break;
		case 55: /* real - real */
			POP 1; TOS->s_r =- (TOS+1)->s_r; break;
		case 56: /* Boolean and Boolean */
			POP 1; TOS->s_b =& (TOS+1)->s_b; break;
		case 57: /* integer * integer */
			POP 1; TOS->s_i =* (TOS+1)->s_i; break;
		case 58: /* integer div integer */
			POP 1; if((TOS+1)->s_i == 0)
				return(DIVCHK);
			TOS->s_i =/ (TOS+1)->s_i;
			break;
		case 59: /* integer mod integer */
			POP 1; if((TOS+1)->s_i == 0)
				return(DIVCHK);
			TOS->s_i =% (TOS+1)->s_i;
			break;
		case 60: /* real * real */
			POP 1; TOS->s_r =* (TOS+1)->s_r;
			break;
		case 61: /* real / real */
			POP 1; if((TOS+1)->s_r == 0)
				return(DIVCHK);
			TOS->s_r =/ (TOS+1)->s_r;
			break;
		case 62:  /* readln */
			if(ch == EOF)
				return(REDCHK);
			while(ch != NL)
				nextch();
			nextch();
			break;
		case 63: /* writeln */
			putchar(NL);
			chrcnt = 0;
			if(++lncnt > LINELMT)
				return(LINCHK);
			break;
		}
	}
}
