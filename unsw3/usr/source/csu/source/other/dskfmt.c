#

/*
 *	dskfmt - format disks online ...
 *
 *	i assume the disk driver has been taught all about it ...
 *
 *	usage -	dskfmt special ...
 *
 *				daveh, inc
 *
 *	Modified:	Craig McGregor
 *			Computing Services Unit
 *			UNSW
 *			78/02/20
 */

#define	CYLSIZ	(24*512)

int	zeros[CYLSIZ/2];

int	fmton[]		{ 2, 0, 0, 0 };
int	fmtoff[]	{ 0, 0, 0, 0 };

main(argc, argv)
int	argc;
char	*argv[];
{
	register	i, j, fd;

	if (argc < 2) {
		printf("Usage: /etc/dskfmt special ...\n");
		return(1);
	}

	for (i = 1; i < argc; i++) {
		if ((fd = open(argv[i], 1)) < 0) {
			printf("%s: cannot open\n", argv[i]);
			continue;
		}

		printf("%s:\nAre you sure? ", argv[i]);

		j = getchar();
		if (j != '\n')
			while (getchar() != '\n');
		if (j != 'y') {
			close(fd);
			continue;
		}

		if (stty(fd, fmton) < 0) {
			printf("%s: stty failed\n", argv[i]);
			close(fd);
			continue;
		}

		printf("%s: formatting ...\n", argv[i]);

		for (j = 0; write(fd, zeros, CYLSIZ)==CYLSIZ; j++);
		stty(fd, fmtoff);
		printf("%s: %u cylinders formatted !!!\n", argv[i], j);
		close(fd);
	}
	return(0);
}

putchar(c)
char	c;
{
	write(1, &c, 1);
}

getchar()
{
	char	c;

	if (read(0, &c, 1) != 1)
		return('\n');
	return(c);
}
