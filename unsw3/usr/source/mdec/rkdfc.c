#

/***	this program created by ian j.	
 ***
 ***					july '76
 ***
 ***	performs certain necessary i/o functions for
 ***	rk05 disks:
 ***
 ***	  h for help
 ***	  c for copy function
 ***	  f for format function
 ***	  k for compare function
 ***	  r for read function
 ***
 ***	to create:
 ***			cc -c rkdfc.c
 ***			as rkdfs.s
 ***			ld a.out rkdf.o -lc -l
 ***			mv a.out rkdf
 ***
 */

#define	done	0200

#define	TT	0177560

struct	{
	int	csr;
	int	csb;
	int	tsr;
	int	tsb;
};

#define	RK	0177400

struct	{
	int	rkds;
	int	rker;
	int	rkcs;
	int	rkwc;
	int	rkba;
	int	rkda;
};

/* rk i/o requests */

#define	cntrlr		0001		/* control reset */
#define	write		0403	
#define	read		0405
#define	writechk	0407
#define	seek		0411
#define	readchk		0413
#define	driver		0015		/* drive reset */
#define	wlock		0417
#define	fmtw	       06003		/* format write */

char *bigbuf;	/* pointer to buffer area */
int	iob;	/* number of blocks per big i/o */
int	nio;	/* number of i/o for these blocks */
int    *szio;	/* number of words for each of above i/os */
int	iov;	/* number of blocks left after the big ones */
int    *szov;	/* number of words for this last i/o */

int in;		/* drive number for current input operation */

int out;	/* drive number for current output operation */

main()
{
	register int i;
	register char *r,*q;
for(;;){
	printf("\n\nwhich function is desired ?? ");
	i=getchar();	putchar('\n');
	in=out= -1;
	switch(i){
	default:
			printf("\n\nyou are reqd. to enter a char\n");
			printf("to indicate the function you want\n");
			printf("        h -- help\n");
			printf("        f -- format\n");
			printf("        c -- copy\n");
			printf("        r -- read\n");
			printf("        k -- compare\n");
			break;
	case 'f':
			out=gdrivnum("format drive# ");
			format(out);
			reed(out);
			break;
	case 'k':
			in =gdrivnum(" input drive# ");
			out=gdrivnum("output drive# ");
			check(in,out);
			break;
	case 'c':
			in =gdrivnum(" input drive# ");
			out=gdrivnum("output drive# ");
			wrlock(in) ; /*****/
			reed(in);
			format(out);
			copy(in,out);
			check(in,out);
			break;
	case 'r':
			in=gdrivnum(" input drive# ");
			wrlock(in) ; /*****/
			reed(in);
			break;
	};
        }
}

wrlock(unit)
{
	rkio(unit,0,0,017,0);
	printf("drive %d write protected !!\n",unit);
}

format(unit)
{
	int register i;
	bigbuf[0]=0;
	printf("begin formatting drive %d\n",unit);
	for(i=0;i<nio;i++)
		rkio(unit,bigbuf,iob*i,fmtw,szio);
	rkio(unit,bigbuf,iob*i,fmtw,szov);
	printf("formatting complete !!\n");
}

copy(iunit,ounit)
{
	int register i;
	wrlock(iunit);
	printf("begin copying drive %d to drive %d\n",in,out);
	for(i=0;i<nio;i++){
			rkio(iunit,bigbuf,i*iob,read,szio);
			rkio(ounit,bigbuf,i*iob,write,szio);
			rkio(ounit,bigbuf,i*iob,writechk,szio);
	}
	rkio(iunit,bigbuf,i*iob,read,szov);
	rkio(ounit,bigbuf,i*iob,write,szov);
	rkio(ounit,bigbuf,i*iob,writechk,szov);
	printf("copying complete !!\n");
}

check(iunit,ounit)
{
	int register i;
	wrlock(iunit); wrlock(ounit);
	printf("begin comparing drive %d to drive %d\n",in,out);
	for(i=0;i<nio;i++){
			rkio(iunit,bigbuf,i*iob,read,szio);
			rkio(ounit,bigbuf,i*iob,writechk,szio);
	}
	rkio(iunit,bigbuf,i*iob,read,szov);
	rkio(ounit,bigbuf,i*iob,writechk,szov);
	printf("comparing complete !!\n");
}

reed(unit)
{
	int register i;
/**	wrlock(unit); **/
	printf("begin reading drive %d\n",unit);
	for(i=0;i<nio;i++)
			rkio(unit,bigbuf,i*iob,readchk,szio);
	rkio(unit,bigbuf,i*iob,readchk,szov);
	printf("reading complete !!\n");
}


gdrivnum(s)
char *s;
{
	int register c;
 l:	printf(s);
	c=getchar();	putchar('\n');
	if( ((c=- '0')<0) || (c>7)  ) goto l;
	return(c);
}

putchar(c)
{
	TT->tsb = c;
	while((TT->tsr & done) == 0);
	if(c == '\n') {
			putchar(0);putchar(0);putchar(0);
			putchar(0);putchar(0);putchar(0);
			putchar('\r');
	}
}

getchar()
{	int register c;
	while( (TT->csr&done) == 0 );
	putchar(c=TT->csb&0177);
	if( (c>='A') && (c<='Z') ) c=+ 'a'-'A';
	return(c);
}

rkio(unit,buffer,block,cmd,wc)
{	register nretry;
	nretry  = 15;

 io:	while((RK->rkcs & done) == 0);

	RK->rkda = ((block/12) << 4) | (block % 12) | (unit << 13);
	RK->rkba = buffer;
	RK->rkwc = -wc;
	RK->rkcs = cmd;

	while((RK->rkcs & done) == 0);

	if(RK->rkcs < 0) {
printf("error on drive%d block=%4d -- ds=%6o da=%6o cs=%6o er=%6o\n",
		(RK->rkda>>13)&07,
		12*(RK->rkda&017760>>4)+(RK->rkda&017),
		RK->rkds,RK->rkda,RK->rkcs,RK->rker);
		if(--nretry) {
			RK->rkcs = cntrlr;
			goto io;
		}
		printf("\n recovery impossible\n ");
		restart();
	}
}
/********************************************************/
#define	ssr0	0177572
#define	kisa0	0172340
#define	kisd0	0172300

	int *a;	int k;
	struct { int i; };
initial()
{
	int register j;  int register *d;  int register *x;
	extern char end;  /* identify the end of the bss  */

	bigbuf = (&end+63)&0177700;  a=kisa0;  d=kisd0;
	for(j=0;j<01600;j=+0200) {
		*a++ = j;	/* 4kw 	*/
		*d++ = 077406;  /* r/w  */
	}
	*d = 077406;  *a-- = 07600;
	ssr0->i++;		/* start mem-mngmnt */
	x = 0140000;
	for(*a=(bigbuf/64);*a<07600;*a=+1)for(j=0;j<32;) x[j++]=0;
}
initiam()
{
	int register j;
	ssr0->i = 0; /* stop mem-mngmnt */
	printf("mem =  %lkw\n",(j= *a)/32);
	iob = ((--j) - (bigbuf>>6)) / 8;
	if( iob > 256 ) iob = 256;
	nio = 4872/iob;
	szio = iob * 256;
	iov = 4872 - (nio * iob);
	szov = iov * 256;

	printf("blocks per i/o = %l\n\n",iob);
	printf("iob=%l; nio=%l; szio=%l; iov=%l; szov=%l;\n",
		iob,nio,szio,iov,szov);
}
