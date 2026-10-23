#
/*
 *		VERSATEC printer/plotter driver
 *
 * for the VERSATEC driven by the 9600bd DL11 attached to the 'switch box'
 *
 *	BASSER DEPT. OF COMPUTER SCIENCE - april '76 - PRDL
 */

#include	"../defines.h"
#include	"../param.h"
#include	"../user.h"
#include	"../dl11.h"

/* #define	CYBER	/* deal with Cyber printer control */

#define VSPRI	9	/* 'sleep' constants */
#define VSLWAT  (PLOTLENGTH*2)
#define VSHWAT  (PLOTLENGTH*3)

#define	VSDELAY	5	/* minimum length line */

struct v {	/* character Q header */
	int	cc;	/* count */
	int	cf;	/* first */
	int	cl;	/* last */
	int	flag;	/* state flags */
	int	vcol;	/* accumulated horizontal movement */
	int	col;	/* real horizontal movement */
	int	v_nl;	/* accumulated vertical movement */
	int	nl;	/* real vertical movement */
	int	spaced_out;
#ifdef	CYBER
	int	c_rep;	/* char to be repeated */
#endif
} vs;  

#define	plotposn	col

#define FLUSH	01	/* flag bits */
#define EJECT	02
#define IND	04
#define	RUN	010
#define ASLEEP	020
#define PLOT	040
#define	OPEN	0100
#define	CLOSE	0200
#define	MODES	(IND|EJECT)

/*	special chars
 *
 *  those chars that cause delays on versatec have bit 8 set
 */
#define DO_EOT	0204
#define FORM	014
#define DC1	021
#define	DO_LF	0212	/* real line feed */
#define	DO_FORM	0214	/* real form feed */
#define	DO_CR	0215	/* used to test versatec ready */
#define	DO_DC1	0221	/* signifies start of plot line */
#define	DO_SPAC	0300	/* used to concatenate spaces */

#define LENGTH	54	/* line length of versatec page */
#define MAXCOL  132	/* char width of versatec print line */
#define PLOTLENGTH 128	/* bytes per versatec plot line */

#ifdef	CYBER		/* recognise special chars */
#define VT	013
#define	REP_S	0377	/* repeat space */
#define	REP_0	0376	/* repeat '0' */
#define	KRON_EOF 006
#define	KRON_EOR 022
#endif

#define	set	=|
#define	reset	=& ~

/*
 * open device
 */
vsopen ( dev , flag )
{
	register struct v  *vp = &vs;
	extern vsrint(), vswint();

  if ( dl11[VS] ) {	/* exclusive use */
	u.u_error = ENXIO;
	return;
  }
  dl11[VS] = LPR;
  vp->flag = (OPEN|EJECT|IND);
  if ( vp->col )  {
	vscanon( DO_CR );
  }
  vp->vcol = 8;		/* margin */
  vp->v_nl = 0;
  vp->spaced_out = 0;
#ifdef	CYBER
  vp->c_rep = 0;
#endif
  dl11v[VS].rintad = &vsrint;
  dl11v[VS].wintad = &vswint;
  if ( vp->cc )
	while ( getc( vp ) >= 0 );
  vscanon( FORM );
}




/*
 *	set/read parameters for printing and give device characteristics
 *
 *	first argument has significance as follows
 *		bit 01:	flush buffer
 *		bit 02:	eject last page on close
 *		bit 04:	indent all lines to first tab position
 *	second argument is length of page
 *	third argument is width of page
 */
vssgtty( dev , av )  int *av;
{
	register struct v  *vp;
	register  *f;

  if ( f = av )  {
	*f++ = vs.flag & MODES;
	*f++ = LENGTH;
	*f++ = MAXCOL;
	return( 1 );
  }

  vp = &vs;
  f = &vs.flag;
  *f reset MODES;
  spl4();
	if ( u.u_arg[0] & FLUSH )  {
		while ( getc( vp ) >= 0 );
		vscanon( DO_EOT );
	}else
		while ( vp->cc )  {
			*f set FLUSH;
			sleep( vp , VSPRI );
			*f reset FLUSH;
		}
  spl0();
  *f set (u.u_arg[0] & MODES);
  return( 0 );
}




/*
 * close device
 */
vsclose ( dev )
{
	register struct v *vp;
	extern vsrint();

  vp = &vs;

  vp->flag reset OPEN;

  spl4();
	while ( vp->flag & RUN )  {
		timeout( vsrint , 0 , HZ );
		vp->flag set ASLEEP;
		sleep( vp , VSPRI );
	}
  spl0();

  vp->flag reset PLOT;
  if ( vp->flag & EJECT )
	vscanon( DO_FORM );
  vp->flag set CLOSE;
}




/*
 * write to device
 */
vswrite()
{
	register c;

  while (( c = cpass() ) >= 0 )
	vscanon( c );
}




/*
 * plot on device
 */
vsplot ()
{
	register c;

  vs.flag set PLOT;
  while (( c = cpass() )>= 0 )
	vsoutput(c);
}



/*
 * put char in device Q, & deal with special chars.
 */
vscanon ( c )
  register c;
{
	register struct v *vp;
	register x;

  vp = &vs;

#ifdef	CYBER
  if ( x = vp->c_rep )  {
	vp->c_rep = 0;
	if ( x == '0' )
		do  vscanon( x );  while ( --c );
	else
		vp->vcol =+ c;	/* spaces */
	return;
  }
#endif

  if ( c > ' ' && !(c & 0200) )  {
	while ( vp->v_nl >= LENGTH )  vscanon( DO_FORM );	/* do page feeds */
	while ( vp->v_nl > vp->nl )  vscanon( DO_LF );		/* do line feeds */
	if ( vp->vcol++ < MAXCOL ) {
		if ((x = vp->vcol - ++vp->col) > 0)  {	/* do horizontal movement */
			while ( --x & 0300 )  {
				vsoutput( 0377 );
				x =- 077;
			}
			vsoutput( DO_SPAC | x );
			vp->col = vp->vcol;
		}
		vsoutput( c );
	}
  }else
	if ( c == ' ' )  vp->vcol++;	/* accumulate horiz. movement */
	else
		switch ( c ) {

			case '\n':
				vp->v_nl++;	/* accumulate vertical movement */
			case '\r':
				vp->vcol = ( vp->flag & IND ? 8 : 0 );
				return;

			case '\t':
				vp->vcol =| 7;
				vp->vcol++;
				return;
		
			case '\b':
				if ( --vp->vcol < 0 )
					vp->vcol = 0;
				return;

#ifdef	CYBER
			case VT:
				vp->v_nl =| 7;
				vp->v_nl++;
				return;

			case REP_S:
				vp->c_rep = ' ';  return;
			case REP_0:
				vp->c_rep = '0';  return;

			case KRON_EOF:
			case KRON_EOR:
#endif
			case FORM:
				if ( vp->nl == 0 )  return;	/* if top of page already */
				c = DO_FORM;

			case DO_FORM:
				while ( (vp->v_nl =- LENGTH) >= LENGTH );	/* concatenate multiple blank pages */
				if ( vp->v_nl < 0 )  vp->v_nl = 0;
				vp->nl = 0;
				goto newline;

			case DO_LF:
				vp->nl++;
			case DO_EOT:
newline:
				if ((x = VSDELAY - vp->col) > 0)
					vsoutput( DO_SPAC|x );
			case DO_CR:
				vp->col = 0;
				vsoutput ( c );
				return;
		
		}
}




/*
 * put chars on device Q up to high water mark
 */
vsoutput ( c )
{
	register struct v *vp;
	extern lbolt;

  vp = &vs;
  while ( putc ( c , vp ) )
	sleep( &lbolt , VSPRI );
  if (( vp->flag & RUN ) == 0 )  {
	vp->flag set RUN;
	spl4();
		vsrint();
	spl0();
  }
  if ( vp->cc >= VSHWAT )  {
  	vp->flag set ASLEEP;
  	sleep ( vp , VSPRI );
  }
}




/*
 * print 1 char ( called @ interrupt time )
 */
vswint()
{
	register struct v *vp;
	register c, x;

  vp = &vs;

  if ( vp->spaced_out )  {
	vp->spaced_out--;
	VSADDR->dlwb = ' ';
	return;
  }

  if  ( ( c = getc(vp) ) >= 0 )  {

	if ( vp->flag & PLOT )  {
		if ( vp->plotposn )  {
plot:			/* plot a byte */
			if ( --vp->plotposn == 0 )
				goto wait;
		}else  {
			VSADDR->dlwb = DC1;
			vp->plotposn = PLOTLENGTH-1;
			while ( !(VSADDR->dlws & DONE) );
		}
	}else
		if ( c & 0200 )  {
			if ( c & 0100 )  {
				vp->spaced_out = c & 077;
				c = ' ';
			}else  {
wait:				/* wait for ready interrupt */
				VSADDR->dlws reset IENABLE;
				VSADDR->dlrs set IENABLE;
				x = VSADDR->dlrb;	/* clear buffer */
	
				if ( vp->cc <= VSLWAT && vp->flag & ASLEEP )  {
					vp->flag reset ASLEEP;
					wakeup(vp);
				}
			}
		}

	VSADDR->dlwb = c;

  }else {

	if ( (( vp->flag reset RUN ) & (OPEN|FLUSH)) != OPEN )  {
		if ( vp->flag & PLOT && vp->plotposn )  goto plot;
		if ( vp->flag & CLOSE )
			dl11[VS] = 0;
		else
			wakeup( vp );
	}
	VSADDR->dlws reset IENABLE;
  }
}




/*
 * read interrupts here ( generated by ready flag in Versatec )
 */
vsrint()
{

  VSADDR->dlrs reset IENABLE;
  VSADDR->dlws set IENABLE;
  vswint();
}
