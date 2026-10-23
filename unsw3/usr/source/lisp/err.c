char	line[70];
char	size;
char	*msg;

main()
{

	while(size = getline()) {
		write(1, &size, 1);
		write(1, msg, 63);
	}
}

getline()
{
	register char *p, *q;
	p = line;
	q = p;
	while(*p = getchar())
		if(*p++ == '\n') {
			if(p[-2] == '\\')
				p =- 2;
			while(*q++ != '\t');
			msg = q;
			return(p-q);
		}
	return(0);
}
