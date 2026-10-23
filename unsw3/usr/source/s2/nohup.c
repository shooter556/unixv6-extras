main(argc, argv)
register int   argc;
register char *argv[];
{
	if(argc < 2)
	{
		prints(2, "Arg count\n");
		exit();
	}
	argc--;
	argv++;
	argv[argc] = 0;
	signal( 1, 1);
	signal( 2, 1);
	signal( 3, 1);
	execc( argv[0], argv);
	prints(2, argv[0]);
	prints(2, ": Not found\n");
}
