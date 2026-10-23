#
/*
 *	log  remote_tty
 *
 *  File exchange package for remote unices.
 *
 *  Also has transparent mode for talking to
 *  other operating systems as if local tty.
 *
 *  For the following reasons this program must be
 *  setuid to root for successful use:
 *	1. The tty to be used for communication with other machine
 *	   is to be owned by root and is to have a mode of 600.
 *	2. Needs to run at fairly high priority,
 *	   so nice will be used to obtain same.
 *	3. The connect system call only allowed to be
 *	   used by root, to prevent abuse.
 *
 * Multiple opens of the same tty are assumed
 *  to be taken care of by the appropriate tty
 *  drivers (eg dz & dj) supporting the notion
 *  of certain ttys being single open, and that
 *  ttys used by 'log' for communications are
 *  setup as single open.
 *
 *	Ian Johnstone - AGSM - 1976
 *	Peter Ivanov  - UNSW - 1976
 *
 *	Modified for flow control
 *	 Piers Lauder - Sydney University - Aug '77
 *
 *	Modified for binary file protocol
 *	 Piers Lauder - Sydney University - Sep '77
 *
 *	Modified for settable quit code & shell escape & default file args
 *	 Piers Lauder - Sydney University - Feb '78
 *	 Ian Johnstone - AGSM - Feb '78
 *
 *	Updated to use 'connect' system call.
 *	 Ian Johnstone - AGSM - MAY '78
 *
 *	Updated to use 'access' to validate file access,
 *	 since 'log' will always be setuid to root from
 *	 this time onwards.  Also only allow users with
 *	 a uid < 100 to use log, to stop abuse.  The
 *	 convention is that users with a uid of < 100
 *	 have accounts on multiple machines and thus
 *	 have a reason for using log.
 *	 Ian Johnstone - AGSM - AUG '78
 *	 Piers Lauder - Sydney University - Aug '78
 *
 */

#include	<local-system>
#include	<stat16.h>

#ifdef	AUSAM
#include	<passwd.h>
#include	<uid.h>
	/*
	 *	Piers Lauder	Feb '78
	 *
	 *	Use 16-bit user id.
	 */
#endif	AUSAM


#define	HIPRIORITY	-40
#define	USERPRI		0


/*
 * Regulate file transfer with a file protocol as follows:-
 *
 *  Blocks consist of a header followed by data followed by a trailer.
 *  The header contains a sequence number (0/1) and the size
 *   ( transmitted twice, 2nd as complement of first ).
 *  The trailer contains a longitudinal parity check of the data,
 *   followed by a sum check of the data.
 *
 *  Block format is
 *	STX SEQ SIZ ~SIZ ...DATA... LPC SUM
 *
 *  Blocks are acknowledged by a 2 byte reply consisting of
 *   ACK or NAK followed by the relevant sequence number.
 *
 * Note: that the DATAZ is really dependant on the accuracy
 *	 of the crc check.
 */

#define	REPLYZ		2
#define	RTYPE		0
#define	RSEQ		1

struct
{
	char r_typ;
	char r_seq;
}
reply;

#define	MREPLY		&reply,REPLYZ

#define	HEADER		4
#define	BSEQ		1
#define	TRAILER		2
#define OVERHEAD	(HEADER+TRAILER)
#define	DATAZ		64
#define	DATAZL		64L
#define	BUFZ		(DATAZ+OVERHEAD)

/*
 *  Timeouts - these must be tailored to the line speed,
 *   the basic formulae used being:-
 *	receive timeout = receive time + turnaround time
 *	transmit   "    = 2 * receive timeout
 *	( all times in seconds )
 */
#define	STARTTIMEOUT	10
#define	FLUSHTIMEOUT	1
#define	TURNAROUND	3
#define	XRETRYS		4
#define	RRETRYS		5
#define	GARBAGE		2	/* max garbage chars tolerated before reset */
#define	EXTA		200	/* per DZ11 */
#define	EXTB		1920	/* per DZ11 */
#define	DEFAULTRATE	120	/* 1200 baud default */

int	byterate[16]	/* per DH11 */
{
	0,5,8,10,15,15,20,30,60,120,
	180,240,480,960,EXTA,EXTB
};

int	xtimeout, rtimeout;
long	xrate;			/* currently effective data transfer rate */
long	xtotal;			/* total bytes received/transmitted */
long	xtime;			/* total transmission time */

#define	SOH		001	/* remote accept */
#define	STX		002	/* start of block */
#define	ETX		003	/* remote finish */
#define	EOT		004	/* remote file access error */
#define	ENQ		005	/* remote timeout */
#define	ACK		006	/* xfer ok */
#define	DLE		020	/* no permission to use log at remote !! */
#define	NAK		025	/* xfer failed */

#define	MSOH		"\01\01",2
#define	METX		"\03\03\03\03",4
#define	MEOT		"\04\04",2
#define	MTIMEOUT	"\05\05",2
#define	MDLE		"\020\020",2

#define	PROMPT		"* ",2
#define	SIGTIMEOUT	15
#define	SIGINTERRUPT	3
#define	SIGQUIT		2

int	sav_modes[16];
#define	RAW	040
int	ttymode[3];
int	narg;
char	*cmd[10];
int	xfd	2;		/* fd for remote port */
char	otherm[]	"/dev/xxxxxxxxxxxxxx";
char	c;
unsigned uid;
long	time();
long	filesize;

/* defines for use with access calls */
#define	R	04	/* read permission */
#define W	02	/* write permission */
#define	X	01	/* execute / directory search permission */

#define ENOENT	2	/* No such file/directory */

#define	ESCAPE	021	/* default escape char is ctrl/q */
#define	ESCAPED	"^q"	/* default escape description */
char	quit;		/* escape code */
char	*quitdescrpt;	/* escape code description */
#ifndef	BASSER
struct
{
	char *name;
	char *descpt;	/* description of control code that follows */
	char  escape;	/* default escape code for this line */
}
quits[]
{
	"agsm",	"^a",	001,
	"elec",	"^e",	005,
	"csu",	"^c",	003,
};
#endif	BASSER

struct	err
{
	int	e_count;
	char	*e_mess;
}
errs[]
{
#define	E_RETRYS	0
	{ 0,	"retrys" },
#define	E_SEQUENCE	1
	{ 0,	"seq" },
#define	E_SIZE		2
	{ 0,	"size" },
#define	E_OUTSIZE	3
	{ 0,	"outsize" },
#define	E_TIMEOUTS	4
	{ 0,	"timeouts" },
#define	E_CRC		5
	{ 0,	"crc" },
#define	E_WRITE		6
	{ 0,	"local write" },
#define	E_NAKS		7
	{ 0,	"naks" },
#define E_SYNC		8
	{ 0,	"sync" },
#define	E_READ		9
	{ 0,	"local read" }
};
#define	NERRS		((sizeof errs)/(sizeof errs[0]))

char	buf[BUFZ];

struct
{
	int hiword;
	unsigned loword;
};



/****************************************************************/

main (argc,argv)
char	**argv;
int	argc;
{
	register  x;

	nice( HIPRIORITY );
#ifdef	AUSAM
	uid = getreal();
#else
	uid = ((x = getuid() & 0377)<< 8 ) | x;
#endif	AUSAM
#ifndef	BASSER
	if ( uid >= COMMONUSERS )
	{
		if( argv[0][0] == 'l' )
			printf( "You are not validated for inter-system communication\n" );
		else
			write(2, MDLE );
		return( 1 );
	}
#endif	BASSER

	sentinel();	/* catch all real-time limit expiries */

	switch ( argv[0][0])
	{
	   case 'l':
		if( okuse() == 0 )
		{
			log (argc,argv);
			errors();	/* tell about the errors encountered */
		}
		else
		{
			printf("log cannot be used in this way\n");
			printf("standard input/output cannot be redirected\n");
			printf("NOR can it be run asynchronously\n");
			printf("speaking personnally i am sorry about this\n");
		}
		break;
	   case 's':
		send (argc,argv);
		break;
	   case 'r':
		receive (argc,argv);
		break;
	   default:
		write( 2 , MEOT );
		return(1);
	}

}

/*****************************************************************/

log(argc,argv)
char **argv;
int argc;
{
	register char	*arg;
	register char	*otp;
	register int	i;
	extern char	*fquit();
	extern		finish();

	if (argc != 2)
	{
argerr:
		printf("useage:\n\tlog remote-special-file\n");
		exit(1);
	}

	arg = argv[1];
	otp = &otherm[5];
	for(i=0;i<15;i++) if( (*otp++ = *arg++)==0 ) goto start;
	goto argerr;

 start:
	quit = ESCAPE;
	quitdescrpt = ESCAPED;
#ifndef	BASSER
	for( i=0 ; i < sizeof quits / sizeof quits[0] ; i++ )
	{
		arg = argv[1];
		otp = quits[i].name;
		while( *otp ) if( *otp++ != *arg++ ) break;
		if( *otp != '\0' ) continue;
		quit = quits[i].escape;
		quitdescrpt = quits[i].descpt;
		break;
	}
#endif	BASSER
	signal( SIGINTERRUPT, finish );
	signal( SIGQUIT, finish );

	if( (xfd = open(otherm,2)) < 0 )
	{
		perror(argv[1]);
		exit (2);
	}
	init_remote( xfd );
	printf("* tty\n");
	tty();

	for (;;)
	{
		errors();

		switch(getline())
		{
		/* put a file to remote */
		case 'p':
			if (narg == 3)  { put();  continue; }
			break;
		/* get a file from remote */
		case 'g':
			if (narg == 3)  { get();  continue; }
			break;
		/* talk to remote as a terminal */
		case 't':
			if (narg == 1)  { tty();  continue; }
			break;
		/* stop this sillyness */
		case 's':
			if (narg == 1)  { finish();  continue; }
			break;
		/* change quit code */
		case 'q':
			if (narg == 2)  {  quitdescrpt = fquit();  continue;  }
			break;
		/* local escape to shell */
		case '!':
			sh();
			continue;
		/* unknown command */
		default:
			help();
			continue;
		}
	}
}

/**************/
getline()
{
	static char	line[200];
	register char	*q;
	register	*p;
	register int	i;

	printf(PROMPT);
	if ( (i = read(2,line,sizeof line)) == 1) return(-1);
	if ( i <= 0 )
		finish();
	narg = 1;
	p = &cmd[0];
	q = &line[0];
	*p++ = q;

	for (i = 1; ; i++)
	{
		if (line[i] == ' ')
		{
			line[i] = 0;
			if ( line[i-1] != 0 )
			{
				narg++;
				*p++ = q + 1 + i;
			}
		}
		else if (line[i] == '\n')
		{
			line[i] = 0;
			*p = 0;
			break;
		}
	}

	if ((narg == 2) && ((*q == 'p') || (*q == 'g')))
	{
		*p++ = cmd[1];
		*p = 0;
		narg++;
	}

	return(*q);
}

/*******************/


fini_remote( fd )
register int fd;
{
	gtty ( fd ,ttymode );
	ttymode[2] = sav_modes[fd];
	stty ( fd , ttymode );
}

init_remote( fd )
register int fd;
{
	register speed;

	gtty ( fd , ttymode );
	sav_modes[fd] = ttymode[2];
	ttymode[2] = RAW;
	stty ( fd , ttymode );
	if ( (speed = byterate[(ttymode[0] & 017)]) == 0 )
		speed = DEFAULTRATE;
	xtimeout = 2 * (rtimeout = BUFZ / speed + TURNAROUND);
	xrate = (speed * DATAZL) / BUFZ;
}


set_rate( t )
long t;
{
	xtotal =+ filesize;
	xtime =+ t;
	xrate = xtotal/xtime;
}

/*********************/
finish()
{
#ifdef	BASSER
	printf("have you hung up the 'phone ?\n");
#endif	BASSER
	fini_remote( xfd );
	exit(0);
}

/*************************************************************/

char *fquit()
{
	static char	qs[3];
	register char	*qp = qs;

	switch( quit = *cmd[1] & 0177 )
	{
    case 037:
		return( "US" );
    case 0177:
		return( "DEL" );
    default:
		if ( quit < 040 )
		{
			*qp++ = '^';
			*qp++ = quit+0140;
		}
		else
		{
			*qp++ = quit;
		}
	}
	*qp = 0;

	return( qs );
}

/*************************************************************/

sh()
{
	char *args[4];
	register char **ap = args;
	register char *bp;
	register i = 0;
	static char shf[] "/bin/sh";

	cmd[0]++;
	while ( (bp = cmd[i++]) && cmd[i] )
	{
		while ( *bp++ );
		--bp;
		do *bp++ = ' '; while ( !*bp && bp<cmd[i]);
	}

	*ap++ = shf+5;
	*ap++ = "-c";
	*ap++ = cmd[0];
	*ap = 0;

	if ( !(i = fork()) )
	{
		nice( USERPRI );
		setuid( uid );
		execv( shf , args );
		perror( shf );
		exit( -1 );
	}
	if ( i > 0 )
		waitx( ap );
	else
		perror( "try again" );
}

/*************************************************************/

tty()
{

	/*
	 * logically connect local terminal to remote until quit char received
	 */
	printf("  %s -> %s   ('%s'=quit)\n",SYSTEMID,&otherm[5],quitdescrpt);
	if( connect( 2 , xfd , quit ) < 0 )
	{
		perror("connect failed");
		finish();
	}
}

/*************************************************************/

get()
{
	register	ofd;
	register	n;
	long		t;

	if( (ofd=accreat(cmd[2])) < 0)
	{
		perror(cmd[2]);
		return;
	}

	transfer("send",cmd[1]);

	if ( sync( "open" ) )
	{
		printf( " -- receive rate approx. %D bytes/sec.\n", xrate );
		t = time();
		if ( rproto( ofd , xfd ) == 0 )
		{
			if ( !(t = time() - t) )  t++;
			set_rate( t );
			printf( "%s: %D bytes received in %D secs (%D bytes/sec)\n" , cmd[2] , filesize, t, filesize/t );
		}
		else
		{
			printf( " -- transfer aborted after %D bytes\n", filesize );
		}
	}

	close(ofd);
}

/*************************************************************/

put()
{
	register	ifd;
	register	n;
	long		t;
	struct statbuf	statb;

	if( access(cmd[1],R) || ((ifd = open(cmd[1],0)) < 0) )
	{
		perror( cmd[1] );
		return;
	}

	fstat( ifd, &statb );
	filesize.loword = statb.sb_size1;
	filesize.hiword = statb.sb_size0 & 0377;

	transfer("receive",cmd[2]);

	if ( sync( "create" ) )
	{
		printf( " -- estimated transmit time: %D secs\n", filesize/xrate );
		t = time();

		if ( (n = xproto( ifd , xfd )) == 0 )
		{
			if ( !(t = time() - t) )  t++;
			set_rate( t );
			printf( "%s: %D bytes transmitted in %D secs (%D bytes/sec)\n", cmd[2], filesize, t, filesize/t );
		}
		else
		{
			printf( " --%stransfer aborted after %D bytes\n", (n==2) ? " remote terminate -- " : " ", filesize );
		}
	}

	close (ifd);
}

/*******************************/

transfer(s1,s2)
char	*s1,*s2;
{
	register char	*p, *q;

	q = buf;

	*q++ = '@';

	for (p = s1; *q++ = *p++; );
	q[-1] = ' ';
	for (p = s2; *q++ = *p++; );
	q[-1] = '\n';

	write (xfd, buf, q-buf);
}


/*******************************/

send(argc,argv)
char **argv;
{
	register  fd;

	init_remote( 2 );

	if( argc != 2 || access(argv[1],R) || ((fd=open(argv[1],0))<0) )
	{
		write( 2 , MEOT );
	}
	else
	{
		write( 2 , MSOH );
		xproto( fd , 2 );
	}

	lflush( 2 );
	fini_remote( 2 );
}

/*******************************/

receive(argc,argv)
char **argv;
{
	register	fd;

	init_remote( 2 );

	if( argc != 2 || (fd=accreat(argv[1])) < 0)
	{
		write( 2 , MEOT );
	}
	else
	{
		write( 2 , MSOH );
		rproto( fd , 2 );
	}

	lflush( 2 );
	fini_remote( 2 );
}

/********************************************************************/

int	timedout;

sentinel()
{
	signal( SIGTIMEOUT , sentinel );
	timedout++;
}


tread( f , b , s , timeout )
{
	clktim( timeout );
	timedout = 0;
	read( f , b , s );
	clktim( 0 );
	return( timedout );
}


lflush( fd )
{
	while ( !tread( fd , &c , 1 , FLUSHTIMEOUT ) );
	return( 1 );
}

/********************************************************************/

sync( remote_err )
char	*remote_err;
{
	do
	{
		if ( tread( xfd , &c , 1 , STARTTIMEOUT ) )
		{
			printf( "no response from remote!\n" );
			return( 0 );
		}
		if( c == EOT )
		{
			printf( "can't %s remote file!\n" , remote_err );
			return( 0 );
		}
		if( c == DLE )
		{
			printf( "no permission to use log at remote\n" );
			return( 0 );
		}
	} while( c != SOH);

	lflush( xfd );
	return( 1 );
}

/******************************************************************/

xproto( local , remote )
{
	register  n, retrys, sync;

	sleep( FLUSHTIMEOUT+1 );
	buf[BSEQ] = 1;
	filesize = 0;

	while ((n = read( local , buf+HEADER , DATAZ )) >= 0 )
	{
		crc( n );
		retrys = 0;

		do
		{
			write( remote , buf , n+OVERHEAD );

			sync = 0;
			while ( tread( remote , &reply , 1 , xtimeout ) == 0 )
			{
				switch ( reply.r_typ )
				{
			     default:
					errs[E_SYNC].e_count++;
					if ( ++sync > GARBAGE )
						break;
					else
						continue;

			     case ACK:
			     case NAK:
			     case ENQ:
			     case ETX:
					tread( remote , &reply.r_seq , 1 , xtimeout );
				}
				break;
			}

			if ( reply.r_typ == ETX && reply.r_seq == ETX )
				return( n ? 2 : n );	/* remote finish */

		} while (

			  (  (sync > GARBAGE && lflush( remote ))

			  || (timedout && ++errs[E_TIMEOUTS].e_count)

			  || (reply.r_typ != ACK && ++errs[E_NAKS].e_count)

			  || (reply.r_seq != buf[BSEQ] && ++errs[E_SEQUENCE].e_count)

			  )

			 && retrys++ < XRETRYS

			);

		errs[E_RETRYS].e_count =+ retrys;
		if ( retrys > XRETRYS )
		{
			errs[E_RETRYS].e_count--;
			return( 1 );
		}

		if ( n == 0 )	/* eof */
			return( n );

		filesize =+ n;
	}

	errs[E_READ].e_count++;

	return( 3 );
}

/*******************************************************************/

/* protocol states */
#define	NEWBLOCK	0
#define	SEQUENCE	1
#define	SIZE1		2
#define	SIZE2		3
#define	STATES		4

/* case labels */
#define	newblock	0
#define	sequence	1
#define	size1		2
#define	size2		3

/* decision table */ /*
char	dtbl[STATES]
{
	newblock,
	sequence,
	size1,
	size2
}; */

rproto( local , remote )
{
	register  size, state, x;
	int  wretrys, lastsize, retrys, garbage;
	char  lastseq;

	state = NEWBLOCK;
	retrys = 0;
	reply.r_seq = 0;
	filesize = 0;
	lastseq = -1;
	garbage = 0;
	lastsize = 0;

	for (;;)
	{

		while ( tread( remote , &c , 1 , rtimeout ) )
			if ( retrys++ < RRETRYS )
			{
				errs[E_TIMEOUTS].e_count++;
				write( remote , MTIMEOUT );
			}
			else
			{
	fail:
				write( remote, METX );
				return( 1 );
			}

		x = c & 0377;

		switch ( /*dtbl[*/ state /*]*/ )
		{

	    case newblock:
			if ( x == STX )
			{
				state = SEQUENCE;
				retrys = 0;
				garbage = 0;
			}
			else
				if ( garbage++ >= GARBAGE )
				{
					lflush( remote );
					garbage = 0;
				}
			break;

	    case sequence:
			if ( x & ~1 )
			{
				errs[E_SEQUENCE].e_count++;
fnak:
				state = NEWBLOCK;
				lflush( remote );
nak:
				reply.r_typ = NAK;
rply:
				write( remote, MREPLY );
				break;
			}
			reply.r_seq = x;
			state = SIZE1;
			break;

	    case size1:
			if ( (size = x) > DATAZ )
			{
				errs[E_OUTSIZE].e_count++;
				goto fnak;
			}
			state = SIZE2;
			break;

	    case size2:
			state = NEWBLOCK;
			if ( size != (~x & 0377) )
			{
				errs[E_SIZE].e_count++;
				goto fnak;
			}
			if ( size == 0 )	/* eof */
			{
				write( remote, METX );
				return( 0 );
			}

			if ( (tread( remote , buf+HEADER , size+TRAILER , rtimeout ) && ++errs[E_TIMEOUTS].e_count)
				|| (crc( size ) && ++errs[E_CRC].e_count)
			   )
				goto nak;

			if ( reply.r_seq == lastseq && lastsize > 0 )
			{
				seek( local , -lastsize , 1 );
				filesize =- lastsize;
			}

			wretrys = 0;
			while ( (lastsize = write( local , buf+HEADER , size )) != size )
			{
				errs[E_WRITE].e_count++;
				if ( wretrys++ >= XRETRYS )
					goto fail;
				if ( lastsize > 0 )
					seek( local, -lastsize, 1 );
			}
			filesize =+ size;

			reply.r_typ = ACK;
			lastseq = reply.r_seq;
			goto rply;
		}
	}
}

/********************************************************************/

/*
 *	not a true crc, but nearly so for small 's' ( < 100 say )
 */

crc( s )
{
	register char *p;
	register char lpc, sum;
	char  *end;
	int  error;

	p = buf;  lpc = 0;  sum = 0;  error = 0;

	*p++ = STX;
	*p++ =^ 1;		/* flip sequence number */
	*p++ = s;  *p++ = ~s;

	end = buf+HEADER+s;
	for ( ; p < end ; p++ )
	{
		lpc =^ *p;
		sum =+ *p;
	}

	if ( lpc != *p )  error++;
	*p++ = lpc;
	if ( sum != *p )  error++;
	*p++ = sum;

	return( error );
}

/**********************************************************************/

errors()
{
	register struct err *ep;
	register nerrs;

	nerrs = 0;

	for ( ep = errs ; ep < &errs[NERRS] ; ep++ )
	if ( ep->e_count )
	{
		if ( nerrs++ == 0 )
			printf( "  file transfer protocol errors:-\n" );
		printf( "  %s: %d" , ep->e_mess , ep->e_count );
		ep->e_count = 0;
	}

	if ( nerrs )  printf( "\n" );
	return( nerrs );
}

/***********************************************************************/

help()
{
	printf("\nuse:\n");
	printf("\tt[ty]\n");
	printf("\tp[ut] local-file [remote-file]\n");
	printf("\tg[et] remote-file [local-file]\n");
	printf( "\tq[uit] new-code\n" );
	printf( "\t!{shell command}\n" );
	printf("\ts[top]\n\n");
}

/*******************************************************************/

#ifdef	SPRINTF
#define		NUMBERS
#define		LONG
#include	<sprintf.h>
#endif	SPRINTF

/*
**	check for appropriate permissions on a file
**	passed as 'arg' and creat it if allowable.
*/

accreat( arg )
char *arg;
{
	register char *p,*q,*r;
	int n;
	extern int errno;

	if( (n = access(arg,W)) && (errno == ENOENT) )
	{
		/*
		 * File does not exist check dir for permissions.
		 * Most of the work below is concerned with
		 * isolating the directory name so that an
		 * 'access' can be done.
		 */
		q = (p = arg) + 1;
		r = "";
		while( *q )
		{
			r = q;
			if( *q++ == '/')
			{
				while( *q == '/' )
					q++;
				if (*q)
					p = q;
				else
					*r = 0;
			}
		}
		q = arg;
		if( p == q)
		{
			if (*p == '/')
				q = "/";
			else
				q = "";
		}
		n = access(q, W|X);
		if( *r == 0 ) *r = '/';
	}
	if( n == 0 )
	{
		/* creat the file since access is granted */
		n = creat( arg , 0600 );
		chown( arg , uid );
	}
	if( n < 0 )
		perror(arg);
	return n;
}

/*
**	validate that log is not being used with any
**	standard input or output redirection - this
**	includes asynchronous use.
*/

okuse()
{
	struct
	{
		int majmin;	/* major minor device number for i-node */
		int inum;	/* inode number on device */
		int flags;	/* flags from inode */
#define	IFMT	060000
#define	ICHR	020000
		char dumy[30];	/* the rest ... */
	}
	sbuf[3];
	register i;
	register err = 0;

	for( i = 0; i < 3; i++ )
	{
		if( fstat( i, &sbuf[i] ) == -1 )
		{
			err++;	/* stat failed ??? */
			break;
		}
		if( sbuf[i].flags&IFMT != ICHR )
		{
			err++;	/* not a tty */
			break;
		}
		if( i &&  (sbuf[i].majmin != sbuf[i-1].majmin
			   || sbuf[i].inum != sbuf[i-1].inum) )
		{
			err++;	/* some sort of redirection */
			break;
		}
	}
	return err;
}
