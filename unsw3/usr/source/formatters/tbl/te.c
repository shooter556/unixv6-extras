# include "t..c"
error(s)
	char *s;
{
printf(2, "\n%s: line %d: %s\n", ifile, iline, s);
# ifdef unix
printf(2, "tbl quits\n");
cexit(1);
# endif
# ifdef gcos
printf(2, "run terminated due to error condition detected by tbl preprocessor\n");
cexit(0);
# endif
}
gets1(s)
	char *s;
{
char *p;
iline++;
p=gets(s);
	while (*s) s++;
	s--;
	if (*s == '\\')
		gets1(s);
return(p);
}
# define BACKMAX 500
char backup[BACKMAX];
char *backp backup;
un1getc(c, f)
{
if (c=='\n')
	iline--;
*backp++ = c;
if (backp >= backup+BACKMAX)
	error("too much backup");
}
get1char()
{
int c;
if (backp>backup)
	c = *--backp;
else
	c=getchar();
if (c== '\n')
	iline++;
return(c);
}
