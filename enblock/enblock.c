#include <errno.h>
#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
	char *malloc();
	int c;
	int bsize;
	char *buf;

	switch (argc) {
	case 1:
		bsize = 512;
		break;
	case 2:
		bsize = atoi(argv[1]);
	        break;
	default:
		bsize = 0;
	}
	if (bsize == 0) {
		printf("usage: enblock [blocksize]\n");
		exit(1);
	}
	if((buf = malloc(bsize)) == 0) {
		printf("error: malloc: %d", errno);
		exit(1);
	}
	while ((c = read(0, buf, bsize)) >0) {
		putint(bsize);
		write(1, buf, bsize);
		putint(bsize);
	}
	putint(0);
}

putint(n)
int n;
{
	char obuf[4];

	obuf[0] = n % 256;
	n /= 256;
	obuf[1] = n % 256;
	n /= 256;
	obuf[2] = n % 256;
	n /= 256;
	obuf[3] = n % 256;
	write(1, obuf, 4);
}
