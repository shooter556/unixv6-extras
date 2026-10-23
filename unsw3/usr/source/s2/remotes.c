#define	ARGERR	"remotes: which remote?\n"
#define	OPENERR	"remotes: cant open a remote!\n"
#define	RAWNOECHO	0340
#define BAUDS2400	05413
#define BAUDS4800	06014
#define BAUDS9600	06415

main(argc,argv)
char	*argv[];
{
	int	fd[16];
	register	i;
	int	ttymode[3];
	int	resco;

	if (argc == 1) error(ARGERR);

	resco = 0;
	if ( fork () ) exit();

	signal(2,1);
	signal(3,1);
	ttymode[0] = BAUDS2400;
	ttymode[1] = 0;
	ttymode[2] = RAWNOECHO;

	for (i = 1; (i < 16) && (i < argc); i++)
	{
		if (( fd[i-1] = open (argv[i], 2)) < 0) error(OPENERR);
		stty(fd[i-1], ttymode);
	}
	close(0); close(1); close(2);
	for(;;)
		pp(&resco);
}
error(string)
char	*string;
{
	register char	*p;

	for(p = string; *p != '\0'; p++);

	write(2,string,p-string);

	exit();
}
