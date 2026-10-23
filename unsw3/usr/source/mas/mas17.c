#

#include	"mas0.h"

char NS[]	"";

char *mnemtab[512]
{
NS,	NS,	NS,	NS,	NS,	"jle",	NS,	NS,
NS,	NS,	NS,	"com",	NS,	NS,	"opt",	NS,
NS,	NS,	NS,	NS,	"bit",	NS,	"pul",	NS,
NS,	NS,	"sev",	"jeq",	NS,	"tpa",	NS,	NS,
NS,	NS,	"ble",	"jne",	"jlt",	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	"asr",	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
"beq",	NS,	"zmb",	NS,	NS,	NS,	NS,	NS,
"bne",	"blt",	NS,	NS,	"jls",	NS,	"lsr",	NS,
NS,	NS,	NS,	NS,	NS,	"neg",	"sbc",	NS,
NS,	NS,	NS,	NS,	"txs",	NS,	NS,	"rmb",
NS,	NS,	NS,	NS,	NS,	NS,	"cli",	NS,
NS,	"bls",	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
"wai",	NS,	NS,	NS,	"ror",	NS,	NS,	NS,
"psh",	NS,	NS,	"sec",	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"ora",	"sba",	NS,
NS,	NS,	NS,	NS,	NS,	"end",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	"rts",
NS,	"equ",	NS,	NS,	"ldx",	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	"jmp",	NS,	NS,	NS,	NS,
NS,	"cpx",	NS,	"stx",	NS,	"clv",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
"cba",	"swi",	NS,	NS,	"fcc",	"jsr",	NS,	NS,
"dex",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	"jvs",	"jvc",	NS,	NS,	NS,	NS,
NS,	NS,	NS,	"eor",	NS,	NS,	NS,	NS,
NS,	NS,	"bsr",	NS,	"fcb",	NS,	"asl",	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	"bvs",
"bvc",	NS,	NS,	"fdb",	"bra",	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"inx",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"add",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	"tab",	NS,	NS,	"clc",	NS,
"spc",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"adc",	NS,	NS,
NS,	NS,	NS,	NS,	"rol",	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"clr",	NS,	NS,
NS,	NS,	NS,	NS,	"lds",	NS,	"mon",	NS,
"jhi",	NS,	"tba",	NS,	NS,	NS,	NS,	"aba",
"nam",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	"tap",	"sts",	NS,	NS,	NS,	NS,
"jpl",	NS,	NS,	NS,	NS,	"bhi",	NS,	NS,
"tsx",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
"des",	"dec",	NS,	NS,	NS,	"daa",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"bpl",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"lda",	NS,	NS,
NS,	"pag",	NS,	"sub",	"cmp",	NS,	NS,	NS,
NS,	NS,	NS,	"jmi",	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	"sta",	NS,	NS,	NS,
NS,	NS,	NS,	"and",	NS,	"ins",	"inc",	NS,
NS,	NS,	NS,	NS,	NS,	"jbs",	NS,	NS,
"bmi",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	"jge",	"sei",	NS,	"jcs",	"jcc",	NS,	NS,
NS,	NS,	"nop",	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	"org",	"jbr",	NS,	NS,
NS,	NS,	NS,	NS,	NS,	NS,	"bge",	NS,
"jgt",	"bcs",	"bcc",	NS,	NS,	NS,	NS,	"rti",
"icl",	NS,	NS,	NS,	NS,	NS,	NS,	NS,	/* mod011 */
"tst",	NS,	NS,	NS,	NS,	NS,	NS,	NS,
NS,	NS,	NS,	NS,	NS,	"bgt",	NS,	NS,
NS,	NS,	"seg",	NS,	NS,	NS,	NS,	NS
};

char maddr[512]
{
000,	000,	000,	000,	000,	o_JEQS,	000,	000,
000,	000,	000,	o_AEI,	000,	000,	o_OPT,	000,
000,	000,	000,	000,	o_DUAL,	000,	o_PUSH,	000,
000,	000,	o_INH,	o_JEQS,	000,	o_INH,	000,	000,
000,	000,	o_BNCH,	o_JEQS,	o_JEQS,	000,	000,	000,
000,	000,	000,	000,	000,	000,	o_AEI,	000,
000,	000,	000,	000,	000,	000,	000,	000,
o_BNCH,	000,	o_ZMB,	000,	000,	000,	000,	000,
o_BNCH,	o_BNCH,	000,	000,	o_JEQS,	000,	o_AEI,	000,
000,	000,	000,	000,	000,	o_AEI,	o_DUAL,	000,
000,	000,	000,	000,	o_INH,	000,	000,	o_RMB,
000,	000,	000,	000,	000,	000,	o_INH,	000,
000,	o_BNCH,	000,	000,	000,	000,	000,	000,
000,	000,	000,	000,	000,	000,	000,	000,
o_INH,	000,	000,	000,	o_AEI,	000,	000,	000,
o_PUSH,	000,	000,	o_INH,	000,	000,	000,	000,
000,	000,	000,	000,	000,	000,	000,	000,
000,	000,	000,	000,	000,	o_DUAL,	o_INH,	000,
000,	000,	000,	000,	000,	o_END,	000,	000,
000,	000,	000,	000,	000,	000,	000,	o_INH,
000,	o_EQU,	000,	000,	o_IDEI,	000,	000,	000,
000,	000,	000,	000,	000,	000,	000,	000,
000,	000,	000,	o_JMPS,	000,	000,	000,	000,
000,	o_IDEI,	000,	o_DEI,	000,	o_INH,	000,	000,
000,	000,	000,	000,	000,	000,	000,	000,
o_INH,	o_INH,	000,	000,	o_FCC,	o_JMPS,	000,	000,
o_INH,	000,	000,	000,	000,	000,	000,	000,
000,	000,	o_JEQS,	o_JEQS,	000,	000,	000,	000,
000,	000,	000,	o_DUAL,	000,	000,	000,	000,
000,	000,	o_BNCH,	000,	o_FCB,	000,	o_AEI,	000,
000,	000,	000,	000,	000,	000,	000,	o_BNCH,
o_BNCH,	000,	000,	o_FDB,	o_BNCH,	000,	000,	000,
000,	000,	000,	000,	000,	o_INH,	000,	000,
000,	000,	000,	000,	000,	o_DUAL,	000,	000,
000,	000,	000,	000,	000,	000,	000,	000,
000,	000,	000,	o_INH,	000,	000,	o_INH,	000,
o_SPC,	000,	000,	000,	000,	000,	000,	000,
000,	000,	000,	000,	000,	o_DUAL,	000,	000,
000,	000,	000,	000,	o_AEI,	000,	000,	000,
000,	000,	000,	000,	000,	o_AEI,	000,	000,
000,	000,	000,	000,	o_IDEI,	000,	o_MON,	000,
o_JEQS,	000,	o_INH,	000,	000,	000,	000,	o_INH,
o_NAM,	000,	000,	000,	000,	000,	000,	000,
000,	000,	o_INH,	o_DEI,	000,	000,	000,	000,
o_JEQS,	000,	000,	000,	000,	o_BNCH,	000,	000,
o_INH,	000,	000,	000,	000,	000,	000,	000,
o_INH,	o_AEI,	000,	000,	000,	o_INH,	000,	000,
000,	000,	000,	000,	000,	o_BNCH,	000,	000,
000,	000,	000,	000,	000,	o_DUAL,	000,	000,
000,	o_PAG,	000,	o_DUAL,	o_DUAL,	000,	000,	000,
000,	000,	000,	o_JEQS,	000,	000,	000,	000,
000,	000,	000,	000,	o_STA,	000,	000,	000,
000,	000,	000,	o_DUAL,	000,	o_INH,	o_AEI,	000,
000,	000,	000,	000,	000,	o_JBRS,	000,	000,
o_BNCH,	000,	000,	000,	000,	000,	000,	000,
000,	o_JEQS,	o_INH,	000,	o_JEQS,	o_JEQS,	000,	000,
000,	000,	o_INH,	000,	000,	000,	000,	000,
000,	000,	000,	000,	o_ORG,	o_JBRS,	000,	000,
000,	000,	000,	000,	000,	000,	o_BNCH,	000,
o_JEQS,	o_BNCH,	o_BNCH,	000,	000,	000,	000,	o_INH,
o_ICL,	000,	000,	000,	000,	000,	000,	000,	/* mod011 */
o_AEI,	000,	000,	000,	000,	000,	000,	000,
000,	000,	000,	000,	000,	o_BNCH,	000,	000,
000,	000,	o_SEG,	000,	000,	000,	000,	000
};

struct aform abuf[128];

struct iform ibuf[128];

char inbuf[518];		/* mod005 */

int hashtab[128];		/* hash table for initial addresses of local labels */

struct symbol symtab[NSYMBOL];		/* mod008 */
struct fb fbtable[NFB];			/* mod008 */
int curfb[20];

char symbuf[18];
