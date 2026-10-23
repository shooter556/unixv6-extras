/*
 * cp oldfile newfile
 *
 *	if a first argument of '-' is given (i.e. cp - file1 file2)
 *	then null blocks will be skipped and not allocated.
 */

int	skip	0;

main(argc,argv)
char **argv;
{
	static int buf[256];
	int fold, fnew, n;
	register char *p1, *p2, *bp;
	int mode;

	if (argc>3 && argv[1][0]=='-') {
		skip++;
		argc--;
		argv++;
	}
	if(argc != 3) {
		write(2, "Usage: cp oldfile newfile\n", 26);
		exit(1);
	}
	if((fold = open(argv[1], 0)) < 0) {
		write(2, "Cannot open old file.\n", 22);
		exit(1);
	}
	fstat(fold, buf);
	mode = buf[2];
	/* is target a directory? */
	if (stat(argv[2], buf+50)>=0 && (buf[52]&060000)==040000) {
		p1 = argv[1];
		p2 = argv[2];
		bp = buf+100;
		while(*bp++ = *p2++);
		bp[-1] = '/';
		p2 = bp;
		while(*bp = *p1++)
			if(*bp++ == '/')
				bp = p2;
		argv[2] = buf+100;
	}
	if (stat(argv[2], buf+50) >= 0) {
		if (buf[0]==buf[50] && buf[1]==buf[51]) {
			write(2, "Copying file to itself.\n", 24);
			exit(1);
		}
	}
	if ((fnew = creat(argv[2], mode)) < 0) {
		write(2, "Can't create new file.\n", 23);
		exit(1);
	}
	while(n = read(fold,  buf,  512)) {
	if(n < 0) {
		write(2, "Read error\n", 11);
		exit(1);
	} else
		if (skip) {
			for (p1 = buf; p1 < &buf[256];)
				if (*p1++ != 0)
					goto out;
			seek(fnew, n, 1);
			continue;
		}
	out:
		if(write(fnew, buf, n) != n){
			write(2, "Write error.\n", 13);
			exit(1);
		}
	}
	exit(0);
}
