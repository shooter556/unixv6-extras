#include	"docmod.h"

/* doc.c */

int	cmode	0;	/*command mode - clear for conversational*/

main(argc,argv)	int argc; char **argv;
{	register int i;

	if(argc!=1)
	{	if(argc!=2)error(ARGS);
		i=0;
		while(buff[i]=argv[1][i])i++;
		buff[i]='\n';
		cmode=1;
	}

	if(openf(0))error(EROPEN);

	setexit();
	signal(2,reset);
	printf("\n");

	for(;;)
	{	if(cmode)if(argc++ == 3)dmexit();
		switch(doccsi(i))
		{	case LK :	listkw(); continue;

			case LT :	listtl(); continue;

			case LA :	listall(); continue;

			case KEYWORD :	if((i=kwfind(inkw))<0)error(i);
					else prchain();
					continue;

			case TITLE :	switch(i=gettrec(intitle))

					{	case ERSEEK :
						case ERREAD :
								error(i); continue;
						case EOF :
						case FREE :	error(NOTITLE); continue;

						default :	prabstract(); continue;
					}

			case HELP :	help();
		}
	}
}

openf(mode)
{	if((kwfid=open("/usr/pub/keywords",mode))<1)return(EROPEN);
	if((klfid=open("/usr/pub/keylinks",mode))<1)return(EROPEN);
	if((tfid=open("/usr/pub/titles",mode))<1)return(EROPEN);
	if((afid=open("/usr/pub/abstracts",mode))<1)return(EROPEN);
	return(OK);
}

doccsi(i)
{	char c;
	register int j;

	if(cmode==0)
		for(i=0;;i++)
		{	if(read(0,&c,1)==0)dmexit();
			if((buff[i]=c)=='\n')break;
		}
	if(i==0)return(HELP);
	if(buff[0]>='0' && buff[0]<='9')
	{	scanf(-1,buff,"%d",&intitle);
		return(TITLE);
	}
	else if(i==2 && buff[0]=='l')
		switch(buff[1])
		{	case 'k' :	return(LK);

			case 't' :	return(LT);

			case 'a' :	return(LA);
		}
	for(j=0;j<4;j++)
	{	if(j==i)break;
		inkw[j]=buff[j];
	}
	for(j=i;j<4;j++)inkw[j]=BLANK;
	return(KEYWORD);
}
help()
{	printf("commands :\n\n");
	printf("\tlk = list keywords\n\tlt = list titles\n\tla = list all\n");
	printf("\t n = print abstract for title n\n");
	printf("\t**** any other input is taken as a keyword\n");
}

kwfind(kw)	char kw[];
{	register int i,j;

	for(i=0;;i++)
	{	switch(getkwrec(i))
		{	case ERREAD :	return(ERREAD);
			case ERSEEK :	return(ERSEEK);
			case EOF :	return(NOKEYW);
			case FREE :	continue;
		}
		for(j=0;j<4;j++)
			if(kw[j]!=kwrec.keyword[j])goto no;
		return(i);
	no:	continue;
	}
}

listkw()
{	register int i,j;

	i=1;

	for(j=0;;j++)
	{	switch(getkwrec(j))
		{	case ERSEEK :	error(ERSEEK); return;

			case ERREAD :	error(ERREAD); return;

			case EOF :	printf("\n"); return;

			case FREE :	continue;

			default :	printf("%s ",kwrec.keyword);
					if(i++!= 10)continue;
					i=1;
					printf("\n");
		}
	}
}

listtl()
{	register int j;

	for(j=0;;j++)
	{	switch(gettrec(j))
		{	case ERSEEK:	error(ERSEEK);return;

			case ERREAD :	error(ERREAD); return;

			case EOF :	return;

			case FREE :	continue;

			default :	printf("%3d : %s\n",j,trec.title);
		}
	}
}

listall()
{	register int i;

	printf("KEYWORDS:\n********\n\n");
	listkw();
	printf("\fTITLES:\n*******\n\n");
	listtl();
	printf("\fABSTRACTS:\n**********\n\n");
	for(i=0;;i++)
	{	switch(gettrec(i))
		{	case ERSEEK :	error(ERSEEK); return;

			case ERREAD :	error(ERREAD); return;

			case EOF :	return;

			case FREE :	continue;

			default :	prabstract();
					underline();
		}
	}
}

prchain()
{	register int i;

	if(kwrec.klfp==LAST){error(NOREF); return;}

	for(i=kwrec.klfp;;i=klrec.nklfp)
	{	if(getklrec(i)<LAST || gettrec(klrec.tp)<LAST)
		{	error(ERREAD);
			return;
		}
		printf("%3d : %s\n",klrec.tp,trec.title);
		if(klrec.nklfp==LAST)return;
	}
}

prabstract()
{	register int i,j;

	printf("%s\n\n",trec.title);
	if(trec.afp==LAST){error(NOABSTR); return;}
	for(i=trec.afp;;i=arec.nafp)
	{	if(getarec(i)<LAST){error(ERREAD); return;}
		printf("%s\n",arec.text);
		if(arec.nafp==LAST)return;
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
	{	case EROPEN :	em(0); dmexit();

		case ERREAD :	em(1); break;

		case NOKEYW :	em(2); break;

		case NOREF :	em(3); break;

		case NOTITLE :	em(4); break;

		case NOABSTR :	em(5); break;

		case ARGS :	em(6); dmexit();

	}
}

em(n)
{	printf("%s\n",emsg[n]);
}

underline()
{	printf("\n********************************************************************\n\n");
}

dmexit()
{	cexit();
}
