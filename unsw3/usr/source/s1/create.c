/*
 *	create - create specified files
 */
main( argc, argv ) char **argv;
{
	register i;
	register fd;

	if( argc<2 )
	{
		prints(2, "Usage: create file ...\n");
		exit(1);
	}

	for( i=1; i<argc; i++ )
		if( (fd = creat(argv[i], 0600)) < 0 )
		{
			prints(2, argv[i]);
			prints(2, ": Can't create\n");
		}
		else
		{
			close(fd);
		}
}
