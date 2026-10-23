#
/*
 *	Remove symbol table and relocation bits from run files
 */

char	tmpfil[]	"/tmp/stmaXXXXX";

int	buf[256];

main(argc, argv)
char **argv;
{
	if(argc == 1)
		strip("a.out");
	else
		while(--argc > 0)
			strip(*++argv);
}

strip(s)
char *s;
{
	register fi, fo, c;
	long len;

	if((fi = open(s, 0)) < 0) {
		printf("Open error: %s\n", s);
		return;
	}
	fo = creat(mktemp(tmpfil),0666);
	read(fi, buf, 512);
	if(*buf != 0403 && *buf != 0405 && *buf != 0407 && *buf != 0410 && *buf != 0411) {
		printf("Improper format: %s\n", s);
		goto out;
	}
	len = 16;
	if(*buf != 0403) {
		dpadd(&len, buf[1]);
		dpadd(&len, buf[2]);
		buf[4] = 0;
		buf[7] =| 1;
	} else
		dpadd(&len, buf[6]);

	write(fo, buf, itn(len));
	len =- 512;

	while(len > 0) {
		read(fi, buf, 512);
		write(fo, buf, itn(len));
		len =- 512;
	}

	close(fi);
	close(fo);

	if((fo = creat(s, 0)) < 0) {
		printf("Cannot rewrite %s\n", s);
		goto out;
	}
	if((fi = open(tmpfil, 0)) < 0) {
		printf("Cannot reread temp for %s\n", s);
		goto out;
	}
	while((c = read(fi, buf, 512)) > 0) 
		write(fo, buf, c);
out:
	close(fi);
	close(fo);
	chmod(tmpfil, 0777);
	unlink(tmpfil);
}

itn(l)
long l;
{
	if(l > 512)
		return(512);
	return(l);
}
