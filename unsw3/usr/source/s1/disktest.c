#
/* this program is a generalized diagnostic for any random access */
/*	or sequential device running under unix. */
/*		argument is device name */

/*		Bill Allen */
/*		Naval Postgraduate School */
/*		October, 1975 */



int buffer[256];

int abort;
int stim[2],etim[2],tranf;
int upat;	/*user specified pattern */
int	bn;		/*current block number */
int	r;		/*current test number */
int  tfd;
#define NUMPAT 10
int pat[NUMPAT]
	{ 0,-1,0125252,0052525,0377,0177400,070707,0107070,0100000,1};

char line[80];	/*terminal input buffer */
char *lp;

int	lowbn	0;		/*low address to test */
int	highbn	1024;	/*high address to test */
int repeat	1;		/*test repetition factor */
int	patf	0;		/*pattern flag */
int	outf	1;		/*output flag */
int	seqf	0;		/* sequential-random test flag */


int tvec[2];

main(argc,argv)
char  **argv;
{

	int quit();
	int rubout();

	signal(3,quit);		/*catch quit signal */
	signal(2,rubout);	/*catch rubout */
	if(argc<2){
		printf("Specify device to test\n");
		exit();
	}
	tfd = open(argv[1],2);	/*open test device */
	if(tfd<0){
		printf("unable to open %s\n",argv[1]);
		exit();
	}
	time(tvec);
	srand(tvec[1]);		/*init random number generator */
	while(1)
		scommand();
}


/*read and execute a command */

scommand()
{

	while(read(0,line,80) <= 0);
	lp = &line[0];
	abort = 0;	/*abort the test flag */

	switch(*lp++) {

	case 'l':		/*set low address */
		lowbn = getnum();
		break;

	case 'h':		/*set high address */
		highbn = getnum();
		break;

	case 't':		/*set repetition factor */
		repeat = getnum();
		break;

	case 'p':		/*pattern flag */
		patf = getnum();
		if(patf == 2)
			upat = getnum();
		break;

	case 'o':		/*output flag */
		outf = getnum();
		break;

	case 'd':		/*display all the modes */
		display();
		break;

	case 'q':		/*quit */
		exit();

	case 'g':		/*execute the test */
		time(stim);	/*get time test started */
		for(r=0; r<repeat; r++) {
			writpat();
			if(abort)  return;
			readpat();
			if(abort)  return;
		}
		printf("test completed\n");
		tranf = (highbn-lowbn)*2;	/*number of disk transfers */
		rptim();					/*test statistics */
		break;

	case 'w':	/*write test */
		time(stim);
		for(r=0; r<repeat; r++) {
			writpat();
			if(abort)  return;
		}
		printf("write test completed\n");
		tranf = highbn - lowbn;
		rptim();
		break;

	case 'r':	/*read test */
		time(stim);
		for(r=0; r<repeat; r++) {
			readpat();
			if(abort) return;
		}
		printf("read test completed\n");
		tranf = highbn - lowbn;
		rptim();
		break;

	case 's':		/*set sequential-random flag */
		seqf = getnum();
		break;

	default:
		printf("Unrecognized command\n");
		return;

	}
}

/*get an octal or decimal number from input string */
getnum()
{

	register mode,num;

	while(*lp == ' ') lp++;	/*skip blanks */
	mode = 10;
	num = 0;
	if(*lp == '0')		/*octal */
		mode = 8;
	while(*lp != '\n' && *lp != ' ') {
		if(*lp<'0' || *lp>((mode==8)?'7':'9')) {
			printf("invalid numeric argument\n");
			return(0);
		}
		num = num*mode + (*lp++ - '0');
	}
	return(num);
}

/*display all the flags */

display()
{

	printf("low block number: %d\n",lowbn);
	printf("high block number: %d\n",highbn);
	printf("repeat factor: %d\n",repeat);
	if(seqf)
		printf("random access\n");
	else
		printf("sequential access\n");
	if(outf)
		printf("output normal\n");
	else
		printf("output errors only\n");

	switch(patf) {

	case 0:
		printf("pattern: default set\n");
		break;

	case 1:
		printf("pattern: block number\n");
		break;

	case 2:
		printf("pattern: %.1o\n",upat);
		break;

	default:
		printf("invalid pattern flag\n");
		exit();
	}
}

/* do the test */
writpat()
{

	register tpat,i,temp;
	int j;

	if(outf) {
		if(patf != 1)
			printf("write pass %d pattern: %.1o\n",r,getpat());
		else
			printf("write pass %d pattern: block number\n",r);
	}
	if(seek(tfd,lowbn,3)<0) {
		printf("unable to seek to block %d\n",lowbn);
		return;
	}
	for(bn=lowbn; bn!=highbn; bn++) {
		if(abort) return;
		tpat = getpat();
		for(i=0; i<256; i++)
			buffer[i] = tpat;
		if(write(tfd,buffer,512) != 512)
			printf("write error block %d pattern %.1o\n",bn,tpat);
	}
}

readpat()
{

	register tpat,i,temp;
	int eflag, ecount, eword;
	int j;

	if(outf) {
		if(patf != 1)
			printf("read pass %d pattern: %.1o\n",r,getpat());
		else
			printf("read pass %d pattern: block number\n",r);
	}
	if(seek(tfd,lowbn,3)<0) {
		printf("unable to seek to block %d\n",lowbn);
		return;
	}
	for(j=lowbn; j!=highbn; j++) {
		if(abort)  return;
		if(seqf) {	/*random access */
			temp = rand();
			bn = lowbn + (temp%(highbn-lowbn));
			if(seek(tfd,bn,3)<0) {
				printf("seek error block %d\n",bn);
				continue;
			}
		}
		else
			bn = j;
		tpat = getpat();
		if(read(tfd,buffer,512) != 512) {
			printf("read error block %d\n",bn);
			continue;
		}
		ecount=1; eflag=0;
		for(i=0; i<256; i++) {
			if(buffer[i] != tpat) {
				if(eflag==0) {
					if(ecount>1)
						printf("compare error count %d\n",ecount);
newerr:
					eflag++;
					ecount = 1;
					eword = buffer[i];
					printf("compare error block %d word %d pattern %.1o bad %.1o\n",
					bn,i,tpat,buffer[i]);
				}
				else {
					if(eword==buffer[i])
						ecount++;
					else
						goto newerr;
				}
			}
		}	/*endfor */
		if(ecount>1)
			printf("compare error count %d\n",ecount);
	}
}

/* get a pattern for the current block */
getpat()
{

	switch(patf) {

	case 0:		/*default set */
		return(pat[r%NUMPAT]);

	case 1:		/*block number */
		return(bn);

	case 2:
		return(upat);

	}
}

quit()
{
	signal(3,quit);
	printf("test aborted\n");
	abort++;
}

rubout()
{
	printf("pass %d block %d pattern %.1o\n",r,bn,getpat());
	signal(2,rubout);
	return;
}

rptim()
{

	time(etim);		/*test end time */
	printf("%d passes with %d transfers per pass in %d seconds\n",
		repeat,tranf,etim[1]-stim[1]);
}
