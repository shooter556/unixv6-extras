long time();
char	date[] "/bin/date";
char	m1[] "if ACCEPTABLE just <cr>\notherwise ENTER date in the form\n[[[[[yy]mm]dd]hh]mm]\n";
char	m2[] "date non-executable\n";
char	m3[] "\n\n\n\n\n\t\t\t";
clock()	{	}
main()
{
	char buf[2100]; int register n;
	signal(15,clock);
	clktim(60);	/* give them 60 seconds */
	write(1,m3,sizeof m3);
	write(1,ctime(time()),27);
	write(1,m1,sizeof m1);
	if( (n=read(0,buf,sizeof buf))<=1 ) {
		execl(&date[0],&date[5],0);
	} else {
		buf[10]=buf[n]=0;
		execl(&date[0],&date[5],buf,0);
	}
	write(1,m2,sizeof m2);
}
