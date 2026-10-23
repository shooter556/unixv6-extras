/*	Disk patch program	*/
/*		zapdisk file block offset oldword new-word	*/
/*	it's only for the real dog-days	*/
/*		Gerry Barksdale (Feb 1976)	*/
int fp, blkno, off, oldval, newval;
int saveword;
main(argc,argv)
char **argv;
{
	if(argc != 6)
		{printf("argc=%d\n",argc);exit(1);}

	if((fp=open(argv[1],2))<0)
		{printf("%s no good\n",argv[1]);exit(1);}
	blkno=geto(argv[2]);
	off=geto(argv[3]);
	oldval=geto(argv[4]);
	newval=geto(argv[5]);
printf("%s %o %o %o %o\n",argv[1],blkno,off,oldval,newval);
	seek(fp,blkno,3);	/*get block	*/
	seek(fp,off,1);	/*offset	*/
	read(fp,&saveword,2);
	if(saveword!=oldval) {
		printf("wrong word: %o\n",saveword);
		exit(1);
	}
	seek(fp,-2,1);
	write(fp,&newval,2);
}
geto(nn)
char **nn;
{
int j, k;
char *n;
	n = nn;
	j=k=0;
	while(k = *(n)++)
		j = j*8 +k-'0';
	return(j);
}
