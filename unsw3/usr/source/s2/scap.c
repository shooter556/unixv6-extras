#
char *copr "Copyright (c) 1975 Michael D. Tilson";
#define PMAX 	132
#define CR	'\r'
#define HT	'\t'
#define FF	014
#define BS	010
#define NL	'\n'

/*	filter to underscore capital letters on the line printer.
 *	will also work on any terminal which can interpret carriage return
 *	and backspace commands 'properly'.
 *	handles a maximum of 132 characters per line.
 */


cline(line)
	char line[];
	{
	register int i;
	for (i = 0; i<PMAX; i++) line[i] = ' ';
	}

main() {
	register char c;
	static char line[PMAX+1];
	int linep;
	register int i, p;
	extern int fout;

	fout = dup(1);

	p = 0;
	linep = -1;
	cline(line);

	while((c=getchar()) != '\0') {
		switch(c) {
		default:  if ('A'<=c && c<='Z' && p<PMAX)
				line[linep=p]='_';
			  if(c>=' ') {
				  p++;
				  putchar(c);
			  }
			  break;

		case BS:  --p;
			putchar(c);
			break;

		case CR:  p=0;
			putchar(c);
			break;

		case HT:  p =+ (8 - p%8);
			putchar(c);
			break;

		case NL:  p= 0;
		case FF:
			putchar(CR);
			for(i=0;i<=linep;i++)
				putchar(line[i]);
			cline(line);
			linep = -1;
			putchar(c);
			if (i>p) {
				putchar(CR);
				i = 0;
			}
			while(i<p) {
				putchar(' ');
				i++;
			}
			break;
		}
	}
	flush();
}
