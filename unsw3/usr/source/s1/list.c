#
/* */
/*	list - ASCII source lister */
/*		Gerry Barksdale (August 1975) */
/* */

/*
 *	Extensively (but not exhaustively) debugged ...
 *
 *	Piers Lauder	Feb '78
 */

#define	FF	'\014'

char *pmodtime;
/******* file status **********/
struct {
	char minor;
	char major;
	int inumber;
	int flags;
	char nlinks;
	char uid;
	char gid;
	char size0;
	int size1;
	int addr[8];
	long actime;
	long modtime;	} status;

int wbuf[259] 1;
int rbuf[259];
char margin[50];
int mval 0;	/*margin value */
int line, page, col;
int head, number;
int lineperpage 58;
int tabv 8;
int printer 1;	/*output is intended for printer (FF at end of page) */
int lineno 1;	/*line numbering desired */
int maxcol 75;	/*text width - margin - number */
int lines;	/*number of lines printed on this page */
int beginpg 1;
int endpg 32767;
int print, more, fold;

main(argc,argv)
char **argv;
{
	register int c, com, n;

	while (--argc > 0) {
		argv++;

		if(**argv == '-') {
			while(*++*argv) {
				com = **argv;	/*save command */
				c = 0;
				while((n = *++*argv) >= '0' && (n <= '9'))
					c = c*10 + n - '0';
				(*argv)--;

				switch (com) {

					case 'm':	/* set left margin */
						if(c > 49)
							c = 49;
						maxcol =+ (mval-c);
						mval = c;
						margin[c] = 0;
						while (c-- > 0)
							margin[c] = ' ';
						break;

					case 't':	/* set tab value */
						if( (tabv = c) < 1 )
							tabv = 1;
						break;

					case 'c':	/* CRT display mode */
						printer = 0;
						lineperpage = 20;
						margin[0] = 0;
						break;

					case 'l':	/* set page length */
						if( (lineperpage = c) < 1)
							lineperpage = 58;
						break;

					case 'n':	/* no line numbers */
						maxcol =+ 5;
						lineno = 0;
						break;

					case 'w':	/* page width (cols) */
						maxcol = c - 5*lineno - mval;
						break;

					case '+':	/* start printing with this page */
						beginpg = c;
						break;

					case ':':	/* stop printing after this page */
						endpg = c;
						break;

					case 'p':	/* direct to /dev/lp */
						if((wbuf[0] = open("/dev/lp",1)) < 0) {
							wbuf[0] = 1;
							printf("Cannot open printer\n");
							fflush(wbuf);
							return;
						}
						break;

					case 'f':	/*	fold output */
						fold++;
						break;

				}
			}
			argv++;
			if (--argc < 0)
				return;
		}

		if(argc > 0) {
			if((rbuf[0]=open(*argv,0)) < 0)
				break;
			fstat(rbuf[0],&status);
		} else {
			long time();
			status.modtime = time();
			rbuf[0] = 0;	/*standard input */
		}
		pmodtime = ctime(status.modtime);
		for(c=1;c<4;c++)
			rbuf[c] = 0;	/*initialize getc pointers */
		line=1;
		page=0;
		head=1;	/*force heading on first line */
		more = 1;
		number = 1;

		while (((c=getc(rbuf)) >= 0) && more) {
			c =& 0177;
			while(head) {
				lines = head = 0;
				print = ((++page < beginpg) ? 0 : 1);
				if(page > endpg)
					more = print = 0;
				if(!print)
					break;
				if(printer) {
					putc(FF,wbuf);
				} else {
					fflush(wbuf);
					read(0,&status,1);	/*wait for a char to release page */
				}
				printf("%s%s    Page %d    %s\n",
					margin,*argv,page,pmodtime);
			}
			if(print && number) {
				printf("%s",margin);
				number = 0;
				if(lineno) {
					printf("%4d ",line);
				}
			}
			if(!print && (c != '\n'))
gobble:
				while((c=getc(rbuf)) >= 0 && (c&0177) != '\n') ; /*gobble */
			switch (c) {
	
			case '\t':
				do
					putc(' ',wbuf);
				while(col++ % tabv -tabv +1);
				break;
	
			case '\n':
				if(print)	putc(c,wbuf);
				col = 0;
				line++;
				if(++lines >= lineperpage)
					head++;
				number++;
				break;

			case '\b':
				if ( --col < 0 )
			case '\r':	col = 0;
				putc( c , wbuf );
				break;

			default:
				if(c >= ' ') {	/*it's printable */
					putc(c,wbuf);
					if(++col >= maxcol) {
						if( ! fold) {
							putc(0177,wbuf);
							goto gobble;
						}
						col = 0;
						printf("\n%s    ",margin);	/*fold line */
						lines++;
					}
				}
			}
		}	/* while getc */

		close(rbuf[0]);
	}	/* while argc */

	if(printer)
		putc(FF,wbuf);
	fflush(wbuf);
}
putchar(c)
{	putc(c,wbuf);	}	/*redirect printf() output */
