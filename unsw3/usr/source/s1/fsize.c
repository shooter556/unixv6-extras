#
/*
	fsize file [-]

	command to exit 0(true) if file exists and size>0
			1(false) otherwise
				(if '-' then delete if size=0)

*/


main(argc,argv)
	int argc;  char *argv[];
{
	char buf[36];  int del;

	del=0;
	if(argc==3 && argv[2]!=0 && argv[2][0]=='-') {
		argc--;
		del++;
	}
	if(argc!=2 || argv[1]==0 || stat(argv[1],buf)== -1) exit(1);
	if((buf[11]|buf[10]|buf[9]) != 0) exit(0);

	if(del) unlink(argv[1]);
	exit(1);
}
