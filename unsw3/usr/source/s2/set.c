#define	B2400	11
main(argc,argv)
char **argv;
int argc;
{
	struct {
		char ispeed,ospeed;
		char erase,kill;
		int mode;
	} arg;


	gtty( 1 , &arg );	/* get terminal modes */
	if( arg.ispeed >= B2400 )  {
		arg.erase = 010; /* backspace */
		stty( 1 , &arg );	/* set terminal modes */
	}
	if( argv[0][0] == '-') execl( "/bin/sh" , "-" , 0 );
}
