#
	int buff[256]{ 0,0,0,0,0,0,0,0, 0104401 };
#define relflag buff[7]
#define txtsize buff[1]
main() {
	int register n;

			  /*
			   * add a sys exit as dummy text 
			   */
	read(0,buff,020); write(1,buff,022);
	if(!relflag) {write(2,"relocation ???\n",15);exit();}
	seek(0,txtsize,1);
	seek(1,txtsize-2,1);	/* dont really need the text */
	while((n=read(0,buff,512))>0) write(1,buff,n);
}
