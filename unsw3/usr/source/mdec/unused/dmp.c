buff[24*512];
main(){
	int register i,j,k;
	char file[20];

	close(1);
	if(open("/dev/rmt0",1)<0) exit();
  for(;;){
	write(2,"\n\n\n next file system ?? ",24);
	if( (i=read(0,file,20))<=1 ) exit();
	if(i>15) exit();
	file[i-1]=0;
	if( (k=open(file,0))<0 ) {
		write(2,"file open error!!\n",18);
		continue;
	}
	for(i=0;i<203;i++) write(1,buff,read(k,buff,24*512));
	close(k);
  }
}
