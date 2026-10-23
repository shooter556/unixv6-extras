/* this program tests the magnetic tape units by writting a full	*/
/*	  block of each byte pattern from 000 to 377 for 9 track units	*/
/*	  and from 000 to 077 for 7 track units.  it then reads back	*/
/*	  each block and checks the pattern.	*/

/*		bill allen  march,1975	*/

/* call with the tape unit number to be checked.	*/


char tfile1[] {"/dev/mt0"};
char tfile[10];
char buffer[512];
int  tfd;


main(argc,argv)
char  **argv;
{

	register t1;
	int i,j,k;
	int rubout();

	signal(2,rubout);	/*catch rubout	*/
	for(i=0;i<9;i++)
		tfile[i] = tfile1[i];
	if(argc<=1){
		printf("specify tape unit number\n");
		exit();
	}

	t1 = *argv[1]&0177;		/*get unit number	*/
	if(t1<'0' || t1>'7' || t1 =='3'){
		printf("invalid tape unit number\n");
		exit();
	}

	tfile[7] = t1;		/*set unit number	*/
	tfd = open(tfile,1);	/*writting	*/
	if(tfd<0){
		printf("unable to open %s\n",tfile);
		exit();
	}

	j=0177;		/*end test pattern	*/
	if(t1=='2' || t1>'5')		/*7 track unit	*/
		j=077;

	for(i=0;i<=j;i++){
        for(k=0;k<512;k++)
			buffer[k] = i;
		if(write(tfd,buffer,512) != 512){
			printf("write error block %o\n",i);
			rubout();
		}
	}

	close(tfd);		/*write end of file	*/
	tfd = open(tfile,0);	/*read	*/
	if(tfd<0){
		printf("unable to reopen %s\n",tfile);
		exit();
	}

	for(i=0;i<=j;i++){
        if((k=read(tfd,buffer,512)) != 512){
            printf("read bad byte count %d block %o\n",k,i);
			rubout();
		}
		for(k=0;k<512;k++){
			if(buffer[k] != i)
				printf("read error block %o bad %o\n",i,buffer[k]);
		}
	}		/*end for	*/

	if(read(tfd,buffer,512) != 512){
		printf("end of test\n");
		rubout();
	}
	printf("end of file not sensed\n");
	rubout();
}

/*got a rubout or ready to exti	*/

rubout()
{
	close(tfd);
	exit();
}
