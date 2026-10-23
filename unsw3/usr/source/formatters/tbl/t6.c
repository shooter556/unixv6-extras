# include "t..c"
maktab()
{
# define FN(i,c) font[stynum[i]][c]
# define SZ(i,c) csize[stynum[i]][c]
/* define the tab stops of the table */
int icol, ilin, tsep, k, ik;
int doubled[MAXCOL], acase[MAXCOL];
char *s;
for(icol=0; icol <ncol; icol++)
	{
	doubled[icol] = acase[icol] = 0;
	printf(".nr %d 0\n", icol+CRIGHT);
	for(ilin=0; ilin<nlin; ilin++)
		{
		if (instead[ilin]|| fullbot[ilin]) continue;
		if (fspan(ilin,icol)) continue;
		switch(ctype(ilin,icol))
			{
			case 'a':
				acase[icol]=1;
			case 'n':
				if (table[ilin][icol].rcol!=0)
					{
					if (doubled[icol]==0)
						printf(".nr %d 0\n.nr %d 0\n", S1, S2);
					doubled[icol]=1;
					if (real(s=table[ilin][icol].col))
						{
						printf(".nr %d ", TMP);
						wide(s, FN(ilin,icol), SZ(ilin,icol)); printf("\n");
						printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n", S1, TMP, S1, TMP);
						}
					if (real(s=table[ilin][icol].rcol))
						{
						printf(".nr %d \\w%c%s%c\n",TMP, F1, s, F1);
						printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n",S2,TMP,S2,TMP);
						}
					continue;
					}
			case 'r':
			case 'c':
			case 'l':
				if (real(s=table[ilin][icol].col))
					{
					printf(".nr %d ", TMP);
					wide(s, FN(ilin,icol), SZ(ilin,icol)); printf("\n");
					printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n", icol+CRIGHT, TMP, icol+CRIGHT, TMP);
					}
			}
		}
	if (acase[icol])
		printf(".if \\n(%d>=\\n(%d .nr %d \\n(%du+2n\n",S2,icol+CRIGHT,icol+CRIGHT,S2);
	if (doubled[icol])
		{
		printf(".nr %d \\n(%d\n", icol+CMID, S1);
		printf(".nr %d \\n(%d+\\n(%d\n",TMP,icol+CMID,S2);
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",TMP,icol+CRIGHT,icol+CRIGHT,TMP);
		printf(".if \\n(%d<\\n(%d .nr %d +(\\n(%d-\\n(%d)/2\n",TMP,icol+CRIGHT,icol+CMID,icol+CRIGHT,TMP);
		}
	if (cll[icol][0])
		{
		printf(".nr %d %sn\n", TMP, cll[icol]);
		printf(".if \\n(%d<\\n(%d .nr %d \\n(%d\n",icol+CRIGHT, TMP, icol+CRIGHT, TMP);
		}
	for(ilin=0; ilin<nlin; ilin++)
	if (k=lspan(ilin, icol))
		{
		printf(".nr %d ", TMP);
		wide(table[ilin][icol-k].col, FN(ilin,icol-k), SZ(ilin,icol-k));
		for(ik=k; ik>=0; ik--)
			printf("-\\n(%d",CRIGHT+icol-ik);
		printf("\n");
		printf(".if \\n(%d>0 .nr %d \\n(%d/%d\n", TMP, TMP, TMP, k);
		printf(".if \\n(%d<0 .nr %d 0\n", TMP, TMP);
		for(ik=1; ik<=k; ik++)
			{
			if (doubled[icol-k+ik])
				printf(".nr %d +\\n(%d/2\n", icol-k+ik+CMID, TMP);
			printf(".nr %d +\\n(%d\n", icol-k+ik+CRIGHT, TMP);
			}
		}
	}
/* if even requested, make all columns widest width */
# define TMP1 S1
# define TMP2 S2
if (evenflg)
	{
	printf(".nr %d 0\n", TMP);
	for(icol=0; icol<ncol; icol++)
		{
		if (evenup[icol]==0) continue;
		printf(".if \\n(%d>\\n(%d .nr %d \\n(%d\n",
		icol+CRIGHT, TMP, TMP, icol+CRIGHT);
		}
	for(icol=0; icol<ncol; icol++)
		{
		if (evenup[icol]==0)
			/* if column not evened just retain old interval */
			continue;
		if (doubled[icol])
			printf(".nr %d (100*\\n(%d/\\n(%d)*\\n(%d/100\n",
				icol+CMID, icol+CMID, icol+CRIGHT, TMP);
				/* that nonsense with the 100's and parens tries
				   to avoid overflow while proportionally shifting
				   the middle of the number */
		printf(".nr %d \\n(%d\n", icol+CRIGHT, TMP);
		}
	}
/* now adjust for total table width */
for(tsep=icol=0; icol<ncol; icol++)
	tsep=+ sep[icol];
if (expflg)
	{
	printf(".nr %d 0", TMP);
	for(icol=0; icol<ncol; icol++)
		printf("+\\n(%d", icol+CRIGHT);
	printf("\n");
	printf(".nr %d \\n(.l-\\n(%d\n", TMP, TMP);
	if (boxflg || dboxflg || allflg)
		tsep =+ 1;
	else
		tsep =- sep[ncol-1];
	printf(".nr %d \\n(%d/%d\n", TMP, TMP,  tsep);
	printf(".if \\n(%d<0 .nr %d 0\n", TMP, TMP);
	}
else
	printf(".nr %d 1n\n", TMP);
printf(".nr %d 0\n",CRIGHT-1);
tsep= (boxflg || allflg || dboxflg || left1flg) ? 1 : 0;
for(icol=0; icol<ncol; icol++)
	{
	printf(".nr %d \\n(%d+(%d*\\n(%d)\n",icol+CLEFT, icol+CRIGHT-1, tsep, TMP);
	printf(".nr %d +\\n(%d\n",icol+CRIGHT, icol+CLEFT);
	if (doubled[icol])
		{
		printf(".nr %d +\\n(%d\n", icol+CMID, icol+CLEFT);
	/*  printf(".if n .if \\n(%d%%24>0 .nr %d +12u\n",icol+CMID, icol+CMID); */
		}
	tsep=sep[icol];
	}
if (rightl)
	printf(".nr %d (\\n(%d+\\n(%d)/2\n",ncol+CRIGHT-1, ncol+CLEFT-1, ncol+CRIGHT-2);
printf(".nr TW \\n(%d\n", ncol+CRIGHT-1);
if (boxflg || allflg || dboxflg)
	printf(".nr TW +%d*\\n(%d\n", sep[ncol-1], TMP);
printf(".if t .if (\\n(TW+\\n(.o)>7.75i .tm Table at line %d file %s is too wide - \\n(TW units\n", iline-1, ifile);
return;
}
wide(s, fn, size)
	char *s, *size, *fn;
{
if (point(s))
	{
	printf("\\w%c", F1);
	if (*fn>0) putfont(fn);
	if (*size) putsize(size);
	printf("%s", s);
	if (*fn>0) putfont("P");
	if (*size) putsize("0");
	printf("%c",F1);
	}
else
	printf("\\n(%c-", s);
}
