main()
{
	register n1,n2,n3;

	if( !(n1=fork()) )
		execl("sd1","sd1",0);
	if( !(n2=fork()) )
		execl("sd2","sd2",0);
	if( !(n3=fork()) )
		execl("sd2","sd2",0);
	wait();
	kill(n1,9);
	kill(n2,9);
	kill(n3,9);
	wait();
	printf("\nsd1 & both sd2s now dead\n\n");
}
