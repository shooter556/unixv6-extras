struct	sgttyb
{
	char	ispeed,
		ospeed,
		erase,
		kill;
	unsigned mode;
};

/* modes */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	NLDELAY	001400
#define	TBDELAY	002000
#define	INVCASE	004000
#define	CRDELAY	030000
#define	VTDELAY	040000
#define FLOWCNTRL 0100000	/* flow control required */
