#
/*
 * program to compile messages from standard input
 *  and send it to the standard output.
 * stored in message.c  compiled in kwtc2.
 */
#define alfafile "/etc/letters"
#define flag 010000
main() {
	register	x,y,code;
	int	alfdes,chd,n,p,buf[12];
	char messg[60][133];
/*
 * to insert newline character at the end of the line
 */
	for(x =0;x <= 59; x++) messg[x][132]= '\n' ;
	if((alfdes =open(alfafile,0)) < 0 )
	{
		write(2,"cannot find character set \n",27);
		exit(1);
	}
/*
 * message assembling
 */
	write(2,"<NEW  PAGE>\n",12);
	for(p=0; p<5; p++)
	{
		n = 0;
	nextc:	if( !read(0,&chd,1))
		{
			chd = 0;
			break;
		}
		if(n++>10 || chd =='\n')
		{
			while( chd != '\n' ) if(! read(0,&chd,1)) break;
			continue;
		}
		chd =- 040 ;
		seek(alfdes,chd*24,0);
		read(alfdes,buf,24);
		for(y=0;y<=11;y++)
		{
			code = buf[y];
			for(x=0;x<=11;x++)
			messg[12*p+y][12*(n-1)+x] = ((code=<<1) & flag) ? '*' : ' ';
		}
		goto nextc;
	}
/*
 * print message
 */
	write(1,messg,60*133);
	close(alfdes);
	if( chd ) execl("/usr/bin/message","message",0);
}
