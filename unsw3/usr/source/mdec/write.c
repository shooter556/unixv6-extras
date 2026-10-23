buff[24*256];
main(){
	int register i,j,k;

	j=creat("/dev/rmt1",0666);

			i=open("/dev/rhp7",0);
	for(k=0;k<1218;k++) { read(i,buff,24*512); write(j,buff,24*512); }
	close(i);	i=open("/dev/rhp1",0);
	for(k=0;k<609;k++) { read(i,buff,24*512); write(j,buff,24*512); }
	close(i);	i=open("/dev/rhp6",0);
	for(k=0;k<203;k++) { read(i,buff,24*512); write(j,buff,24*512); }

}
