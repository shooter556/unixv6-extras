#
	int buff[256]{ 0,0,0,0,0,0,0,0, 0104401 };
#define relflag buff[7]
#define txtsize buff[1]
main(c,v)
char *v[];
{
	int register n;

			  /*
			   * add a sys exit as dummy text 
			   */
	if( c != 3 )
	{
		write(2, "Usage: transfer in out\n", 23);
		exit(1);
	}
	close(0); close(1);
	if( open(v[1], 0) != 0 )
	{
		write(2, "Cannot open input\n", 18);
		exit(1);
	}
	if( creat(v[2], 0700) != 1)
	{
		write(2, "Cannot create output\n", 21);
		exit(1);
	}
	read(0,buff,020); write(1,buff,022);
	if(!relflag) {write(2,"relocation ???\n",15);exit();}
	seek(0,txtsize,1);
	seek(1,txtsize-2,1);	/* dont really need the text */
	while((n=read(0,buff,512))>0) write(1,buff,n);
}
