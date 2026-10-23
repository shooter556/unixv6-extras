#define ALPHA		(ch >= 'a' && ch <= 'z')
#define NUMERIC		(ch >= '0' && ch <= '9')
#define SETSIZE		4	/* used in error recovery */
#define NULLSET		{0,0,0,0}
#define NKW		(sizeof keyword)/(sizeof keyword[0])
#define CYCLE		1000	/* instruction exec. incr. */


/*
 *	reports from scanner.
 *	stored in 'sy'.
 */
#define INTCON		0
#define REALCON		1
#define CHARCON		2
#define STRING		3
#define NOTSY		4
#define PLUS		5
#define MINUS		6
#define TIMES		7
#define IDIV		8
#define RDIV		9
#define IMOD		10
#define ANDSY		11
#define ORSY		12
#define EQL		13
#define NEQ		14
#define GTR		15
#define GEQ		16
#define LSS		17
#define LEQ		18
#define LPARENT		19
#define RPARENT		20
#define LBRACK		21
#define RBRACK		22
#define COMMA		23
#define SEMICOLN	24
#define PERIOD		25
#define COLON		26
#define BECOMES		27
#define CONSTSY		28
#define TYPESY		29
#define VARSY		30
#define FUNCSY		31
#define PROCSY		32
#define ARRAYSY		33
#define RECORDSY	34
#define PROGSY		35
#define IDENT		36
#define BEGINSY		37
#define IFSY		38
#define CASESY		39
#define REPEATSY	40
#define WHILESY		41
#define FORSY		42
#define ENDSY		43
#define ELSESY		44
#define UNTILSY		45
#define OFSY		46
#define DOSY		47
#define TOSY		48
#define DOWNTOSY	49
#define THENSY		50


/*
 *	structure used to process expressions.
 */
struct item {
	char	i_typ;	/* type */
	char	i_ref;	/* table reference */
};
/*
 *	structure for processing constants
 */
struct conrec {
	char	c_typ;	/* type */
	long	c_i;	/* used if integer */
	PASREAL	c_r;	/* used if real */
};
int	errs[SETSIZE];	/* used to provide error listing */
int	plist;		/* listing to be produced */
