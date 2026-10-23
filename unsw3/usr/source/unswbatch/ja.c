#

	/* Ja.c - Job accounting program */

#define LOCAL	010
#define CYBER	020
#define ERROR	040
#define DISPOSE	024

struct INODE
	{
	char	minor;
	char	major;
	int	inumber;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	int	actime[2];
	int	modtime[2];
	};

char	*first 0,
	*limit 0,
	*start 0,
	*next 0,
	*temp 0;

int	less(), scribes();
int	compar &less,		/* comparison routine for insert() */
	(*action)() &scribes,	/* print routine for scan() */
	seqno 0,
	base[2] {0,0},		/* time for current batch process */
				/* status of last time() call */
	minute 0,
	day 0,
	hour 0,
	days 0,
	year 0,
	jobdes 0,			/* file descriptor for jobs' file */
	sumdes 0,			/* file descriptor for summary file */
	root 0;			/*  root of binary tree */
				/* typical entry in summary file (one for each day) */
struct SUMMARY
	{
	int	s_origin[5];
	int	s_local[2];
	int	s_send[2];
	int	s_received[2];
	int	s_printed[2];
	} summary[15] 0;
				/* typical entry in jobs' file */
struct JOB
	{
	char	j_jobname[7];
	char	j_extension[3];
	int	j_read;
	int	j_sent;
	int	j_received;
	int	j_printing;
	int	j_exit;
	char	j_origin;
	} *item 0;

struct NODE
	{
	struct JOB  *n_information;
	struct NODE *n_less;
	struct NODE *n_great;
	} *stack 0;

char *jobf ".work/.rawstatistics";
char *sumf ".work/.coostatistics";
char *temf "/tmp/jatemf";

abort(string)
	{

	bye(string);
	unlink(temf);
	exit(-1);
	}

bye(string)
	{
	register char *p;

	for (p = string; *p++; );
	p[-1] = '\n';
	write(1,"Ja: ",4);
	write(1,string,p-string);
	}

ciaou(string)
	{
	extern int fout;

	update(1);
	bye(string);
	close(jobdes); close(sumdes);
	flush(); close(fout);
	execl("/bin/lpr","lpr","-r","-n2","-ijoblist",temf,0);
	abort("can't find 'lpr'");
	}

char *ll_header
	{
"comparison of average waiting times for local jobs (at hourly intervals)"
	};
char *l_header
	{
" read		   wait		  0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16"
	};
char *cc_header
	{
"comparison of average delays times (to/from) for cyber jobs (at hourly intervals)"
	};
char *c_header
	{
" read         to        from      0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16"
	};

graph()
	{
	register struct SUMMARY *sm;
	int hr,a,b,t;

	printf("\014%31.21s%88s\n\n%s\n\n",ytime(0),ll_header,l_header);
	printf("%25s\n\n","dd hh mm");
	for (hr = 7, sm = &summary[0]; sm < &summary[15]; hr++, sm++)
		{
		interval(hr);
		puts(" !      ");
		a = 5 + delay(ldiv(sm->s_local,sm->s_origin[0]));
		puts(" -      ! ");
		for (a =/ 10, t = 100; t-- > 0 && a-- > 0; putchar('+'));
		putchar('\n');
		}
	printf("\n\n\n\n%31.21s%92s\n\n%s\n\n",ytime(0),cc_header,c_header);
	printf("%30s\n\n","dd hh mm   dd hh mm");
	for (hr = 7, sm = &summary[0]; sm < &summary[15]; hr++, sm++)
		{
		interval(hr);
		puts(" !");
		a = 5 + delay(ldiv(sm->s_send,t = sm->s_origin[1]+sm->s_origin[2]));
		b = 5 + delay(ldiv(sm->s_received,t));
		puts(" - ! ");
		for (a =/ 10, t = 100; t-- > 0 && a-- > 0; putchar('+'));
		for (b =/ 10; t-- > 0 && b-- > 0; putchar('*'));
		putchar('\n');
		}
	}

int wc[7] { 4,5,-1,0,1,2,3 };
int dc[7] { 3,4,0,0,0,1,2  };

#define size sizeof summary

incore()
	{
	register int *p;

	time(0);
	if (read(position(0,dc[day]+5*((days+wc[(5*year-349>>2)%7])/7),size),summary,size) != size)
		for (p = &summary[0]; p < &summary[15]; *p++ = 0);
	}

#define BUFLEN 10240

init()
	{
	char linbuf[256];
	struct INODE inode[1];
	extern int fout;
	int length;

	if (stat(temf,inode) != -1)
		{
		bye("already active");
		exit(-1);
		}
	if ((fout = creat(temf,0600)) < 0)
		abort("can't create temporary file");
	close(0); close(2);
	if ((jobdes = open(jobf,0)) < 0)
		abort("can't open jobs' file");
	if ((sumdes = open(sumf,2)) < 0)
		if ((sumdes = creat(sumf,0644)) < 0)
			abort("can't create summary file");
	fstat(jobdes,inode);
	if ((length = inode->size0 || inode->size1 > BUFLEN ? BUFLEN : inode->size1) <= 6)
		abort("no jobs processed");
	if (length&1)
		abort("jobs' file is of odd length");
	if ((first = sbrk(14*(length/11))) == -1)
		abort("memory overflow");
	stack = limit = (next = item = first)+length;
	if (inode->size0 == 0 && inode->size1 == length)
		{
		read(jobdes,first,length);
		next =+ length;
		}
	else
		readtil(limit);
	if (*first != 0)
		abort("format error on jobs' file");
	write(1,"Are you sure batch is stopped - ",32);
	read(1,linbuf,256);
	if (linbuf[0] != 'y')
		{
		unlink(temf);
		exit(0);
		}
	if ((length = fork()) < 0)
		abort("try again");
	if (length == 0)
		{
		execl("stop","stop",0);
		abort("can't find 'stop'");
		}
	wait(&length);
	if ((length = fork()) == -1)
		abort("try again");
	if (length != 0)
		exit(0);
	signal(2,1); signal(3,1);
	}

insert()
	{
	register int *p, *q, (*cmp)();
	char *c;

	if (item->j_origin&LOCAL)
		item->j_extension[1] = item->j_extension[2] = 0;
	for (c = &item->j_jobname[7]; --c >= item->j_jobname; )
		if (*c >= 'a' && *c <= 'z')
			*c =- 'a' - 'A';
	for (p = &root, cmp = compar; *p; )
		{
		p = *p;
		if ((*cmp)(*p++))
			p++;
		}
	*p = q = stack++;
	*q++ = item;
	*q++ = 0;
	*q = 0;
	}

interval(hr)
	{
	switch (hr)
		{
	default:
		printf("%2d:30 ",hr); break;
	case 7:
		puts("before"); break;
	case 21:
		puts("after ");
		}
	}

less(jb)
struct JOB *jb;
	{
	register char *p,*q,*r;

	for (p = item->j_jobname,q = jb->j_jobname,r = &item->j_jobname[20]; *p == *q++; )
		if (p++ >= r)
			return(0);
	return(*p > *--q);
	}

origin(org)
	{
	switch (org&070)
		{
	default:
		return(4);
	case LOCAL:
		return(0);
	case CYBER:
		switch (org&07)
			{
		default:
			return(4);
		case 01:
			return(1);
		case 02:
			return(2);
		case 04:
			return(3);
			}
		}
	}

char *null "";
char *fmt " - %8.8s";
char *noentry " -         ";
char *origins[]
	{
	"loc",
	"cyb",
	"tty",
	"dis",
	"err"
	};

scribes(job)
struct JOB *job;
	{
	register struct JOB *jb;
	register struct SUMMARY *sm;
	register int i;

	jb = job;
	if (seqno++ % 56 == 0)
		{
		printf("\014%25.21s%9s%14s%11s%9s%10s%35s%21s\n\n",
			ytime(0),"read","sent/start","back/stop","print","exit","delay at each stage","total");
		puts("        jobname/ext   org");
		for (i = 11; --i > 0; )
			printf("   dd hh%cmm",i > 5 ? ':' : ' ');
		puts("\n\n");
		}
	printf("%5d - %7.7s%c%3.3s - %3.3s",
		seqno,
		jb->j_jobname,
		(jb->j_origin&CYBER) && (jb->j_origin&7) != 4 ? '/' : ' ',
		(jb->j_origin&CYBER) && (jb->j_origin&7) != 4 ? jb->j_extension : null,
		origins[origin(jb->j_origin)]);
	time(0);
	if ((jb->j_origin&077) == DISPOSE)
		puts(noentry);
	else
		printf(fmt,time(jb->j_read));
	if (jb->j_origin&ERROR)
		for (i = 5; --i > 0; puts(noentry));
	else
		{
		if ((jb->j_origin&077) == DISPOSE)
			puts(noentry);
		else
			printf(fmt,time(jb->j_sent));
		printf(fmt,time(jb->j_received));
		printf(fmt,time(jb->j_printing));
		printf(fmt,time(jb->j_exit));
		}
	time((jb->j_origin&077) == DISPOSE ? jb->j_received : jb->j_read);
	if ((hour =- 8) < 0)
		hour = 0;
	else
		if (hour > 13)
			hour = 14;
		else
			if (minute > 29)
				hour++;
	sm = &summary[hour];
	if (jb->j_origin&ERROR)
		for(i = 6; --i > 0; puts(noentry));
	else
		{
		if ((jb->j_origin&077) == DISPOSE)
			{
			puts(noentry); puts(noentry);
			}
		else
			if (jb->j_origin&LOCAL)
				{
				add(sm->s_local,delay(jb->j_sent-jb->j_read));
				delay(jb->j_received-jb->j_sent);
				}
			else
				{
				add(sm->s_send,delay(jb->j_sent-jb->j_read));
				add(sm->s_received,delay(jb->j_received-jb->j_sent));
				}
		add(sm->s_printed,delay(jb->j_printing-jb->j_received));
		delay(jb->j_exit-jb->j_printing);
		delay(jb->j_exit-((jb->j_origin&077) == DISPOSE ? jb->j_received : jb->j_read));
		}
	sm->s_origin[origin(jb->j_origin)]++;
	putchar('\n');
	}

puts(string)
	{
	register char *p;

	for (p = string; putchar(*p++); );
	}

readtil(addr)
char *addr;
	{
	register int num;

	if ((num = read(jobdes,next,addr-next&~0777)) != 0)
		{
		if (num == -1)
			ciaou("read error on jobs' file");
		if (num&1)
			ciaou("odd boundary on jobs' file");
		next =+ num;
		return;
		}
	close(jobdes);
	if ((jobdes = creat(jobf,0644)) < 0)
		ciaou("can't reduce jobs' file");
	seek(jobdes,2,0); write(jobdes,base,4);
	ciaou("has completed");
	}

#define NIL 0

reset()
	{
	register int *p,*q,*r;

	update(0);
	for (p = first,q = item,r = next; q < r; *p++ = *q++);
	start = item = first;
	next = p;
	readtil(limit);
	}


update(coldstart)
	{

	scan(root);
	if (coldstart)
		{
		graph();
		if (write(position(1),summary,size) != size)
			abort("write error on summary file");
		seqno = 0;
		}
	root = NIL; stack = limit;
	}

main(a,v)
char *v[];
	{
	register int *p,*q,*r;

	for (init(); ; )
		if (item+1 > next)
			if (limit-next < 512)
				if (start >= next)
					reset();
				else
					if (start < (temp = first+(next-item)) || start-temp < 512)
						reset();
					else
						{
						for (p = first,q = item,r = next; q < r; *p++ = *q++);
						item = first; next = p; readtil(start);
						}
			else
				if (start < next)
					readtil(limit);
				else
					if (start-next < 512)
						reset();
					else
						readtil(start);
		else
			if (item->j_jobname[0] != 0)
				{ insert(); item++; }
			else
				{
				update();
				for (p = item,r = next; ; p = q)
					{
					if ((q = p+3) > r)
						{ item = p; reset(); goto oops; }
					if (*q != 0)
						break;
					}
				p++; base[0] = *p++; base[1] = *p++;
				incore();
				start = item = p;
			oops:
				continue;
				}
	}
