#include	"docmod.h"

/* dump.c */

main()

{	if(openf(0))error(EROPEN);
	dumpit();
}


openf(mode)
{	if((kwfid=open("/usr/pub/keywords",mode))<1)return(EROPEN);
	if((klfid=open("/usr/pub/keylinks",mode))<1)return(EROPEN);
	if((tfid=open("/usr/pub/titles",mode))<1)return(EROPEN);
	if((afid=open("/usr/pub/abstracts",mode))<1)return(EROPEN);
	return(OK);
}

dumpit()
{	int i;

	printf("KEYWORDS:\n********\n\n");
	for(i=0;;i++)
	{	printf("%4d : ",i);
		switch(getkwrec(i))
		{	case EOF :	printf("*EOF\n");
					goto out1;
			case FREE :	printf("FREE\n");
					continue;
			case LAST :	printf("LAST");
					goto cont1;
			default :	printf("%4d",kwrec.klfp);
			cont1 :		printf(" %s\n",kwrec.keyword);
					continue;
		}
		out1:	break;
	}
	printf("\fKEYLINKS:\n********\n\n");
	for(i=0;;i++)
	{	printf("%4d : ",i);
		switch(getklrec(i))
		{	case EOF :	printf("*EOF\n");
					goto out2;
			case FREE :	printf("FREE\n");
					continue;
			case LAST :	printf("LAST");
					goto cont2;
			default :	printf("%4d",klrec.nklfp);
			cont2 :		printf(" %4d\n",klrec.tp);
					continue;
		}
		out2:	break;
	}
	printf("\fTITLES:\n*******\n\n");
	for(i=0;;i++)
	{	printf("%4d : ",i);
		switch(gettrec(i))
		{	case EOF :	printf("*EOF\n");
					goto out3;
			case FREE :	printf("FREE\n");
					continue;
			case LAST :	printf("LAST");
					goto cont3;
			default :	printf("%4d",trec.afp);
			cont3 :		printf(" %s\n",trec.title);
					continue;
		}
		out3:	break;
	}
	printf("\fABSTRACTS:\n**********\n\n");
	for(i=0;;i++)
	{	printf("%4d : ",i);
		switch(getarec(i))
		{	case EOF :	printf("*EOF\n");
					goto out4;
			case FREE :	printf("FREE\n");
					continue;
			case LAST :	printf("LAST");
					goto cont4;
			default :	printf("%4d",arec.nafp);
			cont4 :		printf(" %s\n",arec.text);
					continue;
		}
		out4:	break;
	}
}

getkwrec(i)
{	return(getrec(kwfid,i,&kwrec,sizeof kwrec));
}

getklrec(i)
{	return(getrec(klfid,i,&klrec,sizeof klrec));
}

gettrec(i)
{	return(getrec(tfid,i,&trec,sizeof trec));
}

getarec(i)
{	return(getrec(afid,i,&arec,sizeof arec));
}

getrec(fid,i,recadrs,recsize)	int *recadrs;
{	int n;

	if(n=seekrec(fid,i,recsize))return(n);
	switch(n=read(fid,recadrs,recsize))
	{	case -1 :	return(ERREAD);

		case 0 :	return(EOF);
	}
	if(n!= recsize)return(ERREAD);
	return(*recadrs);
}

seekrec(fid,i,recsize)
{	int offset;

	seek(fid,0,0);
	switch(recsize)
	{	case 4 :
			while(i>8191){seek(fid,32764,1); i=- 8191;}
			break;
		case 6 :
			while(i>5461){seek(fid,32766,1); i=- 5461;}
			break;
		case 72 :
			while(i>455){seek(fid,32760,1); i=- 455;}
	}
	offset=i*recsize;
	if(seek(fid,offset,1)== -1)return(ERSEEK);
	return(OK);
}

error(n)
{	switch(n)
	{	case EROPEN :	em(0); exit();

		case ERREAD :	em(1); break;

		case NOKEYW :	em(2); break;

		case NOREF :	em(3); break;

		case NOTITLE :	em(4); break;

		case NOABSTR :	em(5); break;

		case ARGS :	em(6); exit();

	}
}

em(n)
{	printf("%s\n",emsg[n]);
}
