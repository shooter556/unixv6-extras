#
# include "t..c"
# define BROKPIPE 13

# ifdef unix
# define MACROS "/usr/lib/tmac.s"
# endif

# ifdef gcos
# define MACROS "cc/troff/smac"
# endif

# define ever (;;)

main(argc,argv)
	char *argv[];
{
#ifdef gcos
if(!intss()) cout = copen("*qq", 'w'); /* default media code is type 5 */
#endif
# ifdef unix
int badsig();
signal(BROKPIPE, badsig);
# endif
cexit(tbl(argc,argv));
}


tbl(argc,argv)
	char *argv[];
{
char line[512];
for ever
{
	if (argc>1)
	{
	argc--;
	argv++;
	if (match("-ms", *argv))
		*argv = MACROS;
	cin = copen(ifile= *argv, 'r');
	iline=1;
	if (cin < 0)
		{
		printf(2, "can't open input file %s\n", ifile);
		return(1);
		}
	}
while (gets1(line))
	{
	puts(line);
	if (prefix(".TS", line))
		tableput();
	}
cclose(cin);
if (argc <= 1) break;
}
return(0);
}
# ifdef unix
badsig()
{
signal(BROKPIPE, 1);
 cexit(0);
}
# endif
