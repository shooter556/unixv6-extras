#
/*
 *	Implements chdir which full wild-character argument:
 *	sh passes 'funny' characters to glob, which then
 *	executes chdir
 */
#define	BDIR	"chdir: bad directory\n"
#define	BSH	"sh: cannot execute\n"
main(argc, argv)
char **argv;
{
	if(argc != 2) {
		write(2, BDIR, sizeof BDIR);
		goto sexec;
	}

	if(chdir(argv[1]) < 0) {
		write(2, BDIR, sizeof BDIR);
	}

sexec:
	execl("/bin/sh", "-", 0);
	write(2, BSH, sizeof BSH);
	exit(1);
}
