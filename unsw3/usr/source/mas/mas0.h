#					/* mod007 */


#define	STAR	'*'
#define	TAB	'\t'
#define	SP	' '
#define COLON	':'
#define NL	'\n'
#define CR	'\r'			/* mod004 */
#define BACKSP	'\b'			/* mod004 */
#define EOF	-1			/* mod005 */
#define MINUS	'-'
#define PLUS	'+'
#define SLASH	'/'
#define LPAR	'('
#define RPAR	')'
#define QUOTE	'\''
#define COMMA	','
#define PERCEN	'%'
#define BACKSL	'\\'


#define e_CON	0
#define e_SYM	1
#define e_TL	2
#define e_PC	3

#define e_TERM	0
#define e_PLUS	1
#define e_MINUS	2
#define e_SLASH	3
#define	e_STAR	4

#define UND	0
#define EXP	1
#define ABS	2
#define EST	3


#define o_INH	0
#define o_DUAL	1
#define o_STA	2
#define o_AEI	3
#define o_PUSH	4
#define o_IDEI	5
#define o_DEI	6
#define o_JMPS	7
#define o_BNCH	8
#define o_JBRS	10
#define o_JEQS	11

#define o_SEG	100
#define o_END	101
#define o_EQU	102
#define o_FCB	103
#define o_FCC	104
#define o_FDB	105
#define o_MON	106
#define o_NAM	107
#define o_OPT	108
#define o_ORG	109
#define o_PAG	110
#define o_RMB	111
#define o_ZMB	112
#define o_SPC	113
#define	o_ICL	114



struct segment
{
	char segchar;		/* segmentation char. */
	char segtype;		/* 2 if '-' sign used; else 1 */
	char segcount;
};



#define	NSYMBOL	600			/* mod008 - number of symbols + 1 */
#define LMAX	7			/* mod006 - maximum label field length */

struct	symbol
{
	char s_name[LMAX+1];		/* mod006 */
	int s_pc;
	int s_chain;
	char s_def;
	char s_seg;
};


#define	NFB	400

struct	fb
{
	int fb_pc;
	char fb_def;
	char fb_lab;
};


struct	aform
{
	int a_pc;
	int a_def;
};


#define a_EQU	100
#define a_LONG	0
#define a_JBRS	1
#define a_DIR	2
#define a_JEQS	3


struct iform
{
	int i_pc;
	int i_def;
};

#define i_LONG	1
#define i_DIR	2
#define i_REL	3


struct	expr
{
	char e_rator;
	char e_rand;
	int e_val;
};


struct	evalx
{
	int r_val;
	char r_type;
};


/* offsets from listbuf */

#define l_UND	0
#define l_ADR	2
#define l_C1	9
#define l_C2	12
#define l_LAB	19
#define l_CL	20
#define l_MNE	27
#define l_ACC	31
#define l_OPR	34
#define l_COM	52
#define l_EOL	131			/* mod001 */


#define DIGIT(ch)	ch >= '0' && ch <= '9'		/* mod012 */
