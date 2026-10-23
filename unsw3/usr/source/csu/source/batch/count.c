/*
 * count - count cyber line errors
 *
 *					daveh.  march '78
 */
struct
{
	char	name[8];
	int	type,
		value;
} nl[]
{
	"_ycering", 0, 0,
	"_ycecarr", 0, 0,
	"_yceover", 0, 0,
	"_ycerrfc", 0, 0,
	"_lpages", 0, 0,
	"\0\0\0\0\0\0\0\0", 0, 0
};
main(argc, argv)
char	**argv;
{
	extern	fout;
	register	fd;
	register	i;

	int	ering, ecarrier, eoverrun, errfc[6];
	int	lpages;
	long	cards;

	fout = dup(1);
	nlist( argc>1 ? argv[1] : "/unix", nl );
	for ( i = 0; i <= 3; i++ )
		if ( nl[i].type <= 0 )
		{
			prints(fout, "Count: Nlist failed\n");
			exit(1);
		}
	if ( (fd = open("/dev/kmem", 0)) < 0 )
	{
		prints(fout, "Count: Can't open memory\n");
		exit(1);
	}

	seek(fd, nl[0].value, 0);	read(fd, &ering, 2);
	seek(fd, nl[1].value, 0);	read(fd, &ecarrier, 2);
	seek(fd, nl[2].value, 0);	read(fd, &eoverrun, 2);
	seek(fd, nl[3].value, 0);	read(fd, errfc, 12);
	seek(fd, nl[4].value, 0);	read(fd, &lpages, 2);

	printf("Error count on %s\n\nring%10t%6l\ncarrier%10t%6l\noverrun%10t%6l\n",
		ijtime(), ering, ecarrier, eoverrun);
	printf("format%10t%6l\nlong. par%10t%6l\nreceive%10t%6l\ntransmit%10t%6l\nlength%10t%6l\nprotocol%10t%6l\n",
		errfc[0], errfc[1], errfc[2], errfc[3], errfc[4], errfc[5]);
	printf("\nPages%10t%6l\n", lpages);
	flush();
	exit(0);
}
