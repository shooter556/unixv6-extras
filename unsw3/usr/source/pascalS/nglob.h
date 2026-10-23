#define PASREAL double
#define DOUBLE 		/* define to get double precision */
#define TRUE		1
#define FALSE		0
#define EOF		'\0'	/* getchar's end-of-file value */
#define EOR		'\022'	/* end-of-record in ascii */
#define NL		'\n'
#define TAB		'\t'
#define ALNG		10	/* significant chars in ids. */
#define LLNG		120	/* max input line length */
#define EMAX		32	/* max real exponent */
#define EMIN		-32	/* min real exponent */
#define KMAX		11	/* max significant digits */
#define PREMAX		15	/* significant digits for write */
#define TMAX		100	/* symbol table size */
#define BMAX		20	/* block table size */
#define AMAX		30	/* array table size */
#define C2MAX		20	/* real const table size */
#define CSMAX		30	/* case table size */
#define CMAX		850	/* code table size */
#define LMAX		7	/* max nesting level */
#define SMAX		600	/* string table size */
#define ERMAX		58	/* max error number */
#define OMAX		63	/* max instruction code */
#define XMAX		2048	/* max index */
#define NMAX		32767	/* max integer */
#define LONGMAX		2147483647	/* max long */
#define LINELENG	136	/* max output line length */
#define STACKSZE	500	/* execution stack size */

/*
 *	possible values for 'obj' field in symbol table.
 */
#define KONSTANT	0
#define VARIABLE	1
#define TYPE		2
#define PROCEDRE	3
#define FUNCTION	4


/*
 *	possible values of 'typ' field of symbol table.
 */
#define NOTYP		1
#define INTS		2
#define REALS		4
#define BOOLS		8
#define CHARS		16
#define ARRAYS		32
#define RECORDS		64

struct{
	int hiword,loword;
};
/*
 *	structure to access fields of 'getchar's' buffer.
 */
struct buf {
	int	fildes;		/* file descriptor */
	int	nleft;		/* chars left */
	char	*nextp;		/* pointer to next char */
	char	buffer[512];	/* the buffer */
};



/*
 *	instruction format.
 */
struct order {
	char	o_f;	/* opcode */
	char	o_x;	/* level. used in load instructions */
	int	o_y;	/* jump address, var address at level o_x,*/
} code[CMAX+1];		/* or type for read,write etc. */

/*
 *	structure of the symbol table .
 */
struct tab {
	char	t_name[ALNG+1];	/* name of symbol */
	char	t_link;		/* link back used in lookup */
	char	t_obj;		/* var, proc, func, const etc */
	char	t_typ;		/* char, int, real, bool, etc. */
	char	t_ref;		/* further type referenced */
	char	t_nrm;		/* value or var param. */
	char	t_lev;		/* declaration level */
	int	t_adr;		/* address at that level, or value */
} tab[TMAX+1];

/*
 *	array table structure.
 */
struct atab {
	char	a_inxtyp;	/* index type */
	char	a_eltyp;	/* element type */
	char	a_elref;	/* further type referenced */
	int	a_low;		/* lower bound */
	int	a_high;		/* upper bound */
	int	a_elsize;	/* store for each element */
	int	a_size;		/* store for whole array */
} atab[AMAX+1];

/*
 *	block table structure .
 */
struct btab {
	char	b_last;		/* link to last symbol at level */
	char	b_lastpar;	/* link to last param. of proc. */
	int	b_psize;	/* space needed for parameters */
	int	b_vsize;	/* space needed for local vars. */
} btab[BMAX+1];


/*
 *	structure of case table.
 */
struct casetab {
	int	c_val;	/* value of case label */
	int	c_lc;	/* value of location counter */
} casetab[CSMAX+1];

char	compiling;	/* 1=compile mode; 0=execute mode */
char	sy;		/* current symbol */
char	id[ALNG+1];	/* current identifier */
long	inum;		/* current integer */
PASREAL	rnum;		/* current real */
int	strgpos;	/* position in the string table (insymbol) */
char	sleng;		/* string length */
char	stab[SMAX+1];	/* string table */
char	ch;		/* current character */
char	line[LLNG];	/* input line */
char	cc;		/* position in line */
int	lc;		/* location counter */
char	ll;		/* length of line */
int	lineno;		/* current line number */
char	errpos;		/* position in line of last error */
char	progname[ALNG+1];	/* program name in header */
char	iflag,oflag;	/* whether 'input' & 'output' in header */
char	t,a,b,c2;	/* table indices */
int	sx;		/* string table index */
char	display[LMAX+1];	/* display table */
int	exittab[CSMAX+1];	/* jump table for exitting cases */
PASREAL	rconst[C2MAX+1];/* real constant table */
struct{
	long r_i;
};
char	errcnt;		/* compile error count */
char	level;		/* lexical level */

char	fld[CHARS+1],string[LINELENG];
long	ocnt, maxocnt;		/* runtime instruction count nad max */
