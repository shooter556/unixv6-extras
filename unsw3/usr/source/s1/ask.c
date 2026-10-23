char	buf[100];


yes()
{	exit(0);	}

no()
{	exit(1);	}

main(c,v)
char	**v;
{
	extern fout;
	fout = 2;
	v++;
	while(--c)
		printf("%s ",*v++);
	printf("? ");
	flush();
	signal(15,no);
	clktim(30);
	read(2,buf,sizeof buf);
	if(buf[0] != 'y') no();
	yes();
}
