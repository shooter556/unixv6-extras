# include "t..c"
# define realsplit ((ct=='a'||ct=='n') && table[nl][c].rcol)
int watchout;
int once;
putline(i, nl)
	/* i is line number for deciding format */
	/* nl is line number for finding data   usually identical */
{
int c, lf, ct, form, lwid, vspf;
char *s, *size, *fn;
watchout=vspf=0;
if (i==0) once=0;
if (i==0 && ( allflg || boxflg || dboxflg))
	fullwide(0,   dboxflg? '=' : '-');
if (instead[nl]==0 && fullbot[nl] ==0)
for(c=0; c<ncol; c++)
	{
	s = table[nl][c].col;
	if (s==0) continue;
	if (point(s)) continue;
	printf(".ne \\n(%c|u\n",s);
	}
if (linestop[nl])
	printf(".mk #%c\n", linestop[nl]+'a'-1);
lf = prev(nl);
for(c=0; c<ncol; c++)
	{
	if (instead[nl]==0 && fullbot[nl]==0)
	if (vspen(table[nl][c].col)) vspf=1;
	if (lf>=0)
		if (vspen(table[lf][c].col)) vspf=1;
	}
if (vspf)
	{
	printf(".nr #^ \\n(\\*(#du\n");
	printf(".nr #- \\n(#^\n"); /* current line position relative to bottom */
	}
vspf=0;
for(c=0; c<ncol; c++)
	{
	s = table[nl][c].col;
	if (s==0) continue;
	if (point(s) ) continue;
	lf=prev(nl);
	if (lf>=0 && vspen(table[lf][c].col))
		printf(".if (\\n(%c|+\\n(^%c-1v)>\\n(#- .nr #- +(\\n(%c|+\\n(^%c-\\n(#--1v)\n",s,'a'+c,s,'a'+c);
	else
		printf(".if (\\n(%c|+\\n(#^-1v)>\\n(#- .nr #- +(\\n(%c|+\\n(#^-\\n(#--1v)\n",s,s);
	}
if (instead[nl])
	{
	puts(instead[nl]);
	return;
	}
if (fullbot[nl])
	{
	switch (ct=fullbot[nl])
		{
		case '=':
		case '-':
			fullwide(nl,ct);
		}
	return;
	}
if (allflg && once>0 )
	fullwide(i,'-');
once=1;
runtabs(i);
if (allh(i))
	printf(".vs 2p\n");
printf("\\&");
for(c=0; c<ncol; c++)
	{
	if (rightl && c+1==ncol) continue;
	if (i+1<nlin && (lf=left(i,c, &lwid))>=0)
		{
		tohcol(c);
		drawvert(lf, i, c, lwid);
		}
	if ((form=ctype(i,c))!='s')
		printf("\\h'|\\n(%du'",
			 ((form=='n'||form=='a')&&lused[c]==0)?c+CMID:c+CLEFT);
	s= table[nl][c].col;
	fn = font[stynum[i]][c];
	size = csize[stynum[i]][c];
	if (*size==0)size=0;
	switch(ct=ctype(i, c))
		{
		case 'n':
		case 'a':
			if (table[nl][c].rcol)
				{
			   if (lused[c]) /*Zero field width*/
				{
				printf("%c%c",F1,F2);
				puttext(table[nl][c].col,fn,size,nl,c);
				printf("%c",F1);
				}
				s= table[nl][c].rcol;
				form=1;
				break;
				}
		case 'c':
			form=3; break;
		case 'r':
			form=2; break;
		case 'l':
			form=1; break;
		case '-':
		case '=':
			makeline(i,c,ct);
			continue;
		default:
			continue;
		}
	if (realsplit ? rused[c]: used[c]) /*Zero field width*/
		; else continue;
	/* form: 1 left, 2 right, 3 center adjust */
	if (ifline(s))
		{
		makeline(i,c,ifline(s));
		continue;
		}
	printf("%c", F1);
	if (form!= 1)
		printf("%c", F2);
	if (vspen(s))
		vspf=1;
	else
	puttext(s, fn, size, nl,c);
	if (form !=2)
		printf("%c", F2);
	printf("%c", F1);
	/* if lines need to be split for gcos here is the place for a backslash */
	}
printf("\n");
if (allh(i)) printf(".vs\n");
if (watchout)
	funnies(i,nl);
if (vspf)
	{
	for(c=0; c<ncol; c++)
		if (vspen(table[nl][c].col) && (nl==0 || (lf=prev(nl))<0 || !vspen(table[lf][c].col)))
			printf(".nr ^%c \\n(#^u\n", 'a'+c);
	}
}
puttext(s,fn, size, ilin, icol)
	char *s, *size, *fn;
{
	int ip, cmidx;
if (point(s))
	{
	putfont(fn);
	putsize(size);
	ip = prev(ilin);
	cmidx = ctop[stynum[ilin]][icol]==0;
	if (ip>=0)
	if (vspen(table[ip][icol].col))
		{
		printf("\\v'-(\\n(\\*(#du-\\n(^%cu", icol+'a');
		if (cmidx)
			printf("-((\\n(#-u-\\n(^%cu)/2u)", icol+'a');
		printf("'");
		}
	printf("%s",s);
	if (ip>=0)
	if (vspen(table[ip][icol].col))
		{
		printf("\\v'(\\n(\\*(#du-\\n(^%cu", icol+'a');
		if (cmidx)
			printf("-((\\n(#-u-\\n(^%cu)/2u)", icol+'a');
		printf("'");
		}
	if (*fn>0) putfont("P");
	if (size!=0) putsize("0");
	}
else
	watchout=1;
}
funnies( stl, lin)
{
/* write out funny diverted things */
int c, s;
char *fn;
printf(".mk ##\n"); /* rmember current vertical position */
printf(".nr %d \\n(##\n", S1); /* bottom position */
for(c=0; c<ncol; c++)
	{
	s = table[lin][c].col;
	if (point(s)) continue;
	if (s==0) continue;
	printf(".sp |\\n(##u-1v\n");
	printf(".nr %d ", SIND);
	switch(ctype(stl,c))
		{
		case 'n':
		case 'c':
			printf("(\\n(%du+\\n(%du-\\n(%c-u)/2u\n",c+CLEFT,c-1+ctspan(lin,c)+CRIGHT, s);
			break;
		case 'l':
		case 'a':
			printf("\\n(%du\n",c+CLEFT);
			break;
		case 'r':
			printf("\\n(%du-\\n(%c-u\n", c+CRIGHT, s);
			break;
		}
	printf(".in +\\n(%du\n", SIND);
	fn=font[stynum[stl]][c];
	putfont(fn);
	if (stl>0 && vspen(table[prev(stl)][c].col))
		{
		printf(".sp |\\n(^%cu\n", 'a'+c);
		if (ctop[stynum[stl]][c]==0)
			{
			printf(".nr %d \\n(#-u-\\n(^%c-\\n(%c|+1v\n",TMP, 'a'+c, s);
			printf(".if \\n(%d>0 .sp \\n(%du/2u\n", TMP, TMP);
			}
		}
	printf(".%c+\n",s);
	printf(".in -\\n(%du\n", SIND);
	if (*fn>0) putfont("P");
	printf(".mk %d\n", S2);
	printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n", S2, S1, S1, S2);
	}
printf(".sp |\\n(%du\n", S1);
}
putfont(fn)
	char *fn;
{
if (fn && *fn)
	printf( fn[1] ? "\\f(%.2s" : "\\f%.2s",  fn);
}
putsize(s)
	char *s;
{
if (s && *s)
	printf("\\s%s",s);
}
