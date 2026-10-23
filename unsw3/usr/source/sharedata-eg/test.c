main()
{
	register n1,n2;

	if( !(n1=fork()) )
		execl("sd1","sd1",0);
	if( !(n2=fork()) )
		execl("sd2","sd2",0);
	wait();
	kill(n1,9);
	kill(n2,9);
	wait();
	printf("\nsd1 & sd2 now dead\n\n");
}
