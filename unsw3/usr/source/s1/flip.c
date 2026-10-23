#
/*
	FLIP - take a list of player names and print
		them out in random order
*/
#define NPLAYERS 60
char list[NPLAYERS];
main(argc,argv)
	int argc;
	char **argv;
{
	register i;
	register j;
	extern long time();
	long buf;
	struct { int hi,lo; };

	buf = time();
	srand(buf.lo);
	if (--argc>NPLAYERS) {
		printf("Too many players\n");
		exit();
	}
	for (i=0; i<argc; i++){
		j=rand()%argc;
		if (list[j]) {
			i--;
			continue;
		}
		list[j]=1;
		printf("%s\n",argv[j+1]);
	}
}
