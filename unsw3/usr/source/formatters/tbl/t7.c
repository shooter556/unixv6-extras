# include "t..c"
# define realsplit ((ct=='a'||ct=='n') && table[i][c].rcol)
runout()
{
int i, c;
if (boxflg || allflg || dboxflg) need();
if (ctrflg)
	{
	printf(".nr #I \\n(.i\n");
	printf(".in +(\\n(.lu-\\n(TWu-\\n(.iu)/2u\n");
	}
printf(".fc %c %c\n", F1, F2);
printf(".nr #T 0\n");
deftail();
for(i=0; i<nlin; i++)
	putline(i,i);
if (leftover)
	yetmore();
printf(".fc\n");
printf(".nr T. 1\n");
printf(".T# 1\n");
if (ctrflg)
	printf(".in \\n(#Iu\n");
}
runtabs(i)
{
int c, ct;
printf(".ta ");
for(c=0; c<ncol; c++)
	{
	if (fspan(i,c))
		continue;
	switch(ct=ctype(i,c))
		{
		case 'n':
		case 'a':
			if (table[i][c].rcol)
			  if (lused[c]) /*Zero field width*/
				printf("\\n(%du ",c+CMID);
		case 'c':
		case 'l':
		case 'r':
		    if (realsplit? rused[c]: used[c])
			printf("\\n(%du ",c+CRIGHT);
			continue;
		case 's':
			if (lspan(i,c))
				printf("\\n(%du ", c+CRIGHT);
			continue;
		}
	}
printf("\n");
}
ifline(s)
	char *s;
{
if (s[1] ) return(0);
if (s[0] == '_') return('-');
if (s[0] == '=') return('=');
return(0);
}
need()
{
int texlin, horlin, i;
for(texlin=horlin=i=0; i<nlin; i++)
	{
	if (fullbot[i]!=0)
		horlin++;
	else
	if (instead[i]!=0)
		continue;
	else
		texlin++;
	}
printf(".ne %dv+%dp\n",texlin,2*horlin);
}
deftail()
{
int i, c, lf, lwid;
printf(".eo\n");
printf(".de T#\n");
printf(".ds #d .d\n");
printf(".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
	printf(".mk ##\n");
	printf(".nr ## -1v\n");
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			printf(".if \\n(#T>0 .nr #%c \\n(#T\n",linestop[i]+'a'-1);
if (boxflg || allflg || dboxflg) /* bottom of table line */
	if (fullbot[nlin-1]==0)
		{
		printf(".if \\n(T. .vs2p\n");
		printf(".if \\n(T. ");
		drawline(nlin-1,0,ncol, dboxflg ? '=' : '-',1);
		printf("\n.if \\n(T. .vs\n");
		/* T. is really an argument to a macro but because of 
		   eqn we don't dare pass it as an argument and reference by $1 */
		}
	for(c=0; c<ncol; c++)
		{
		if ((lf=left(nlin-1,c, &lwid))>=0)
			{
			printf(".if \\n(#%c>0 .sp -1\n",linestop[lf]+'a'-1);
			printf(".if \\n(#%c>0 ", linestop[lf]+'a'-1);
			tohcol(c);
			drawvert(lf, nlin-1, c, lwid);
			printf("\\h'|\\n(TWu'\n");
			}
		}
	if (boxflg || allflg || dboxflg) /* right hand line */
		{
		printf(".if \\n(#a>0 .sp -1\n");
		printf(".if \\n(#a>0 \\h'|\\n(TWu'");
		drawvert (0, nlin-1, ncol, dboxflg? 2 : 1);
		printf("\n");
		}
printf("..\n");
printf(".ec\n");
}
