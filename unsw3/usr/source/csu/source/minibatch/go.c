/*
 *	go - start things rolling
 */

char	*procs[]
{
	"ear",
	"crd",
	"spl",
	0
};

char	*lock	"/tmp/batchlock";

main()
{
	register	*p;
	register	fid;
	extern	fout;
	int	pid;
	char	buf[36];

	signal(1, 1); signal(2, 1); signal(3, 1);

	fout = 2;

	if ( stat(lock, buf) == 0 )
		error("Already active\n");

	if ( (fid = creat(lock, 0600)) < 0 )
		error("Can't create lock\n");

	p = procs;

	do
		if ( !(pid = fork()) )
		{
			execl(*p, *p, 0);
			printf(*p);
			error("Can't exec\n");
		}
		else
		write(fid, &pid, 2);

	while (*++p);

	exit(0);
}

error(s)
char	*s;
{
	printf("Go: %s", s);
	flush();
	exit(1);
}
