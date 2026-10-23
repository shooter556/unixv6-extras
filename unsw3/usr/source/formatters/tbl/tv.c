# include "t..c"
drawvert(start,end, c, lwid)
{
char *exb1, *exb2, *ext1, *ext2;
int sl, ln, tp, pos;
tp =exb1 =exb2 =ext1 = ext2 = 0;
end++;
while (instead[end]) end++;
for(ln=0; ln<lwid; ln++)
	{
	pos = 2*ln-lwid+1;
	if (pos!=tp) printf("\\h'%dp'", pos-tp);
	tp = pos;
	if (end<nlin)
		{
		if (fullbot[end]|| (!instead[end] && allh(end)))
			exb1= "2p";
		else
		switch (midbar(end,c))
			{
			case '-':
			exb1 = "(1v-.5m)"; break;
			case '=':
			exb1 = "(1v-.5m+1p)"; break;
			}
		}
	if (lwid>1)
	switch(interh(end, c))
		{
		case THRU: exb2 ="-1p"; break;
		case RIGHT: exb2 = ln==0 ? "1p" : "-1p";break;
		case LEFT: exb2 = ln==1 ? "1p" : "-1p"; break;
		}
	if (lwid==1)
	switch(interh(end,c))
		{
		case THRU: exb2 = "-1p"; break;
		case RIGHT: case LEFT: exb2 = "1p"; break;
		}
	if (start>0)
		{
		sl = start-1;
		while (instead[sl]&&sl>=0) sl--;
		if (fullbot[sl] || allh(sl))
			ext1 = "0p";
		else
		if (sl>=0)
		switch(midbar(sl,c))
			{
			case '-':
			ext1= ".5m"; break;
			case '=':
			ext1= "(.5m+1p)"; break;
			default:
			ext1 = "-2p"; break;
			}
		else
			ext1 = "2p"; 
		}
	if (lwid>1)
		switch(interh(start,c))
			{
			case THRU: ext2 = "1p"; break;
			case LEFT: ext2 = ln==0 ? "1p" : "-1p"; break;
			case RIGHT: ext2 = ln==1 ? "1p" : "-1p"; break;
			}
	else if (lwid==1)
		switch(interh(start,c))
			{
			case THRU: ext2 = "1p"; break;
			case LEFT: case RIGHT: ext2 = "-1p"; break;
			}
	if (exb1)
		printf("\\v'%s'", exb1);
	if (exb2)
		printf("\\v'%s'", exb2);
	printf("\\L'|\\n(#%cu-1v", linestop[start]+'a'-1);
	if (ext1)
		printf("-%s",ext1);
	if (ext2)
		printf("+%s", ext2);
	if (exb1)
		printf("-%s", exb1);
	if (exb2)
		printf("-%s", exb2);
	/* the string #d is either "nl" or ".d" depending
	   on diversions; on GCOS not the same */
	printf("'\\v'\\n(\\*(#du-\\n(#%cu+1v", linestop[start]+'a'-1);
	if (ext1)
		printf("+%s",ext1);
	if (ext2)
		printf("-%s", ext2);
	printf("'");
	}
}


midbar(i,c)
{
int k;
k = midbcol(i,c);
if (k==0 && c>0)
	k = midbcol(i, c-1);
return(k);
}
midbcol(i,c)
{
int ct;
while ( (ct=ctype(i,c)) == 's')
	c--;
if (ct=='-' || ct == '=')
	return(ct);
if (ct=barent(table[i][c].col))
	return(ct);
return(0);
}

barent(s)
	char *s;
{
if (s==0) return (1);
if (s[1]!= 0)
	return(0);
switch(s[0])
	{
	case '_':
		return('-');
	case '=':
		return('=');
	}
return(0);
}
