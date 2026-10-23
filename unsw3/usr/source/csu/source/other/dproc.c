#

/*
 * Process table display
 */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"

struct proc	*procp;

int	fid;		/* memory */

struct modtab {
	char	*name;
	char	*offset;
	char	size;
	int	value;
	char	gotit;
	} modtab[] {
	{ "pri",	&0->p_nice,	1	},
	{ "rtl",	&0->p_rtl,	2	},
	{ "stl",	&0->p_stl,	2	},
	{ "ppid",	&0->p_ppid,	2	},
	0
	};

int	zappable;	/* things may be zapped */

main(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	register char	*cp;
	register struct proc	*pp;
	int	pri;

	if (argc <= 1) {
		printf("Usage: dproc [pri=n] [rtl=n] [stl=n] [ppid=n] pid ...\n");
		return(1);
	}

	gettable(0, proc, sizeof proc, &procp);
	fid = open("/dev/mem", 2);

	while (--argc > 0) {
		cp = *++argv;
		if (*cp>='a' && *cp<='z') {
			match(cp);
			continue;
		}
		for (i = 0; *cp>='0' && *cp<='9';)
			i = i*10 + *cp++ - '0';
		if (*cp != '\0') {
			printf("Nondigit: %c\n", *cp);
			continue;
		}
		pp = findproc(i);
		if (pp == NULL) {
			printf("No process: %u\n", i);
			continue;
		}
		if (zappable)
			zapproc(pp);
		else
			listproc(pp);
	}
	return(0);
}

findproc(n)
int	n;
{
	register struct proc	*pp;

	for (pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_pid == n)
			return(pp);
	return(NULL);
}

char	*procstates[] {
	"sleep (hi)",
	"sleep (lo)",
	"running",
	"exec",
	"zombie",
	"traced",
	};

char	*procflags[] {
	"in core",
	"fixed",
	"lock",
	"swapping",
	"traced",
	"another tracing flag?",
	};

listproc(pp)
register struct proc	*pp;
{
	register int	i;
	register char	*cp;
	char	pwbuf[100];

	printf("Address: %.1o\n", &procp[pp-proc]);
	printf("Process id: %u\n", pp->p_pid);
	printf("parent: %u\n", pp->p_ppid);
	printf("state: %s\n", procstates[pp->p_stat-1]);
	printf("flags:\n");
	for (i = 0; i < 6; i++)
		if (pp->p_flag & (1 << i))
			printf(" %s;", procflags[i]);
	printf("\n");
	if (pp->p_stat != SZOMB) {
		if (!getpw(pp->p_uid, pwbuf)) {
			for (cp = pwbuf; *cp != '\0'; cp++)
				if (*cp == ':') {
					*cp = '\0';
					break;
				}
			cp = pwbuf;
		} else
			cp = "?unknown";
		printf("user-id: %u  %s\n", pp->p_uid, cp);
	}
	printf("Priority: %d\n", pp->p_nice);
	if (pp->p_stat != SZOMB) {
		printf("p_pri: %u\n", pp->p_pri);
		printf("p_time: %u\n", pp->p_time);
	}
	if (pp->p_rtl != 0)
		printf("Real-time delay: %u secs\n", pp->p_rtl);
	if (pp->p_stl != 0)
		printf("Sleeping: %u secs\n", pp->p_stl);
	if (pp->p_stat != SZOMB) {
		printf("p_addr: %.1o\n", pp->p_addr);
		printf("p_size: %u  (%.1o)\n", pp->p_size, pp->p_size);
		printf("p_wchan: %.1o\n", pp->p_wchan);
		printf("p_textp: %.1o\n", pp->p_textp);
	}
}

/*
 * Zap possible entries in proc table?
 */
zapproc(pp)
struct proc	*pp;
{
	register char	*p;
	register struct modtab	*mp;

	p = &procp[pp - proc];
	for (mp = modtab; mp->name != 0; mp++)
		if (mp->gotit) {
			seek(fid, p + mp->offset, 0);
			write(fid, &mp->value, mp->size);
		}
}

/*
 * Find keyword entries
 */
match(acp)
char	*acp;
{
	register char	*p;
	register struct modtab	*mp;

	for (p = acp; *p != '\0'; p++)
		if (*p == '=') {
			*p++ = '\0';
			break;
		}

	for (mp = modtab; mp->name != 0; mp++)
		if (cmpstr(acp, mp->name)) {
			mp->gotit = 1;
			mp->value = atoi(p);
			zappable = 1;
			return;
		}
	printf("string not found: %s\n", acp);
}

cmpstr(s1, s2)
register char	*s1, *s2;
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return(1);
	return(0);
}
