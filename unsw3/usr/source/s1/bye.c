	/* sign-off */
char	m1[] "you are not alone ??\n";
char	m2[] "all right ... be impatient ... see if i care!\n";
main()
{
	int register nuser,f;	char buf[16];
	extern	wake();
	signal(2,1);
	chmod("/",0755);
	sync();
	if((f=open("/etc/utmp",nuser=0))>0) {
		while( read(f,buf,16)==16 ) if(buf[0]) nuser++;
		if(nuser>1) { write(1,m1,sizeof m1); exit(); }
	}
	signal(2,wake);
	sleep(30);
	signal(2,1);
	chmod("/",0755);
	sync();
	write(1,"bye bye\n",9);
}

wake()
{
	write(2,m2,sizeof m2);
}
