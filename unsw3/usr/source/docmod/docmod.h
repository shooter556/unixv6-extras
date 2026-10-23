#

/* docmod.h */

extern	int	*reset();

struct
{	int	klfp;
	char	keyword[4];
}kwrec	{0,"    "};
int	end1	0;

struct
{	int	nklfp;
	int	tp;
}klrec;

struct
{	int	afp;
	char	title[70];
}trec;

struct arec
{	int	nafp;
	char	text[70];
}arec;

int	kwfid,klfid,tfid,afid;

int	intitle;		/*terminal input*/
char	inkw[4];

char	*emsg[]
{	"can't open file",
	"file read error",
	"keyword not found",
	"no references to this keyword",
	"no title for this number",
	"no abstract for this title",
	"arg count",
	"file write error",
	"file seek error",
	"can't exec editor",
	"invalid file name"
};

#define	OK	0
#define	LAST	-1
#define	FREE	-2
#define	EOF	-3
#define	ERSEEK	-4
#define	ERREAD	-5
#define	EROPEN	-6
#define	NOABSTR	-7
#define	NOKEYW	-8
#define	NOREF	-9
#define	NOTITLE	-10
#define	ARGS	-11
#define	ERWRITE	-12
#define	ERFORK	-13
#define	EREXEC	-14
#define	ERFNAM	-15

#define	HELP	-20
#define	TITLE	-21
#define	KEYWORD	-22
#define	LK	-23
#define	LT	-24
#define	LA	-25
#define	EDIT	-26
#define	INSERT	-27
#define	INSKW	-28
#define	LINK	-29
#define	DELKW	-30
#define	DELT	-31
#define	KILINK	-32
#define	LISTKL	-33
#define	REPLACE	-34
#define	TWRITE	-35

#define	BLANK	040

char	buff[50];
