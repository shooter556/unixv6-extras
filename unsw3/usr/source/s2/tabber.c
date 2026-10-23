main()
{
	extern fin, fout;
	register pos, opos, c;

	fin = dup(0);
	fout = dup(1);
	while(c = getchar()) {
		if(c == '\n') {
			pos = 0;
			opos = 0;
			putchar(c);
		} else if(c == ' ') {
			pos++;
		} else {
			while((opos|7)+1 <= pos) {
				opos = (opos|7) + 1;
				putchar('\t');
			}
			while(opos < pos) {
				putchar(' ');
				opos++;
			}
			putchar(c);
			pos++;
			opos++;
		}
	}
	flush();
}
