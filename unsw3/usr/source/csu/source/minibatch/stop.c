/*
 *	stop - terminate things
 */

char	*lock	"/tmp/batchlock";

main()
{
	extern	fout;
	int	pid;
	register	fid;

	fout = 2;

	if ( (fid = open(lock,0)) < 0 )
		error("Not active\n");

	while ( read(fid, &pid, 2) == 2 )
		if ( kill(pid, 9) < 0 )
			printf("%d: can't kill\n", pid);

	close(fid);
	if ( unlink(lock) < 0 )
		printf("Can't remove lock\n");
	flush();
	exit(0);
}

error(s)
char	*s;
{
	printf("Stop: %s", s);
	flush();
	exit(1);
}
