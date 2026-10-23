#

/*
 * Common file for "slup", the new super-duper
 * source librarian and update program.
 */

#define	LMAGIC	0121212	/* magic number identifier */
#define	VERSION	"(v2.1)"	/* version number of slup */

#define	NAMLENG	14	/* length of names */ /* DIRSIZ */

#define	LINESPP	60	/* lines per listing page */

#define	NULL	0	/* null pointer */

#define	NCALLS	10	/* maximum call nesting for extractions */

/* command flags */
#define	INSERT	1	/* insert module */
#define	EXTRACT	2	/* extract module */
#define	EXORIG	3	/* extract original source module */
#define	EXMODS	4	/* extract entire list of changes */
#define	DELETE	5	/* delete modules */
#define	DELUPDT	6	/* delete updates */
#define	UPDATE	7	/* update */
#define	COMMON	8	/* tag files as common */
#define	INSGRP	9	/* insert files into groups */
#define	DELGRP	10	/* remove files from group */

/* update format control bytes */
#define	UOWN0	1	/* set owner of next lines - after */
#define	UOWN1	2	/* set owner of next lines - before */
#define	UOWN2	3	/* set owner of next lines - append */
#define	UDEL	4	/* delete next lines */
#define	URESTOR	5	/* restor next lines */

/* option bits in "options" */
#define	OPTNEWL	0000001	/* create a new library */
#define	OPTGRPS	0000002	/* groups have been specified */
#define	OPTREPL	0000004	/* replace modified files in library */
#define	OPTXTR	0000010	/* extract compile file at end */
#define	OPTPRNT	0000020	/* print listing of compile file at end */
#define	OPTLIST	0000040	/* list library contents at end */
#define	OPTOFIL	0000100	/* output list file specified */
#define	OPTDUAL	0000200	/* delete update (all) */
#define	OPTTEST	0000400	/* only testing updates */
#define	OPTCOMM	0001000	/* common status on */

/*
 * suboptions
 */
/* extract */
#define	OPTXNCL	0000001	/* don't do calls when extracting */
#define	OPTXCC	0000002	/* C-compiler mode output */
/* print */
#define	OPTPCOM	0000010	/* print included common files */
#define	OPTPDEL	0000020	/* print deleted lines as well */
#define	OPTPYNK	0000040	/* print yanked modsets */
#define	OPTPDIR	0000100	/* print directives */
/* update */
#define	OPTUPRT	0000200	/* print modifications made to files */
/* list */
#define	OPTLBIG	0000400	/* long listing */
#define	OPTLGRP	0001000	/* list group names */
#define	OPTLMOD	0002000	/* list modset names */
#define	OPTLXRF	0004000	/* list modset cross-reference */
#define	OPTLDOC	0010000	/* list modset documentation */

/**********************************************************************/

/* Error message numbers */
#define	NOERR	0	/* good return */
#define	ERR	-1	/* any error */
#define	ERRIO	-2	/* i/o error */
#define	ERROLD	-3	/* can't open old file */
#define	ERRNEW	-4	/* can't create new file */
#define	ERRTEMP	-5	/* can't create temporary file */
#define	ERRMEM	-6	/* insufficient memory */
#define	ERR2MNY	-7	/* too many arguments */
#define	ERR2FEW	-8	/* too few arguments */
#define	ERRLINE	-9	/* line too long */
#define	ERRCODE	-10	/* non-ascii */
#define	ERRNLNG	-11	/* name too long */
#define	ERRLEAD	-12	/* first character of name must be alphabetic */
#define	ERRNMCH	-13	/* illegal character in name */
#define	ERRUDEL	-14	/* reference to deleted modset */
#define	ERRDIRC	-15	/* illegal update directive */
#define	ERRNDIG	-16	/* non-digit encountered */
#define	ERRNFIL	-17	/* no file specified for modification */
#define	ERRNMOD	-18	/* no modification set name specified */
#define	ERR2FAR	-19	/* specified line too high */
#define	ERRPAST	-20	/* past position in file */
#define	ERRPREF	-21	/* illegal prefix */
#define	ERRYANK	-22	/* reference to yanked modset */
#define	ERRFFND	-23	/* can't find file entry in library */
#define	ERRFDUP	-24	/* duplicate file name */
#define	ERRMFND	-25	/* can't find modset */
#define	ERRMDUP	-26	/* duplicate modset name */
#define	ERRGFND	-27	/* can't find group name in library */
#define	ERRGDUP	-28	/* duplicate group name */
#define	ERRDONE	-29	/* module already in file */
#define	ERRCALL	-30	/* too many calls */
#define	ERRIMPL	-99	/* not yet implemented */
/* spare 31 */

#define	ERROR(i)	{ i; goto errlabl; }

/**********************************************************************/

#define	REWIND(f)	seek(f, 0, 0)
#define	TRUNC(nm)	close(creat(nm, 0600))

/**********************************************************************/

/*
 * Beginning of library file
 */

struct	lheader {
	int	l_magic;	/* magic number */
	long	l_crdate;	/* creation date */
	long	l_modate;	/* last modification date */
	unsigned	l_files;	/* no. of files in library */
	unsigned	l_mods;	/* no. of modifications */
	long	l_size;		/* size of library */
	char	l_groups[16][NAMLENG];	/* group names */
	} lheader;

int	libmods;		/* flag meaning that library was modified */

/**********************************************************************/

/*
 * subfile directory - follows "lheader"
 */

struct	filentry {
	char	f_name[NAMLENG];	/* file name */
	long	f_date;		/* module inserted */
	long	f_base;		/* subfile address */
	long	f_size;		/* size of subfile */
	int	f_groups;	/* mask of containing groups */
	unsigned	f_mods;	/* number of modification sets applied */
	int	f_flags;	/* file flags */
	} *filentry;

int	nfiles;			/* entries in directory (# of mntentries/filentries) */
int	mxnfiles;		/* memory allocated for directory */

/* file flags bits */
#define	FXCOMMN	0001	/* common file */
#define	FXNEW	0002	/* updates only file - newdeck */
#define	FXRUN	0100	/* touched file */
#define	FXUPDAT	0200	/* updated this run */

/**********************************************************************/

/*
 * Modification directory - follows subfile directory
 */

struct	modentry {
	char	m_name[NAMLENG];	/* modification name */
	long	m_date;		/* date of modification */
	unsigned	m_refs;	/* no. of subfiles modified */
	int	m_flags;	/* modification flags */
	long	m_base;		/* beginning of documentation */
	long	m_size;		/* size of area */
	} *modentry;

int	nmods;		/* no. of modifications in directory */
int	mxnmods;	/* size of table */

#define	MXRUN	0000100	/* touched modset */
#define	MUUPDAT	0000200	/* this one updated */

int	docfile;	/* file descriptor of documentation temporary */
char	*docname;	/* name of documentation temporary */
long	docleng;	/* leng of documentation temporary */

/**********************************************************************/

/*
 * At beginning of subfile
 */

struct	uentry {
	char	u_name[NAMLENG];	/* name of modset */
	char	u_flags;	/* update flags */
	char	u_pad;
	} *uentry;

struct	uwork {
	char	w_name[NAMLENG];	/* name of modset */
	char	w_flags;	/* modification flags */
	unsigned	w_lines;	/* current line number */
	char	w_cmd;		/* command type (del, rest etc.) */
				/* zero if insert */
	char	w_modfind;	/* search modset */
				/* ignored if insert */
	unsigned	w_count;	/* lines involved */
	} *uwork;

unsigned	works;		/* no. of work entries */
unsigned	mxworks;	/* allocated memory */

/* update flags bits */
#define	UXYANKD	0001	/* this modset has been yanked */
#define	UXEND	0100	/* doing append */
#define	UXRUN	0200	/* delete or yank this modset */

/**********************************************************************/

/*
 * used for:
 *	group names
 *	du (modsets)
 *	xu (modsets)
 */

struct	listitem {
	char	s_name[NAMLENG];	/* name of something */
	};

struct	namelist {
	unsigned	s_mxcnt;	/* size of buffer */
	unsigned	s_cnt;		/* names in buffer */
	struct listitem	*s_list;	/* buffer */
	};

struct namelist	grplist;	/* for group reference */
struct namelist	mgrplist;	/* names to be added */
struct namelist	rgrplist;	/* names to be removed */
struct namelist	igrplist;	/* files to be grouped */
struct namelist	dgrplist;	/* files to be ungrouped */

struct namelist	dmodlist;	/* modsets to be deleted */
struct namelist	xmodlist;	/* modsets to be extracted */

struct namelist	modfiles;	/* files mod'ed by this modset */

/**********************************************************************/

int	libfile;		/* file-descriptor */
char	*libname;		/* library name */
char	libnbuf[100];		/* library name is stored here */

char	*scrtemp;		/* scratch file */
int	scrfid;			/* file-descriptor for scratch file */
long	blkbase;		/* base of allocated disk region */

char	*worktemp;		/* work file */
int	workfid;		/* file-descriptor for work file */

int	insfile;		/* file currently being inserted into library */

/**********************************************************************/

/* character i/o */

/* structure of file control block */
struct	ubuf {
	int	io_fid;		/* file descriptor */
	int	io_cnt;		/* characters left in buffer */
	char	*io_ptr;	/* next spot in buffer */
	char	io_buf[512];	/* buffer. */
	};

#define	getc(iop)	((--(iop)->io_cnt >= 0) ? (*(iop)->io_ptr++ & 0377) : fload(iop))
#define	ungetc(iop)	(*--(iop)->io_ptr = (c), (iop)->io_cnt++)
#define	putc(c,iop)	( *(iop)->io_ptr++ = (c), ((--(iop)->io_cnt) > 0) ? 0 : fflush(iop) )
#define	FINSET(iop,f)	{ (iop)->io_fid = f; (iop)->io_cnt = 0; }
#define	FOUTSET(iop,f)	{ (iop)->io_fid = f; (iop)->io_cnt = sizeof (iop)->io_buf; (iop)->io_ptr = (iop)->io_buf; }
#define	unread(iop)	(seek((iop)->io_fid, -(iop)->io_cnt, 1))
#define	FIRWND(iop)	{ REWIND((iop)->io_fid); (iop)->io_cnt = 0; }
#define	FORWND(iop)	{ REWIND((iop)->io_fid); (iop)->io_cnt = sizeof (iop)->io_buf; (iop)->io_ptr = (iop)->io_buf; }

struct ubuf	ftmp1, ftmp2;
long	ulngi, ulngo;		/* length of files */

char	*utemp;			/* update temporary filename */
int	ufid;			/* update filedescriptor */

struct ubuf	ufsav;
long	ulngsav;		/* length of save file */
unsigned	savlines;	/* number of lines in save file */

int	modfout;		/* output of "-xu" command */
char	*modoutn;		/* name of -xu output file */

/**********************************************************************/

/* listings */

int	prntmode;		/* current printing mode selected:
				 *	0.	console
				 *	1.	extraction listing
				 *	2.	update listing
				 *	3.	yank listing
				 *	4.	Documentation listing
				 */

struct ubuf	fout;
struct ubuf	fin;

char	*lstname;		/* temporary name for listings */
int	lstfile;		/* file-descriptor for listing temp */

int	lineposn;		/* line counter for page size */
int	fileno;			/* page number for top of page */
int	pageno;			/* page number within file */
char	*nowtime;		/* current time for top of page */
char	*title;			/* page title */

char	*descrip;		/* description of this operation */
char	*filename;		/* name of the currently active file */
int	promptdone;		/* description typed flag */

unsigned	linenum;	/* line number on output listing */

/**********************************************************************/

/* update variables */

char	upeekc;			/* single character lookahead buffer */
char	dfltmod[NAMLENG];	/* default modset name */

/* structure for carrying line specifiers */
struct	tag {
	char	t_name[NAMLENG];	/* modset name */
	int	t_line;		/* line number */
	} tag1, tag2;

char	thismod[NAMLENG+1];	/* name of current modset */
char	updtid;			/* modset i.d. of current modset (being added) */
				/* should be == (works - 1) */
char	scanid;			/* modset i.d. currently positioned at */
char	srchid;			/* modset i.d. to be searched for */
unsigned	srchlin;	/* line number to be searched for */
int	posnmode;		/* true for before instead of after */
int	unewflag;		/* true if new module */
int	uappndf;		/* in append mode */

/**********************************************************************/

int	func;			/* major command */
int	options;		/* mask of list options */
int	subopts;		/* suboptions */
int	verbose;		/* verbose mode */
int	dfltescape;		/* initial escape */
int	grpmask;		/* all mentioned groups */

long	now;			/* current time/date */

int	errflag;

char	aline[150];		/* text buffer */
char	*alinep;		/* cursor for buffer */
char	workb[150];		/* working text buffer */
char	*workp;			/* cursor for working buffer */

/**********************************************************************/

char	workbuf[100];		/* work area for file-names */

int	statbuf[18];		/* buffer for doing stat's */

/*
 * update commands
 */
#define	UTEXT	0	/* pure and simple text line */
#define	IDENT	1
#define	DECK	2
#define	NEWDECK	3
#define	AFTER	4
#define	BEFORE	5
#define	UAPPEND	6
#define	UDELETE	7
#define	RESTORE	8
#define	PREFIX	9
#define	MODNAME	10
#define	EXIT	11
#define	COMMF	12
#define	COMMK	13
#define	YANK	14
#define	UNYANK	15
#define	CALL	16
#define	CPREFIX	17

struct	cmds {
	int	cmd;
	char	*name1;
	char	*name2;
	};

/**********************************************************************/

/* System externals */
long	time();
int	*localtime();
char	*ctime();
char	*alloc();
int	free();
int	mktemp();
int	creat();
int	open();
int	read();
int	write();
int	seek();
int	lseek();
int	close();
int	stat();
int	printf();
int	unlink();
int	fork();
int	execl();
int	exit();
