#

/*
 * Inode table display program
 */

#include "/usr/sys/param.h"
#include "/usr/sys/inode.h"

struct inode	*inodep;

main(argc, argv)
int	argc;
char	*argv[];
{
	register int	i;
	register char	*cp;
	int	dev;
	int	stbuf[18];

	if ((argc =- 2) < 0) {
		printf("Usage: dinode device [i-number] ...\n");
		return(1);
	}

	gettable(2, inode, sizeof inode, &inodep);

	cp = *++argv;
	if (stat(cp, stbuf) < 0) {
		for (i = 0; *cp>='0' && *cp<='9';)
			i = i*10 + *cp++ - '0';
		if (*cp != '\0') {
			printf("Illegal device: %s\n", *argv);
			return(1);
		}
		dev = i;
	} else
		switch (stbuf[2] & IFMT) {

		case 0:		/* file */
		case IFDIR:	/* directory */
			dev = stbuf[0];
			break;

		case IFCHR:	/* character special */
			printf("Can't be a character special file\n");
			return(1);

		case IFBLK:	/* block special */
			dev = stbuf[6];
			break;
		}

	if (argc == 0)
		findinos(dev);
	else
		do {
			cp = *++argv;
			for (i = 0; *cp>='0' && *cp<='9';)
				i = i*10 + *cp++ - '0';
			if (*cp != '\0') {
				printf("Non-numeric: %c\n", *cp);
				continue;
			}
			listino(dev, i);
		} while (--argc > 0);
	return(0);
}

findinos(dev)
int	dev;
{
	register struct inode	*ip;

	for (ip = &inode[0]; ip < &inode[NINODE]; ip++) {
		if (ip->i_number == 0)
			continue;
		if (ip->i_dev == dev)
			printf("ino: %u\n", ip->i_number);
	}
}

char	*iflags[] {
	"locked",
	"updated",
	"access time modified",
	"mounted on",
	"wanted",
	"pure text",
	};

listino(dev, ino)
int	dev;
int	ino;
{
	register struct inode	*ip;
	register int	i;
	register char	*cp;
	char	pwbuf[100];

	for (ip = &inode[0]; ip < &inode[NINODE]; ip++)
		if (ip->i_dev==dev && ip->i_number==ino)
			goto gotit;
	printf("No incore i-node: %u\n", ino);
	return;

gotit:
	printf("I-node address: %o\n", &inodep[ip-inode]);
	printf("i_number: %u\n", ip->i_number);
	printf("i_dev: %.1o\n", ip->i_dev);
	printf("i_flag: %.1o\n", ip->i_flag);
	for (i = 0; i < 6; i++)
		if (ip->i_flag & (1 << i))
			printf(" %s;", iflags[i]);
	printf("\n");
	printf("i_count: %u\n", ip->i_count & 0377);
	printf("i_mode: %.1o\n", ip->i_mode);
	printf("i_nlink: %u\n", ip->i_nlink & 0377);
	if (!getpw(ip->i_uid & 0377, pwbuf)) {
		for (cp = pwbuf; *cp != '\0'; cp++)
			if (*cp == ':') {
				*cp = '\0';
				break;
			}
		cp = pwbuf;
	} else
		cp = "?unknown";
	printf("i_uid: %d %s\n", ip->i_uid&0377, cp);
	printf("i_size: %D (%O)\n", ip->i_size0&0377, ip->i_size1, ip->i_size0&0377, ip->i_size1);
	printf("i_addr:\n");
	for (i = 0; i < 8; i++)
		printf("    %6u (%6o)\n", ip->i_addr[i], ip->i_addr[i]);
}
