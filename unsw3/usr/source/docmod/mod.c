#include	"docmod.h"

int	tempfile;
int	freew	FREE;
int	delete;		/*set for title delete : clear otherwise*/
char	infile[41];
char	filename[14];
struct arec	arecbuff;

/* mod.c */

main(argc,argv)	int argc; char **argv;
{	register int i;

	if(argc!=1)error(ARGS);

	while(openf(2))error(EROPEN);

	setexit();
	signal(2,reset);
	printf("\n");

	for(;;)
	{	switch(modcsi(i))
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

			case EDIT :	switch(i=edit())
					{	case EOF :	return(NOTITLE);

						case OK :	continue;

						default :	error(i); continue;
					}

			case INSERT :	if(i=insert())error(i);
					continue;

			case INSKW :	if(i=inskw())error(i); continue;

			case LINK :	if(i=link())error(i); continue;

			case DELT :	delete=1; if(i=deletet(intitle))error(i);
					delete=0; continue;

			case DELKW :	if(i=delkw())error(i); continue;

			case KILINK :	if((i=kwfind(inkw))<0)error(i);
					else if(kilink(i))error(i);
					continue;

			case LISTKL :	if((i=listkl())<0)error(i);
					continue;

			case REPLACE :	if(i=replace())error(i);
					continue;

			case TWRITE :	if(i=twrite())error(i);
					continue;

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

modcsi(i)
{	char c;
	register int j, k;

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
	if(buff[0]=='-')
		switch(buff[1])
		{	case '?' :
			case 'w' :
			case 'e' :	scanf(-1,&buff[2],"%d",&intitle);
					if(buff[1]=='?')return(LISTKL);
					if(buff[1]=='w')return(TWRITE);
					return(EDIT);

			case 'i' :	j=2;
					while(buff[j]==BLANK)j++;
					scanf(-1,&buff[j],"%s",infile);
					return(INSERT);

			case 'k' :	j=2;
					while(buff[j]==BLANK)j++;
					for(k=0;k<4;k++)
					  if((inkw[k]=buff[j++])=='\n')break;
					for(j=k;j<4;j++)inkw[j]=BLANK;
					return(INSKW);

			case 'x' :
			case 'l' :	j=2;
					while(buff[j]==BLANK)j++;
					for(k=0;k<4;k++)
					  if((inkw[k]=buff[j++])==BLANK)goto out;
					while(buff[j++]!= BLANK);
			out:		scanf(-1,&buff[j],"%d",&intitle);
					for(j=k;j<4;j++)inkw[j]=BLANK;
					if(buff[1]=='x')return(KILINK);
					return(LINK);

			case 'd' :	j=2;
					while(buff[j]==BLANK)j++;
					if(buff[j]>='0' && buff[j]<='9')
					{	scanf(-1,&buff[j],"%d",&intitle);
						return(DELT);
					}
					else
					{	for(k=0;k<4;k++)
						  if((inkw[k]=buff[j++])=='\n')break;
						for(j=k;j<4;j++)inkw[j]=BLANK;
						return(DELKW);
					}

			case 'r' :	j=2;
					while(buff[j]==BLANK)j++;
					for(k=0;k<14;k++)
						if((filename[k]=buff[j++])==BLANK)break;
					if(k==14){ error(ERFNAM); return(ERFNAM); }
					while(buff[j]==BLANK)j++;
					filename[k]=0;
					scanf(-1,&buff[j],"%d",&intitle);
					return(REPLACE);

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
	printf("\t-k string = insert keyword 'string'\n");
	printf("\t-l string n = link keyword 'string' to title 'n'\n");
	printf("\t-x string n = kill link between keyword 'string' & title 'n'\n");
	printf("\t-d string or n = delete keyword 'string' or title 'n'\n");
	printf("\t-e n = edit title 'n'\n");
	printf("\t-w n = write title 'n' & abstract to file 'n'\n");
	printf("\t-? n = list keywords linked to title 'n'\n");
	printf("\t-i filename = insert title & abstract from file\n");
	printf("\t-r filename n = replace title & abstract 'n' with file\n");
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

listkl()
{	register int j,k;

	for(k=0;;k++)
		switch(getkwrec(k))
		{	case ERREAD :	return(ERREAD);
			case ERSEEK :	return(ERSEEK);
			case EOF :	return(OK);
			case LAST :
			case FREE :	break;
			default :
				for(j=kwrec.klfp;;j=klrec.nklfp)
				{	if(getklrec(j)<LAST)return(ERREAD);
					if(klrec.tp==intitle)
					{	printf("%s\n",kwrec.keyword);
						break;
					}
					if(klrec.nklfp==LAST)break;
				}
		}
}

twrite()
{	char t[6];
	register int i;

	printf(-1,t,"%d",intitle);
	if((i=gettrec(intitle))<LAST)return(i);
	if((tempfile=creat(t,0777))<1)return(EROPEN);
	if(i=saveta())return(i);
	return(OK);
}

edit()
{	int pid;
	register int i;

	if((tempfile=creat("tempfile",0777))<1)return(EROPEN);
	if((i=gettrec(intitle))<LAST)return(i);
	if(i=saveta())return(i);
	cclose(tempfile);
	pid=0;
	if((pid=fork())== -1)return(ERFORK);
	if(pid)
	{	signal(2,1);
		while(wait()!=pid);
		signal(2,reset);
	}
	else
	{	execl("/bin/ed","em","-e","tempfile",0);
		return(EREXEC);
	}
	if((tempfile=open("tempfile",2))<1)return(EROPEN);
	if(i=replta())return(i);
	return(OK);
}

insert()
{	register int i, j;

	if((tempfile=open(infile,0))<1)return(EROPEN);
	if((i=sfreet(-1))<0)return(i);
	j=insertta(i);
	if((j&&delete)==0)printf("inserted as title no. %d\n",i);
	cclose(tempfile);
	return(j);
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

sfreekw(n)
{	register int i,j;
	int ibuff;

	for(j=n+1;;j++)
	{	seekkwrec(j);
		if((i=read(kwfid,&ibuff,2))==0)return(j);
		if(i!=2)return(ERREAD);
		if(ibuff==FREE)return(j);
	}
}

sfreekl(n)
{	register int i,j;
	int ibuff;

	for(j=n+1;;j++)
	{	seekklrec(j);
		if((i=read(klfid,&ibuff,2))==0)return(j);
		if(i!=2)return(ERREAD);
		if(ibuff==FREE)return(j);
	}
}

sfreet(n)
{	register int i,j;
	int ibuff;

	for(j=n+1;;j++)
	{	seektrec(j);
		if((i=read(tfid,&ibuff,2))==0)return(j);
		if(i!=2)return(ERREAD);
		if(ibuff==FREE)return(j);
	}
}

sfreea(n)
{	register int i,j;
	int ibuff;

	for(j=n+1;;j++)
	{	seekarec(j);
		if((i=read(afid,&ibuff,2))==0)return(j);
		if(i!=2)return(ERREAD);
		if(ibuff==FREE)return(j);
	}
}

seekkwrec(n)
{	return(seekrec(kwfid,n,sizeof kwrec));
}

seekklrec(n)
{	return(seekrec(klfid,n,sizeof klrec));
}

seektrec(n)
{	return(seekrec(tfid,n,sizeof trec));
}

seekarec(n)
{	return(seekrec(afid,n,sizeof arec));
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


delkw()
{	register int i;

	if((i=kwfind(inkw))<0)return(i);
	seek(kwfid,-6,1);
	write(kwfid,&freew,2);
	if(kwrec.klfp!=LAST)
		for(i=kwrec.klfp;;i=klrec.nklfp)
		{	if(getklrec(i)<LAST)return(ERREAD);
			seek(klfid,-4,1);
			write(klfid,&freew,2);
			if(klrec.nklfp==LAST)break;
		}
	return(OK);
}

replace()
{	register int i;

	if((tempfile=open(filename,0))<0)return(EROPEN);
	if(i=deletet(intitle))return(i);
	if(i=insertta(intitle))return(i);
	return(OK);
}

link()
{	register i, j, k;

	if((i=kwfind(inkw))<0)return(i);
	if((j=gettrec(intitle))<LAST)return(j);
	if((j=sfreekl(-1))<0)return(j);
	klrec.nklfp=kwrec.klfp;
	klrec.tp=intitle;
	kwrec.klfp=j;
	if(k=seekklrec(j))return(j);
	if(k=wkl())return(k);
	if(k=seekkwrec(i))return(k);
	return(wkw());
}

inskw()
{	register int i, j;

	if(kwfind(inkw)>=0)return(OK);
	if((i=sfreekw(-1))<0)return(j);
	kwrec.klfp= LAST;
	for(j=0;j<4;j++)kwrec.keyword[j]=inkw[j];
	if(j=seekkwrec(i))return(j);
	return(wkw());
}

/* replaces title "intitle" & abstract with tempfile - deletes tempfile */

replta()
{	register int i;

	if(i=deletet(intitle))return(i);
	if(i=insertta(intitle))return(i);
	cclose(tempfile);
	unlink("tempfile");
	return(OK);
}

/* inserts title and abstract from tempfile at titleno "n" */
insertta(n)
{	int i, a, newa;
	register char *r1, *r2;

	seek(tempfile,0,0);
	if(getline(tempfile,trec.title))return(ERREAD);
	trec.afp= LAST;
	a= -1;
	arecbuff.nafp= LAST;
	for(;;)
	{	if(getline(tempfile,arecbuff.text)==0)
		{	newa=sfreea(a);
			if(trec.afp==LAST)trec.afp=newa;
			else
			{	arec.nafp=newa;
				seekarec(a);
				wabstr();
			}
			a=newa;
			r1= &arecbuff;	r2= &arec;
			for(i=0;i<sizeof arec;i++)*r2++= *r1++;
		}
		else
		{	if(a!= -1)
			{	seekarec(a);
				wabstr();
			}
			seektrec(n);
			wtitle();
			break;
		}
	}
	return(OK);
}

/* copies title & abstract for current title to tempfile */

saveta()
{	register int i,j;

	printf(tempfile,"%s\n",trec.title);
	if(trec.afp==LAST)return(OK);
	for(i=trec.afp;;i=arec.nafp)
	{	if(getarec(i)<LAST)return(ERREAD);
		printf(tempfile,"%s\n",arec.text);
		if(arec.nafp==LAST)return(OK);
	}
}

deletet(n)
{	register int i,j;

	if((i=gettrec(n))<LAST)return(i);
	seek(tfid,-sizeof trec,1);
	if(write(tfid,&freew,2)!=2)return(ERWRITE);
	if(delete)
	for(i=0;;i++)	/*kill links*/
	{	switch(getkwrec(i))
		{	case EOF :	goto fin;

			case FREE :	continue;

			case ERREAD :	return(ERREAD);

			default :	kilink(i); continue;
		}
		fin:	break;
	}
	if(trec.afp==LAST)return(OK);
	for(j=trec.afp;;j=arec.nafp)
	{	if(getarec(j)<LAST)return(ERREAD);
		seek(afid,-sizeof arec,1);
		if(write(afid,&freew,2)!=2)return(ERWRITE);
		if(arec.nafp==LAST)return(OK);
	}
}

kilink(j)	/*j=kwrec number*/
{	register int i, save;

	if(kwrec.klfp==LAST)return(NOREF);
	for(i=kwrec.klfp;;i=klrec.nklfp)
	{	if(getklrec(i)<LAST)return(ERREAD);
		if(klrec.tp==intitle)break;
		if(klrec.nklfp==LAST)return(NOREF);
		save=i;
	}
	if(i==kwrec.klfp)
	{	kwrec.klfp=klrec.nklfp;
		seekkwrec(j);
		if(wkw())return(ERWRITE);
		seekklrec(i);
		if(write(klfid,&freew,2)!=2)return(ERWRITE);
		return(OK);
	}
	else
	{	seekklrec(i);
		if(write(klfid,&freew,2)!=2)return(ERWRITE);
		seekklrec(save);
		if(write(klfid,&klrec.nklfp,2)!=2)return(ERWRITE);
		return(OK);
	}
}

getline(fid,bufadrs)
{	char c;
	register char *r1;
	register int i;

	r1=bufadrs;
	for(i=0;;i++)
	{	if(read(fid,&c,1)!= 1)return(ERREAD);
		if(i>68)--r1;
		if((*r1++ =c)=='\n')break;
	}
	*(--r1)=0;
	return(OK);
}

wkw()
{	if(write(kwfid,&kwrec,sizeof kwrec)!=sizeof kwrec)return(ERWRITE);
	return(OK);
}

wkl()
{	if(write(klfid,&klrec,sizeof klrec)!=sizeof klrec)return(ERWRITE);
	return(OK);
}

wtitle()
{	if(write(tfid,&trec,sizeof trec)!=sizeof trec)return(ERWRITE);
	return(OK);
}

wabstr()
{	if(write(afid,&arec,sizeof arec)!=sizeof arec)return(ERWRITE);
	return(OK);
}

error(n)
{	switch(n)
	{	case EROPEN :	em(0); break;

		case ERREAD :	em(1); break;

		case NOKEYW :	em(2); break;

		case NOREF :	em(3); break;

		case NOTITLE :	em(4); break;

		case NOABSTR :	em(5); break;

		case ARGS :	em(6); dmexit();

		case ERWRITE :	em(7); break;

		case ERSEEK :	em(8); break;

		case EREXEC :
		case ERFORK :	em(9); break;

		case ERFNAM :	em(10); break;

		default :	printf("error no. %d\n",n);
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
