#
/*
 *	     DZ-11 driver
 *	     ------------
 *
 *		Written to handle single dz - `carrier'|`ring' support non-existent
 *
 *					Piers Lauder
 *					SYDNEY UNIVERSITY
 *					July 1977
 *
 *		Re-written to handle multiple dz's and `carrier'|`ring'.
 *
 *					Ian Johnstone
 *					UNSW
 *					December 1977
 *					January  1978
 */


/* ( no messages )
#define	MESSAGES	/* if ya want to be bombarded with messages
			   if OVERUN/PARITY errors are detected !! */
		/* INITIALIZATION of dz[] depends on CARRIER & RING */
#define	CARRIER		/* define if any lines to be used with carrier */
#define	RING		/* define if any dialup lines in use */
		/* INITIALIZATION of dz[] depends on CARRIER & RING */

#ifdef	RING
/*#define	DEBUG		/* define to debug info for dialup lines */
			/* costs approximately 128 bytes */
#endif	RING

#include	"../defines.h"
#include	"../param.h"
#ifdef	DEBUG
#ifdef	AUSAML
#include	"../lnode.h"
#endif	AUSAML
#include	"../systm.h"
#endif	DEBUG
#include	"../conf.h"
#include	"../user.h"
#include	"../tty.h"
#include	"../proc.h"


#define	NDZ		1		/* no. of dz-11s */
#define NLINES		8*NDZ		/* total no. of lines available */
#define	TSCANRATE	2		/* scan silo and modem lines every 2 'tics' */
					/* MUST always be >= 2   ALWAYS  */

#ifdef	RING
#define	RESPONDTIME	(25*HZ)		/* seconds allowed for carrier to appear
					   after answering the phone */
#define	CARRIERTIME	(1*HZ)		/* 1 second allowed for carrier to drop
					   before hanging up !!	*/
#endif	RING

#define FLUSHTIME	5		/* allow contoller this many clock ticks
					   to send buffered characters before
					   setting speeds  */

#define SSPEED		 12		/* standard speed 4800 bd */

struct	dz			/* one for each dz-11 */
{
	int	*dzaddr;	/* address of control registers for this dz-11 */
#ifdef	CARRIER|RING
	char	carrier;	/* set bits for lines with carrier operational */
#endif	CARRIER|RING
#ifdef	RING
	char	ring;		/* set bits for lines with dial in modems */
	char	active;		/* flags for active dialup lines */
#endif	RING
	char	openl;		/* flags for open lines */
	char	sopen;		/* set bits for lines with exclusive use */
				/* that is only ONE `open' allowed (e.g. LA180 line) */
	unsigned pyerrors;	/*  number of parity errors on input */
	unsigned overrors;	/*  number of overrun errors on input */
}
	dz[ NDZ ]
{
	0160040, 0375, 0374, 0, 0, 0, 0, 0, /* initialization depends on RING & CARRIER */
};

  int	dzscanning;		/* true when carrier|ring|receive scanning in operation */
				/* equal to total number of `open' lines */

/*
 *	DZ11 register layout
 */

struct	dzr_read	{
	int	dzcsr;		/* r/w */
	int	dzrbuf; 	/* no bit, byte, or tst ops */
	char	dztcr;		/* r/w */
	char	dzdtr;		/* r/w */
	char	dzring;
	char	dzcarr;
};
struct	dzr_write	{
	int	dzfill1;
	int	dzlpr;		/* no bit or byte ops */
	char	dzfill2;
	char	dzfill3;
	char	dztbuf; 	/* no bit ops */
	char	dzbrk;		/* no bit ops */
};

/*
 *	register control bits
 */
#define TRDY		0100000 	/* dzcsr */
#define TIE		040000
#define SA		020000
#define SAE		010000
#define TLINE		03400
#define RDONE		0200
#define RIE		0100
#define MSE		040
#define CLR		020

#define RCVR_ON 	010000		/* dzlpr */
#define S9600		07000
#define S300		02400
#define S134_5		01400
#define S110		01000
#define ODD_PAR 	0300
#define EVN_PAR 	0100
#define TWOSBIT		040
#define C8BIT		030
#define C7BIT		020
#define	C6BIT		010
/* #define IBM2741		RCVR_ON|S134_5|ODD_PAR|C6BIT	/* if you must */

#define	RERROR		070000		/* dzrbuf */
#define OVR_RUN 	040000	
#define FRAME		020000
#define PARITY		010000
#define LINE_NO 	03400

/*
 *	table to map UNIX standard speeds to DZ11 speeds
 *	illegal speeds are ignored.
 */
char	dzspeedmap[16]
{
	    0		/* 0 - zero */
	, 020		/* 1 - 50 */
	, 021		/* 2 - 75 */
	, 022		/* 3 - 110 */
	, 023		/* 4 - 134.5 */
	, 024		/* 5 - 150 */
	,0200		/* 6 - 200 -- ## ILLEGAL ## */
	, 025		/* 7 - 300 */
	, 026		/* 8 - 600 */
	, 027		/* 9 - 1200 */
	, 030		/* 10 - 1800 */
	, 032		/* 11 - 2400 */
	, 034		/* 12 - 4800 */
	, 036		/* 13 - 9600 */
	, 031		/* 14 - ext A - maps to 2000 */
	, 037		/* 15 - ext B - maps to 19200 */
};



struct	tty	dz11[NLINES];	/* one TTY structure per line */

char	dzdelays[ NLINES ];	/* Each entry contains a signed clock tick
				   count.  Accuracy is limited to TSCANRATE.
				   If entry <= zero then no timeout in force.
				   Necessary to reduce timeout usage when
				   supporting multiple DZ's			*/
#ifdef	RING
int	dzringt[ NLINES ];	/* Each entry contains a clock tick count.
				   Used to control modem with respect to
				   the CARRIER signal.				*/
#endif	RING


/*
 *	open a DZ11 line
 */
dzopen( dev , flag )
{
	register struct tty *tp;
	register struct dz *dzp;
	register t_bit;
	extern  dzstart(), dzscan();

	if( dev.d_minor >= NLINES ) 
	{
		u.u_error = ENXIO;
		return;
	}
	dzp = &dz[dev.d_minor>>3];

	t_bit = (1 << (dev.d_minor&07));

	if( (dzp->sopen&t_bit) && (dzp->openl&t_bit) )
	{
		u.u_error = EOPENFAIL;
		return;
	}
	tp = &dz11[dev.d_minor];
	if( u.u_procp->p_ttyp == 0 )  u.u_procp->p_ttyp = tp;

	if( (tp->t_state & ISOPEN) == 0 )
	{
		tp->t_dev = dev;
		tp->t_addr = &dzstart;
		tp->t_speeds = SSPEED|(SSPEED<<8);
		tp->t_flags = ODDP|EVENP|XTABS|CRMOD|RAW;

		if( dzp->openl == 0 ) 
			dzp->dzaddr->dzcsr =| (TIE|RIE|SAE|MSE);	/* reciever interrupt every 16 chars */

		dzp->openl =| t_bit;

		if( dzscanning == 0 ) 
			timeout( &dzscan , 0 , TSCANRATE ); /* start scanning */
		dzscanning++;
		spl5();
#ifdef	RING|CARRIER
		if(
#ifdef	RING
		    !(dzp->ring & t_bit)
#endif	RING
#ifdef	RING&CARRIER
		  &&
#endif	RING&CARRIER
#ifdef	CARRIER
		    !(dzp->carrier & t_bit)
#endif	CARRIER
		)
#endif	RING|CARRIER
			dzp->dzaddr->dzdtr =| t_bit;		/* turn on DTR for non-dialup lines */
#ifdef	RING|CARRIER
		else
		{
#ifdef	DEBUG
			printf("%d wo%d\n",time.loint,dev.d_minor);
#endif	DEBUG
			while( !(dzp->dzaddr->dzcarr&t_bit) )
			{
				tp->t_state =| WOPEN;
				sleep( &tp->t_rawq , TTIPRI );
			}
#ifdef	DEBUG
			printf("%d op%d\n",time.loint,dev.d_minor);
#endif	DEBUG
		}
#endif	RING|CARRIER
		spl0();

		tp->t_state = (ISOPEN|CARR_ON|SSTART);

		dzparam( tp , 1 );

	}
}


/*
 *	scan open lines for:
 *			1.  carrier|ring as appropriate
 *			2.  process timeouts as dictated by dzdelays
 *			3.  for input by calling dzrint
 */
dzscan()	/* at spl5 */
{
	register struct dz *dzp;
	struct tty *tp;
	int *p;
	extern dzstart(),dzrint(),dzscan();

#ifdef	CARRIER|RING
 /*
  *	scan open dialup/carrier lines.
  */

  tp = &dz11[0];
#ifdef	RING
  p  = &dzringt[0];
#endif	RING
  for( dzp = &dz[0] ; dzp < &dz[NDZ] ; dzp++ )
  {

	register struct dzr_read *dzaddr;
	char scanl,scanc;

	dzaddr = dzp->dzaddr;
  if(
#ifdef	RING
	(scanl = (dzp->openl & dzp->ring   ) )
#endif	RING
#ifdef	RING&CARRIER
	  |
#endif	RING&CARRIER
#ifdef	CARRIER
	(scanc = (dzp->openl & dzp->carrier) )
#endif	CARRIER
     )	{

	  register t_bit;

  	  for ( t_bit = 1  ;  t_bit & 0377  ;  t_bit =<< 1 , tp++, p++ )
#ifdef	RING
	     if( scanl & t_bit )
	     {
		/* now have an open `dialup' line */
		if( dzp->active & t_bit )
		{
			if( !(dzaddr->dzcarr & t_bit) )
			{
			     if( *p == 0 ) *p = CARRIERTIME;
				else if( (*p =- TSCANRATE) <= 0 )
				     {
				     *p = 0;	/* disable */
#ifdef	DEBUG
				     printf("%d hu%d\n",time.loint,tp->t_dev.d_minor);
#endif	DEBUG
				     dzaddr->dzdtr =& ~t_bit; /* hang up the phone */
				     signal( tp , SIGHUP );
				     flushtty( tp );
				     dzaddr->dztcr =& ~t_bit; /* disable transmit */
				     dzp->active =& ~t_bit;
				     }
			}
			else
			{
				if( tp->t_state&WOPEN ) wakeup( &tp->t_rawq );
				*p = 0;
				if( !( tp->t_state&TIMEOUT ) && ( tp->t_outq.c_cc ) )
					dzaddr->dztcr =| t_bit;
			}
		}
		else
		{
 			  if( (!(dzaddr->dzdtr & t_bit)) && (dzaddr->dzring & t_bit) )
			  {
				dzaddr->dzdtr =| t_bit; /* answer the phone */
				*p = RESPONDTIME;
			  	dzp->active  =| t_bit;
		  		dzaddr->dztcr =| t_bit;
#ifdef	DEBUG
				printf("%d ap%d\n",time.loint,tp->t_dev.d_minor);
#endif	DEBUG
			  }
		}
	    }
#endif	RING
#ifdef	CARRIER&RING
	       else
#endif
#ifdef	CARRIER
		  /* carrier only line */
		  if( (scanc & t_bit) && (dzaddr->dzcarr & t_bit) )
		  {
			if( tp->t_state & WOPEN)
			{
				dzaddr->dzdtr =| t_bit;
				wakeup(&tp->t_rawq);
			}
			else if( ( ! (tp->t_state&TIMEOUT) ) && ( tp->t_outq.c_cc ) )
				dzaddr->dztcr =| t_bit;
		  }
#endif	CARRIER
	} else  tp=+ 8, p=+ 8;	/* in the case where no dialup/carrier lines on current dz */
  }
#endif	CARRIER|RING

 /*
  *	process timeouts for each line
  */

    {
	register i;

	for( i=0 ; i < NLINES ; i++ )
		if( (dzdelays[i]>0) && (--dzdelays[i]<=0) )
		{
			dz11[i].t_state =& ~ TIMEOUT;
			dzstart( &dz11[i] );
		}
    }

 /*
  *	scan each dz for input
  */

  dzrint(0);
  if( dzscanning > 0 ) timeout( &dzscan , 0 , TSCANRATE );
}



/*
 *	close a DZ11 line
 */
dzclose( dev )
{
	register struct tty *tp;
	register t_bit;
	register struct dz *dzp;

	tp = &dz11[dev.d_minor];
	wflushtty(tp);
	tp->t_state = SSTART;

	dzp = &dz[dev.d_minor>>3];
	t_bit = 1 << (dev.d_minor&07);
	if( ( dzp->openl =& ~t_bit ) == 0 )  dzp->dzaddr->dzcsr = 0; /* disable receive on final close */
#ifdef	RING
	if( dzp->ring & t_bit )
	{
		if( tp->t_flags & HUPCL ) dzp->dzaddr->dzdtr =& ~t_bit;	/* hang up the phone line */
	}
	else
	{
		dzp->dzaddr->dzdtr =& ~t_bit;	/* turn off dtr for non-dialup lines */
	}
#endif	RING
#ifndef	RING
	dzp->dzaddr->dzdtr =& ~t_bit;	/* turn off dtr for non-dialup lines */
#endif	RING

	dzscanning--;
}



/*
 *	read from a DZ11 line
 */
dzread( dev )
{
	ttread( &dz11[dev.d_minor] );
}



/*
 *	write on a DZ11 line
 */
dzwrite( dev )
{
	ttwrite( &dz11[dev.d_minor] );
}



/*
 *	stty/gtty for DZ11
 */
dzsgtty( dev , av )  int *av;
{
	register struct tty *tp;

	if( (av == 0) && (dzspeedmap[u.u_arg[0] & 017] < 0) )
	{
		u.u_error = ENXIO;		/* illegal speed */
		return;
	}
	tp = &dz11[dev.d_minor];
	if( ttystty( tp , av ) )  return;
	dzparam( tp , 1 );
}



/*
 *	set parameters from open or stty into DZ hardware registers
 */
dzparam( tp , dflag )
register struct tty *tp;
{
	register lpr,x;
	extern wakeup();

  lpr = dzspeedmap[tp->t_speeds & 017] << 8;

#ifdef	IBM2741
  if( lpr == (RCVR_ON|S134_5) )  lpr = IBM2741;
  else
  {
#endif	IBM2741
	if( lpr == (RCVR_ON|S110) )  lpr =| TWOSBIT;

	if( (x = tp->t_flags) & EVENP )  if( (x & ODDP) == 0 )  lpr =| (EVN_PAR|C7BIT);
					  else  lpr =| C8BIT;
	else
		if( x & ODDP )  lpr =| (ODD_PAR|C7BIT);
		else  lpr =| C8BIT;
#ifdef	IBM2741
  }
#endif	IBM2741

  if( dflag ) {
	/* delay only if it is permissible */
#ifndef	DELAY
	timeout( &wakeup , tp , FLUSHTIME );	/* wakeup in 5 tics */
	sleep( tp , TTOPRI );		/* delay while controller flushes */
#else
	delay( FLUSHTIME );	/* hang 5 */
#endif	DELAY
  }

  dz[tp->t_dev.d_minor>>3].dzaddr->dzlpr = lpr | (tp->t_dev.d_minor&07);
}



/*
 *	start (restart) transmission on a DZ11 line
 */
dzstart( tp )
register struct tty *tp;
{
	register t_bit;
	register struct dz *dzp;

	t_bit = 1 << ( tp->t_dev.d_minor & 07 );
	dzp = &dz[tp->t_dev.d_minor >> 3];

#ifdef	CARRIER|RING
	if( (!(dzp->carrier&t_bit)) || (dzp->dzaddr->dzcarr&t_bit) )
#endif	CARRIER|RING
		dzp->dzaddr->dztcr =| t_bit;
}



/*
 *	DZ11 transmitter interrupt.
 *
 *	Scan every line on each dz.
 *	Commencing with the device that caused
 *	dzxint to be called.
 */
dzxint(dev)
{
	register struct tty *tp;
	register c;
	register struct dzr_read *dzaddr;
	struct dz *dzp;
	struct tty *dzbase;
	int t_bit, lino, i, n;

  n = dev.d_minor;
  for( i=0 ; i < NDZ ; i++ )
  {
	dzaddr = (dzp = &dz[n]) -> dzaddr;
	dzbase = &dz11[ n * 8 ];
	while ( (c = dzaddr->dzcsr) < 0 )		/* xmit line ready */
	{
	  t_bit = 1 << ( lino = (c >> 8) & 07 );

	  tp = &dzbase[lino];

#ifdef	CARRIER|RING
	  if( ( (!(dzp->carrier & t_bit))||(dzaddr->dzcarr & t_bit) ) && ((c=getc(&tp->t_outq)) >= 0) )
#endif	CARRIER|RING
#ifndef	CARRIER|RING
	  if( (c=getc(&tp->t_outq)) >= 0 )
#endif	CARRIER|RING
		if( c <= 0177 || tp->t_flags == RAW )
			dzaddr->dztbuf = c;
		else
		{
			dzaddr->dztcr =& ~t_bit;
			tp->t_state =| TIMEOUT;
			dzdelays[tp-dz11] = ((c & 0177) + (TSCANRATE-1))/TSCANRATE + 1; /* set up timeout */
			continue;
		}
	  else
		dzaddr->dztcr =& ~t_bit;

#ifdef	TTY_HISPEED
	  if( tp->t_outq.c_cc <= ( ( tp->t_speeds & 017 ) > B1200 ? TTHSLOWAT : TTLOWAT ) &&  tp->t_state & ASLEEP )
	  {
#endif	TTY_HISPEED
#ifndef	TTY_HISPEED
	  if( tp->t_outq.c_cc <= TTLOWAT  &&  tp->t_state & ASLEEP )
	  {
#endif	TTY_HISPEED
		tp->t_state =& ~ASLEEP;
		wakeup( &tp->t_outq );
	  }
	}
	if( ++n >= NDZ ) n = 0;
  }
}



/*
 *	DZ11 receiver interrupt
 *
 *	Scan each dz commencing with the
 *	particular device that caused this call.
 *	Storing each charater as it comes.
 */
dzrint(dev)
{
	register struct tty *tp;
	register c;
	register struct dzr_read *dzaddr;
	struct dz *dzp;
	struct tty *dzbase;
	int i, n, lino, t_bit;

  n = dev.d_minor;
  for( i=0 ; i < NDZ ; i++ )
  {
	dzp = &dz[n];
	dzbase = &dz11[ n * 8 ];
	while ( (c = dzp->dzaddr->dzrbuf) < 0 ) 	/* char present in silo */
	{

	  tp = &dzbase[ lino = ((c>>8) & 07) ];
	  t_bit = 1 << lino;

	  if( c & RERROR )
	  {
		if( c & OVR_RUN )
		{
			dzp->overrors++;
#ifdef	MESSAGES
			printf( "over run on dz %d/%d\n", n, lino);
#endif	MESSAGES
		}
		if( c & FRAME )			/* break */
			if( tp->t_flags & RAW )
				c = 0;			/* null ( for getty ) */
			else
				continue; /* ignore framing errors if not raw */
		else
			if( c & PARITY )
			{
				dzp->pyerrors++;
#ifdef	MESSAGES
			printf( "parity on dz %d/%d\n", n, lino);
#endif	MESSAGES
				continue;		/* throw away bad chars */
			}
	  }

#ifdef	CARRIER|RING
	  if( (!(dzp->carrier & t_bit))||(dzp->dzaddr->dzcarr & t_bit) ) ttyinput( c , tp );
#endif	CARRIER|RING
#ifndef	CARRIER|RING
	  ttyinput( c , tp );
#endif	CARRIER|RING
	}
	if( ++n >= NDZ ) n = 0;
  }
}
#ifdef	POWER_FAIL

dzpowf()
{
	register line;
	register struct dz *dzp;
	register struct tty *tp;
	int dzn, t_bit;

	tp = &dz11[0];
	for( dzn = 0 ; dzn < NDZ ; dzn++ ) {
		dzp = &dz[dzn];
		if( dzp->openl ) {
			(dzp->dzaddr)->dzcsr =| (TIE|RIE|SAE|MSE); /* reciever interrupt every 16 chars */
			for( line = 0 ; line < 8 ; line++, tp++ )
				if( tp->t_state & ISOPEN ) {
					t_bit = 1 << line;
					dzparam( tp , 0 ); /* set speeds */
					(dzp->dzaddr)->dztcr =| t_bit; /* enable transmit */
#ifdef	RING
					if( !(dzp->ring & t_bit) )
#endif	RING
						dzp->dzaddr->dzdtr =| t_bit;		/* turn on DTR for non-dialup lines */
				}
		} else tp =+ 8;
	}
}
#endif	POWER_FAIL
