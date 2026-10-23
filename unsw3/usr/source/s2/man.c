#
/*
 *		copyright 1975 ace systems inc.
 */
	char stra[800];	char **args[100];
char dirnm[] "/srce/usr/docmention/man/man1";
#define zz (sizeof dirnm - 2)
main(argc,argv)
int argc; char **argv;
{       
	int register i,o; char register *s;
	if(argc<2) exit();
	args[0] = "nroff";
	args[1] = "-manual";

	i=1;	o=2;	s = stra;
	if( !argv[i][1] ) switch(argv[i][0]) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':	dirnm[zz] = argv[i][0];
			i=i+1;
    }
	if(chdir(dirnm)) {
		printf("man:  chdir to %s failed\n",dirnm);
		exit();
	}
	while(argc>i) {
		 args[o] = s;
		while(*s++ = *argv[i]++);
		*--s = '.';
		*++s = dirnm[zz];
		*++s = 0;
		s++;
		i++;
		o++;
	}
	args[o] = 0;
	execv("/usr/bin/nroff",args);
	printf("exec failed\n");
}
