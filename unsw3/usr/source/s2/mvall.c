char	buf[100];
main(argc, argv)
char **argv;
{
	register i;
	register char *c1, *c2;
	if(argc < 3) {
		write(2, "Usage: mvall file ... directory\n", 32);
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
			execl("/bin/mv", "mv", argv[i], buf);
			write(2, "Mv not found\n", 13);
			exit();
		}
		wait();
	}
}
