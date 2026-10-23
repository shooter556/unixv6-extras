# include "t..c"
struct optstr {char *optnam; int *optadd;} options [] {
	"expand", &expflg,
	"center", &ctrflg,
	"box", &boxflg,
	"allbox", &allflg,
	"doublebox", &dboxflg,
	"frame", &boxflg,
	"doubleframe", &dboxflg,
	"tab", &tab,
	0,0};
getcomm()
{
char line[200], *cp;
struct optstr *lp;
int c;
for(lp= options; lp->optnam; lp++)
	*(lp->optadd) = 0;
texname='a';
tab = '\t';
gets1(line);
/* see if this is a command line */
if (index(line,';') < 0)
	{
	backrest(line);
	return;
	}
for(cp=line; (c = *cp) != ';'; cp++)
	{
	if (!letter(c)) continue;
	for(lp= options; lp->optadd; lp++)
		{
		if (prefix(lp->optnam, cp))
			{
			*(lp->optadd) = 1;
			while (letter(*cp)) cp++;
			if (lp->optadd == &tab)
				{
				while (*cp==' ')cp++;
				if (*cp == '('){
					*(lp->optadd) = *++cp;
					cp++;
					}
				}
			break;
			}
		}
	if (letter(*cp)) error("Illegal option");
	cp--;
	}
cp++;
backrest(cp);
return;
}
backrest(cp)
	char *cp;
{
char *s;
for(s=cp; *s; s++);
un1getc('\n', cin);
while (s>cp)
	un1getc(*--s, cin);
return;
}
