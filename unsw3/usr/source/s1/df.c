#
#define	modij1
	/*
	 *	ianj sep '77
	 *
	 *	if no args given will use current disk as default
	 *	file system.
	 */

#ifndef modij1
char	*dargv[]
{
	"",
	0
};
#endif modij1
#ifdef modij1
struct {
	int	ino;
	char	name[14];
	int 	stbuf[18];
	};
#endif modij1

struct
{
	char	*s_isize;
	char	*s_fsize;
	int	s_nfree;
	int	s_free[100];
	int	s_ninode;
	int	s_inode[100];
	char	s_flock;
	char	s_ilock;
	char	s_fmod;
	int	time[2];
	int	pad[50];
} sblock;

int	fi;

main(argc, argv)
char **argv;
{
	int i;

	if(argc <= 1) {
#ifndef modij1
		for(argc = 1; dargv[argc]; argc++);
		argv = dargv;
	}
#endif modij1
#ifdef modij1
		register	j;
		stat(".", sblock.stbuf);
		i = sblock.stbuf[0];
		chdir("/dev");
		j = open(".", 0);
		while( read(j,&sblock,16) > 0)
		{
			if(sblock.ino == 0) continue;
			stat(sblock.name,sblock.stbuf);
			if(( (sblock.stbuf[2] & 060000) == 060000) &&
				(sblock.stbuf[6] == i) )
				{
					close(j);
					printf("/dev/%s ", sblock.name);
					dfree(sblock.name);
					exit();
				}
		}
		printf("cant find dev\n");
	} else
#endif modij1
	       for (i=1; i<argc; i++) {
			if(argc > 1)
				printf("%s ", argv[i]);
			dfree(argv[i]);
	}
}

dfree(file)
char *file;
{
	int i;

	fi = open(file, 0);
	if(fi < 0) {
		printf("cannot open %s\n", file);
		return;
	}
	sync();
	bread(1, &sblock);
	i = 0;
	while(alloc())
		i++;
	printf("%l\n", i);
	close(fi);
}

alloc()
{
	int b, i, buf[256];

	i = --sblock.s_nfree;
	if(i<0 || i>=100) {
		printf("bad free count\n");
		return(0);
	}
	b = sblock.s_free[i];
	if(b == 0)
		return(0);
	if(b<sblock.s_isize+2 || b>=sblock.s_fsize) {
		printf("bad free block (%l)\n", b);
		return(0);
	}
	if(sblock.s_nfree <= 0) {
		bread(b, buf);
		sblock.s_nfree = buf[0];
		for(i=0; i<100; i++)
			sblock.s_free[i] = buf[i+1];
	}
	return(b);
}

bread(bno, buf)
{
	int n;
	extern errno;

	seek(fi, bno, 3);
	if((n=read(fi, buf, 512)) != 512) {
		printf("read error %d\n", bno);
		printf("count = %d; errno = %d\n", n, errno);
		exit();
	}
}
