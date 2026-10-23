/*	UNIX OVERLAY LOADER
 *	J. N. Rottman
 *
 *	Structure definitions,
 *	Global variables
 */

#define	LIBBS	10
#define	ARCMAGIC	0177555
#define	AUTOL		010
#define	EXCLF		020
#define	ANCHORED	0000001
#define	SIGINT	2

#define	RABS	00
#define	RTXT	02
#define	RDAT	04
#define	RBSS	06
#define	REXT	010


#define	SIZE_SEG	12
#define	SIZE_AUT	8

#define	FILE 	1
#define	INCL	2
#define	EXCL	3

#define	NIL	0
#define	STR	1

#define	S_COMMON	10
#define	S_FILES		6
#define	S_AUTOS		10
#define	S_CALLS		4

#define	EXT	040
#define	UND	00
#define	ABS	01
#define	TXT	02
#define	DAT	03
#define	BSS	04
#define	COM	05
#define	EXS	06

#define	RELFLG	01

#define	GOBBLE	512
#define	NFILES	5
#define	ACTSZ	60

struct	arhd	{
	char	aname[8];
	int	atime[2];
	char	auid;
	char	amode;
	int	asize;
} arhd;

struct	magic	{
	int	mag_num;
	int	mag_txt;
	int	mag_dat;
	int	mag_bss;
	int	mag_smt;
	int	mag_fil[2];
	int	mag_rel;
} magic;

struct {
	char	i_name[8];
	char	i_type;
	char	i_pad;
	char	*i_value;
} i_sym;

struct	symbol	{
	char	sname[8];
	char	stype;
	char	sflags;
	char	*svalue;
	struct	symbol	*srson;
	struct	symbol	*slson;
};

struct	page	{
	int	nuser;
	int	bno;
	int	nibuf;
	int	buff[256];
} page[2];

struct	{
	int	nuser;
	int	bno;
} fpage;

struct	stream	{
	int	*ptr;
	int	bno;
	int	nibuf;
	int	size;
	struct	page	*pno;
};

int	x_infile;
char	*x_inname;
int	x_stat;

struct	stream	text;
struct	stream	reloc;

struct	libn	{
	struct	{
		int	l_bno;
		int	l_off;
	} libns[LIBBS];
	struct	libn	*libnext;
};

struct	{
	struct	libn	*libmod;
	int	libind;
} lib_over, lib_pool;

struct	libn	*libroot, *libact;
int	libcount;

int	symnum;


struct	autos	{
	struct	autos	*a_next;
	char	a_name[8];
};

struct	common	{
	struct	common	*n_next;
	char	n_name[8];
};

struct	calls	{
	struct	calls	*c_next;
	char	*c_pnt;
};

struct	files	{
	struct	files	*f_next;
	int	f_type;
	char	*f_name;
};

struct	{
	char	*tss;
	char	*dss;
	char	*bss;
	char	*sss;
	int	residue;
} res;

struct	seg	{
	int	q_by;
	int	q_bl;
	int	q_rs;
	char	*q_lc;
	char	*q_hc;
	int	q_sn;
};

struct	autol	{
	struct	seg	*jsrseg;
	char	*jsraddr;
};

struct	seg	*op_segs;
struct	seg	*qp_segs;
struct	autol	*op_autos;
struct	autol	*qp_autos;
char	**op_sss;
char	**qp_sss;
char	*ov_lp;
char	*ov_seg;
char	*ov_ss;
char	*ov_se;
char	*ov_nm;
char	*ov_av;
int	oregion;

struct	segment	{

	int	s_num;

	struct	files	*s_files;
	struct	calls	*s_calls;
	struct	autos	*s_autos;
	struct	common	*s_common;
	struct	segment	*s_lseg;
	struct	segment	*s_ansc;

	int	s_flags;

	char	*s_ts, *s_ds, *s_bs, *s_cs;
	char	*s_to, *s_do, *s_bo, *s_co;

	char	*s_so;
	char	*s_crs;
	char	*s_dks;
	char	*s_dkb;
	char	*s_dkc;

	struct	symbol	*s_st;

};

struct	segment	u_root;
struct	segment	u_pool;

int	e_segs;
int	e_autos;

int	cnt_err;
int	cnt_und;

char	*memlow;
char	*memhigh;


int	debug;
int	mflag;
int	nflag;
int	xflag;
int	yflag;
int	eflag;

struct	segment	*seg_list;

char	*overname;
int	fout;

struct	iobuf	{
	int	io_fd;
	int	io_un;
	int	*io_nx;
	int	io_bf[256];
};


char	*tname;

struct	symbol	**vect_s;

char	*txt_j;
char	*dat_j;
char	*bss_j;

int	size_smm;
