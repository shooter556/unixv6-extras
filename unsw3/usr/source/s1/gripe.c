/* copyright 1975 michael d. tilson */

/* not that anyone will want to steal it */

main(argc,argv)
int argc;
char *argv[];
{
char *c;
 long time();
	if (argc>2)
	{
		printf("bad gripe arg\n");
		exit();
	}
	c=ctime(time());
	c[16]='\0';
	if (argc==1 || (argv[1][0]=='s' && argv[1][1]=='\0'))
	{
		execl("/bin/mail","/bin/mail","gripe",0);
	}
	if (argv[1][0]=='p' && argv[1][1]=='\0')
	{
		printf("%s   ***  gripes  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/usr/gripe/.mail",0);
	}
	if (argv[1][0]=='o' && argv[1][1]=='\0')
	{
		printf("%s   ***  old gripes  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/usr/gripe/mbox",0);
	}
	if (argv[1][0]=='r' && argv[1][1]=='\0')
	{
		printf("%s   ***  gripe replys  ***\n\n\n",c);
		execl("/bin/cat","/bin/cat","/usr/gripe/replys",0);
	}

	printf("bad gripe arg\n");
	exit();
}
