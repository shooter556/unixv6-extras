	/*
	 *	ian j sep 77
	 *
	 *	/etc/warn [minutes to shutdown] [minutes between warnings]
	 *
	 *	allow super users to tell users and remind users
	 *	of iminent shutdown of unix
	 */

struct wtmp {
	char name[8];
	char tty;
	char dummy1;
	long time;
	int dummy2;
	} buf;

int mint 5;	/* assume five minutes between messages */
int mtogo 20;	/* assume twenty minutes to shutdown */
char term[] "/dev/tty?";

done() { }

main(argc,argv)
int argc;
char **argv;
{
	register mtty,ufd;
	extern fout;
	register char *ts;
	if( argc > 3 ) { printf("too many args\n"); exit(-1); }
	if( argc > 1 ) mtogo = number(argv[1]);
	if( argc > 2 ) mint  = number(argv[2]);
	if( mint >= mtogo ) { printf("minutes to go(%d) <= minutes twixt(%d)\n",mtogo,mint); exit(-1); }
	printf("\n %l minutes to system shut down\n",mtogo);
	printf(" %l minutes twixt user warnings\n",mint);
	clktim(mtogo*60);
	signal(14, done);

	if( fork() ) exit(0);
	mtty = ttyn(0);	/* note my tty id */
	ts = "minutes";
	for(;;) {
		ufd = open("/etc/utmp",0);
		while( read(ufd,&buf,sizeof buf)==sizeof buf)
			if( (buf.name[0]) && (buf.tty==mtty) ) {
				term[8] = buf.tty;
				if( fout=open(term,1) > 0 ){
					printf("\007\007warning\n");
					printf("\007\007warning\n");
					printf("system going down in %d %s\n",mtogo,ts);
					printf("\007\007warning\n");
					printf("\007\007warning\n");
					flush();
					close(fout);
				}
			}
		sleep(mint*60);
		if( (mtogo =- mint) <= 0 ) exit();
	}
}

number(s)
register char *s;
{
	register  n = 0;

	while( (*s>='0')&&(*s<='9') )  n = n*10 + *s++ - '0';
	return(n);
}
