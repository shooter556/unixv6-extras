/*
	size -- determine object size

*/

main(argc, argv)
char **argv;
{
#ifndef unsw_orig
	int buf[010], f, ac;	long sum;
	struct { long text; };
	struct { long data; };
	struct { long bss; };
#endif
#ifdef unsw_orig
	int buf[010], f, ac, sum;
#endif

	ac = argc;
	if (ac==1) {
		*argv = "a.out";
		ac++;
		--argv;
	}
	while(--ac) {
		++argv;
		if((f=open(*argv, 0))<0) {
			printf("%s not found\n", *argv);
			continue;
		}
		read(f, buf, 0020);
		if(buf[0]!=0411 && buf[0]!=0410 && buf[0]!=0407) {
			printf("Bad format: %s\n", *argv);
			close(f);
			continue;
		}
#ifndef unsw_orig
		printf("%s: ", *argv);
		printf("%u+%u+%u=", buf[1],buf[2],buf[3]);
		buf[0] = 0; sum =  (&buf[0])->text;
		buf[1] = 0; sum =+ (&buf[1])->data;
		buf[2] = 0; sum =+ (&buf[2])->bss;
		printf("%ld (0%lo)\n", sum, sum);
#endif
#ifdef unsw_orig
		if (argc>2)
			printf("%s: ", *argv);
		printf("%l+%l+%l=", buf[1],buf[2],buf[3]);
		sum = buf[1]+buf[2]+buf[3];
		printf("%l (%o)\n", sum, sum);
#endif
		close(f);
	}
}
