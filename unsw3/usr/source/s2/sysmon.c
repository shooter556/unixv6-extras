#
int df[4096] {0};
char copr[] "Copyright (c) 1976 Michael D. Tilson";

/*
 * This is a program to monitor a few system performance parameters
 * which are maintained by the clock routine.
 */

/*
 * To run this program you first need some of the Toronto changes to
 * /usr/sys/ken/clock.c.  That is easy.  Next you need Toronto graphics,
 * software and hardware.  That is hard.  It should be easy to modify
 * this program to suit your needs, however.
 */
#define	YBOT	850.
#define	XLEFT	690.
#define	HEIGHT	100.
#define	SPACING	70.
#define	NSLIDES	5
#define	MAXSUM	70
#define	MINSUM	50
#define	LOCK	"/tmp/sysmon"

#define	CHARSCALE	10

int addrs[4];	/* dimension to number of names */
char *names[]
{
	"_mon_cnt",
	"_mincore",
	"_maxcore",
	"_coremap",
	0
};
#define	MONVALS	0
#define MINCORE	1
#define	MAXCORE	2
#define	COREMAP	3

char *snames[]
{
	"%USER",
	"%SYSL",
	"%SYSH",
	"%SWAIT",
	"%IDLE",
};

struct nl
{
	char name[8];
	int type;
	int value;
} nl[NSLIDES+1];
int val[NSLIDES];
int km;

main(argc,argv)
	int argc;
	char **argv;
{
	register i;
	register sum;
	double x,fsum;

	setup(argc,argv);
	for(;;)
	{
		sleep(2);
		gopen(1);
		getvalues();
		sum=0;
		for(i=0; i<NSLIDES; i++)
			sum =+ val[i];
		if(sum<MINSUM || sum>MAXSUM)
			bad();
		x = XLEFT;
		fsum = sum;
		if(sum>0)
		{
			for(i=0; i<NSLIDES; i++)
			{
				slide(i, x, YBOT, val[i]/fsum);
				x =+ SPACING;
			}
		}
		memmap();
		post(1);
	}
}

bad()
{
	error("bad monitor data");
}

char *sp;
char strng[4];

slide(n, x, y, fract,s)
	int n;
	double x;
	double y;
	double fract;
	char *s;
{
	register int f;

	moveto(x,YBOT);
	line(0., HEIGHT);
	move(0., 30.);
	bigchars(snames[n]);
	moveto(x,y+fract*HEIGHT);
	line(8.,8.);
	line(0.,-16.);
	line(-8.,8.);
	f = fract*100.;
	sp = strng;
	num(f);
	*sp = '\0';
	moveto(x, y-30.);
	bigchars(strng);
}


num(n)
{
	register a;
	if(a=n/10)
		num(a);
	*sp++ = n%10 + '0';
}


getvalues()
{
	seek(km,addrs[MONVALS],0);
	read(km,val,sizeof val);
}


setup(argc,argv)
	int argc;
	char **argv;
{
	register i,j;
	register char *thisname;
	char *ufile;
	int lockfile;
	int pid;
	double x;

	signal(1,1);
	signal(2,1);
	signal(3,1);
	if((lockfile=open(LOCK,0))>=0)
	{
		read(lockfile,&pid,2);
		if(kill(pid,9) == -1)
			error("Can't kill");
		unlink(LOCK);
		exit();
	}
	if((km=open("/dev/kmem",0))<0)
	{
		error("kmem");
		exit();
	}
	if(argc>2)
	{
		error("Arg count");
	}
	if(argc==2)
		ufile = argv[1];
	else
		ufile = "/unix";
	for(i=0; names[i]!=0; i++)
	{
		thisname = names[i];
		for(j=0; nl[i].name[j] = *thisname++; j++)
			;
	}
	nlist(ufile,nl);
	for(i=0; names[i]!=0; i++)
		if(nl[i].type <= 0)
		{
			error("No names");
		}
		else
			addrs[i] = nl[i].value;
	if(init("gw",1024,0,1,"npal.k")<0)
		exit();
	if(fork())
		exit();
	if((lockfile=creat(LOCK,0444))<0)
		exit();
	pid = getpid();
	write(lockfile,&pid,2);
}


#define	MAXMEMSEGS	50

memmap()
{
	int mincore;
	int maxcore;
	register core;
	register i;
	struct
	{
		int sz;
		int adr;
	} a;
	double scl;

	seek(km,addrs[MINCORE],0);
	read(km,&mincore,2);
	seek(km,addrs[MAXCORE],0);
	read(km,&maxcore,2);
	if((core=maxcore-mincore)< 0140 ||
		(core > 010000) ||
		(mincore < 01200) ||
		(maxcore > 010000))
		{
			bad();
		}
	scl = ((NSLIDES-1.0)*SPACING)/(maxcore-mincore);
	seek(km,addrs[COREMAP],0);
	moveto(XLEFT, YBOT-40.);
	line(0., -10.);
	move(0., 5.);
	for(i=0; i<MAXMEMSEGS; i++)
	{
		if(read(km,&a,4)<0)
		{
			bad();
		}
		if(a.sz==0)
			break;
		lineto(XLEFT+(a.adr-mincore)*scl, YBOT-45.);
		move(a.sz*scl, 0.);
	}
	lineto(XLEFT+(NSLIDES-1.0)*SPACING, YBOT-45.);
	move(0.,5.);
	line(0., -10.);
	if(i>=MAXMEMSEGS)
		bad();
}

bigchars(s)
	char *s;
{
	schars(CHARSCALE,s);
}


error(s)
	char *s;
{
	print("Sysmon error:  ");
	print(s);
	print("\n");
	unlink(LOCK);
	exit();
}

print(s)
	char *s;
{
	while(*s)
		write(2,s++,1);
}


#ifndef	GPAC

/* The following routines form a fake GPAC.  The following rules
 * must be observed:  Only one segment, which is always ended with a post.
 * The display file must be named "df".  To redraw the segment, one must be
 * sure that there is at least a 60th sec delay between posts, as no
 * one checks.  There must be no need for clipping.  Only the routines
 * init, finish, gopen, post, moveto, move, lineto, line, chars, and
 * schars may be called.  No one checks for running out of core.
 * In return for all of the above, it runs fast and the code is small.
 *
 * This version uses a routine called "error", which is assummed to do
 * an exit().  For other uses, that should be changed.
 */
#define	DTABR	8
#define	GRAB	0
#define	MAP	1
#define	START	4
#define	STATUS	6
#define	SYNC	0010001
#define	XQT	0100001
#define	JUMP	0
#define	TERM	0100000
#define	SETS	0100223
#define	SETXY	0100222
#define	ISTATE	0107422 /* norm scale, max int, vec mode, long vec, unblank */
#define	CSTATE	0117620 /* scale 011, max int, cmode, short vec */
#define	SHORT	0100060
#define	MEDIUM	0100061
#define	LONG	0100062
#define	CHAR	0100041
#define	VECMODE	0100040
#define	SCALE	0100140
#define	IOFF1	0100204

#define fix(x) (fixdummy=(x))
int fixdummy;
int gformat -1;
int ginvis;

double curx, cury;
int curloc;
int curbuf;
int dmemsiz;
int dxqt;
int dbuffer[2];
int disp;
int lostposition 1;

init(dev,dsize,tab,segs,kfile)
	char *dev,*kfile;
{
	register dp;
	register ddp;
	register kset;
	int i;
	int ksize,com[3];
	char *gwname;
	char *kname;
	kname = "/usr/font/***************";
	for(i=10; kname[i] = *kfile++; i++)
		;
	if((kset=open(kname,0))<0)
	{
		error("kset");
	}
	for(gwname="/dev/gw0"; gwname[7]<'4'; gwname[7]++)
		if((disp=open(gwname,2))>0)
			break;
	if(gwname[7]=='4')
	{
		error("gw open");
	}
	com[0] = GRAB;
	com[1] = com[2] = dsize/1024;
	if(stty(disp,com)<0)
	{
		close(disp);
		error("Core grab");
	}
	com[0] = MAP;
	if(stty(disp,com)<0)
	{
		close(disp);
		error("gw map");
	}
	dmemsiz = dsize;
	seek(kset,4,0);
	read(kset,&ksize,2);
	seek(kset,020,0);
	read(kset,df,ksize);
	com[0] = DTABR;
	com[2] = 0;
	stty(disp,com);
	ddp = dp = (ksize+1)/2;
	dxqt = dp;
	df[dp++] = XQT | ((ddp+3)*2);
	df[dp++] = SYNC;
	df[dp++] = JUMP | (ddp*2);
	df[dp] = TERM;
	dbuffer[0] = dp;
	curloc = dbuffer[1] = (dsize+dp)/2;
	curbuf = 1;
	com[0] = START;
	com[2] = ddp*2;
	stty(disp,com);
}

finish()
{
	close(disp);
}

gopen()
{
	goutput(SETS);
	goutput(ISTATE);
	gformat = -1;
	lostposition++;
}

post()
{
	goutput(TERM);
	df[dxqt] = XQT | (dbuffer[curbuf]*2);
	if(curbuf)
		curbuf = 0;
	else
		curbuf = 1;
	curloc = dbuffer[curbuf];
}

moveto(x,y)
	double x,y;
{
	if(lostposition)
	{
		lostposition = 0;
		goutput(SETXY);
		goutput(fix(x-511.));
		goutput(fix(y-511.));
		curx = x;
		cury = y;
	}
	else
	{
		ginvis++;
		lineto(x,y);
	}
}

move(x,y)
	double x,y;
{
	moveto(curx+x,cury+y);
}

lineto(x,y)
	double x,y;
{
	line(x-curx,y-cury);
}

line(x,y)
	double x,y;
{
	register dx,dy;

	dx = x;
	dy = y;
	curx =+ dx;
	cury =+ dy;
	if(abs(dx)<=127 && abs(dy)<=127)
	{
		gsetformat(MEDIUM);
		if(ginvis)
			goutput(IOFF1);
		goutput(dy|(dx<<8));
	}
	else
	{
		gsetformat(LONG);
		if(ginvis)
			goutput(IOFF1);
		goutput(dy);
		goutput(dx);
	}
	ginvis = 0;
}

chars(s)
	char *s;
{
	register a,b;

	gsetformat(SHORT);
	goutput(CHAR);
	while(a = *s++)
	{
		b = *s++;
		goutput(a|(b<<8));
		if(b==0)
			break;
	}
	goutput(VECMODE);
	lostposition++;
}

schars(scl, s)
	char *s;
{
	goutput(SCALE+scl);
	chars(s);
	goutput(SCALE+8);
}

goutput(n)
{
	if((curloc>=dmemsiz-2 || (curbuf==0 &&  curloc >= dbuffer[1]-2))
		&& n!=TERM)
	{
		error("gw out of core");
	}
	df[curloc++] = n;
}

gsetformat(f)
{
	if(gformat==f)
		return;
	gformat = f;
	goutput(f);
}

#endif
