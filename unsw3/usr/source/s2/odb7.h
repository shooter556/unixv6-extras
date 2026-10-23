#define		BMASK	0377
#define		EOT	04
#define		RUB	0177
#define		ODBINT	1
#define		ODBEXT	0
#define		REGIST	2
#define		NOOFBP	9
extern		struct list {
				int badr;
				int bcnt;
				int bcount;
			    } bkpt[];
extern		int	bytwrd,dot,valfnd,adrval;
extern		int	txtsiz,symfnd,symfil,symsiz,symcntr;
extern		int	corfil,curfil,orgfil;
extern		int	r[],tadrval,scflg,svalfnd;
extern		int	sign;
extern		char	*lp,line[];
extern		char	ch,cmode,symbuf[];
extern		char	symbol[];
extern		char	*lbp,linbuf[],cfdes;
extern		char	*regs[];
extern		int	corbuf[];
extern		char	**users;
extern		char	*sfil;
extern		int	savebp,pid;
