# include "t..c"
savefill()
{
/* remembers the status of fill mode in nr 45 (SF) */
/* nr SF = 0 means no fill, 1 means fill */
printf(".nr %d \\n(.u\n", SF);
printf(".nf\n");
}
rstofill()
{
printf(".if \\n(%d>0 .fi\n", SF);
}
endoff()
{
int i;
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			printf(".nr #%c 0\n", 'a'+i);
printf("%s\n", last);
}
ifdivert()
{
printf(".ds #d .d\n");
printf(".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
}
