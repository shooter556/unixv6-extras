#
/*
 *	Reload root disk from tape to hp disk
 */

#define	HP	0176700
#define	TM	0172520
#define	TK	0177560

struct
{	int	tks;
	int	tkb;
	int	tps;
	int	tpb;
};

struct
{
	int	hpcs1;
	int	hpwc;
	int	hpba;
	int	hpda;
	int	hpcs2;
	int	hpds;
	int	hper1;
	int	hpas;
	int	hpla;
	int	hpdb;
	int	hpmr;
	int	hpdt;
	int	hpsn;
	int	hpof;
	int	hpca;
	int	hpcc;
	int	hper2;
	int	hper3;
	int	hpec1;
	int	hpec2;
	int	hpbae;
};

struct
{
	int	tmer;
	int	tmcs;
	int	tmbc;
	int	tmba;
	int	tmdb;
	int	tmrd;
};

#define	NUM	601
#define	READY	0200
#define	GO	01
#define	PRESET	020
#define	RECAL	06
#define	RCLR	010
#define	OFFSET	014
#define	PIP	020000
#define	ERR	040000
#define	DU	040000
#define	DTE	010000
#define	OPI	020000
#define	CLR	040
#define	FMT22	010000

copy()
{
	register i;

	HP->hpcs2 = CLR;
	HP->hpcs1 = RCLR|GO;
	HP->hpcs1 = PRESET|GO;
	HP->hpof = FMT22;

	for(i=0; i<NUM; i++) {
		read(i);
		write(i);
	}
	boot();
}

int	buf[4096];

read(i)
{
	register retry;

loop:
	TM->tmbc = -8192;
	TM->tmba = buf;
	TM->tmcs =  060003;
	while((TM->tmcs & READY) == 0);
	if(TM->tmcs < 0) {
		while(TM->tmrd & 010000);
		if(++retry <  10) {
			printf("Retry read on %d\n", i);
			TM->tmbc = -1;
			TM->tmcs = 060013;
			while((TM->tmcs & READY) == 0);
			if(TM->tmcs < 0) {
				printf("Error recovery failed %d\n", i);
				tmregs();
				return;
			}
			goto loop;
		} else {
			printf("Retries terminated %d\n", i);
			tmregs();
		}
	} else
		if(TM->tmbc != 0)
			printf("Record %d short; bc=%d\n", i, 8192 + TM->tmbc);
}

write(i)
{
	register retry, cnt;
	extern ldivr;

	retry = 0;
	cnt = 0;

loop:
	HP->hpca = ldiv(0, 16*i, 19*22);
	HP->hpda = ((ldivr/22) << 8) | (ldivr%22);
	HP->hpwc = -4096;
	HP->hpba = buf;
	HP->hpcs1 =  061;
	while((HP->hpcs1 & READY) == 0);
	if(HP->hpcs1 & ERR)  {
		if(HP->hper1 & (DU|DTE|OPI)) {
			HP->hpcs2 = CLR;
			HP->hpcs1 = RECAL|GO;
			while((HP->hpds&PIP) && --cnt);;
		}
		HP->hpcs1 = RCLR|GO;
		if(++retry < 10) {
			printf("Retry write on %d\n", i);
			goto loop;
		}
		printf("Retry count exhausted\n");
		hpregs();
	}
}

tmregs()
{
	printf("tmer  %7o   tmcs  %7o   tmbc  %7o\n", TM->tmer, TM->tmcs, TM->tmbc);
	printf("tmba  %7o   tmdb  %7o   tmrd  %7o\n", TM->tmba, TM->tmdb, TM->tmrd);
}

hpregs()
{
	printf("hpcs1 %7o   hpwc  %7o   hpba  %7o\n", HP->hpcs1, HP->hpwc, HP->hpba);
	printf("hpda  %7o   hpcs2 %7o   hpds  %7o\n", HP->hpda, HP->hpcs2, HP->hpds);
	printf("hper1 %7o   hpas  %7o   hpla  %7o\n", HP->hper1, HP->hpas, HP->hpla);
	printf("hpdb  %7o   hpmr  %7o   hpdt  %7o\n", HP->hpdb, HP->hpmr, HP->hpdt);
	printf("hpsn  %7o   hpof  %7o   hpca  %7o\n", HP->hpsn, HP->hpof, HP->hpca);
	printf("hpcc  %7o   hper2 %7o   hper3 %7o\n", HP->hpcc, HP->hper2, HP->hper3);
	printf("hpec1 %7o   hpec2 %7o   hpbae %7o\n", HP->hpec1, HP->hpec2, HP->hpbae);
}

putchar(c)
{
	if(c == '\n')
		putchar('\r');
	TK->tpb = c;
	while((TK->tps & READY) == 0);
}

boot()
{
	static	int	*bootgo;

	for(;;) {
		HP->hpca = 0;
		HP->hpda = 0;
		HP->hpba = 0;
		HP->hpwc = -256;
		HP->hpcs1 = 071;
		while((HP->hpcs1 & READY) == 0);
		if(HP->hpcs1 & ERR) {
			if(HP->hper1&(DU|DTE|OPI)) {
				HP->hpcs2 = CLR;
				HP->hpcs1 = RECAL|GO;
				while((HP->hpds&PIP));
			}
			HP->hpcs1 = RCLR|GO;
			continue;
		}
		goto bootgo;
	}
}
