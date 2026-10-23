#
/* modded by daveh feb 7:
 *	if '-' flag given, then 'cp' invoked with same flag.
 *	this means that zero blocks, which would normally have
 *	been allocated, get skipped after all.
 */
#ifndef	unsw_orig
int	skip 0;
#endif
char	buf[100];

main(argc, argv)
char **argv;
{
	register i;
	register char *c1, *c2;

#ifndef	unsw_orig
	if( argc > 3 && argv[1][0] == '-' ){
		skip++;
		argc--;
		argv++;
	}
#endif
	if(argc < 3) {
		write(2, "Usage: cpall file ... directory\n", 32);
		exit();
	}
	argc--;
	c1 = buf;
	c2 = argv[argc];
	while(*c1++ = *c2++);
	c1[-1] = '/';
	*c1++ = '.';
	*c1 = '\0';
	for(i=1; i<argc; i++) {
		if(fork()==0) {
#ifndef	unsw_orig
			if( skip )
				execl("/bin/cp", "cp", "-", argv[i], buf);
			else
#endif
			execl("/bin/cp", "cp", argv[i], buf);
			write(2, "Cp not found\n", 13);
			exit();
		}
		wait();
	}
}
