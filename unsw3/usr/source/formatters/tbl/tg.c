# include "t..c"
gettext(sp, ilin,icol, fn, sz)
	char *sp, *fn, *sz;
{
/* get a section of text */
char line[256];
if (texname >'z') error("Too many text block diversions");
printf(".br\n");
printf(".di %c+\n", texname);
if (textflg==0)
	{
	printf(".nr %d \\n(.lu\n", TMP); /* remember old line length */
	textflg=1;
	}
rstofill();
if (fn && *fn) printf(".ft %s\n", fn);
if (sz && *sz) printf(".ps %s\n.vs \\n(.s+2\n",sz);
if (cll[icol][0])
	printf(".ll %sn\n", cll[icol]);
else
	printf(".ll \\n(%du*%du/%du\n",TMP,ctspan(ilin,icol),ncol+1);
while (gets1(line))
	{
	if (line[0]=='T' && line[1]=='}' && line[2]== tab) break;
	if (match("T}", line)) break;
	printf("%s\n", line);
	}
if (fn && *fn) printf(".ft P\n");
if (sz && *sz) printf(".br\n.ps\n.vs\n");
printf(".br\n");
printf(".di\n");
printf(".nr %c| \\n(dn\n", texname);
printf(".nr %c- \\n(dl\n", texname);
/* copy remainder of line */
if (line[2])
	tcopy (sp, line+3);
else
	*sp=0;
return(texname++);
}
untext()
{
printf(".nf\n");
printf(".ll \\n(%du\n", TMP);
}
