#define	entrysize	60

char copr[] "Copyright (c) 1975 Michael D. Tilson";

/* This program allocates and deallocates items in a  file */
/* the items are intended to be the names of mag tapes
 * the format of each line of the file is as follows:
 *	tapename:ownername:comments
 *
 */

int tfile,passwd,newtfile;
int ts,fs,ns,newlen;
char tbuf[6000];
char newentry[entrysize*3+4];
int allocating;
char uname[entrysize+1];
char tname[entrysize+1];

main(argc,argv)
	int argc;
char **argv;
{
	register int uid;
	int tvec[2];
	signal(1,1);
	signal(2,1);
	signal(3,1);
	/* want this to run as user bin, but be callable by super-user
	 * because the single use locking provided by creat does not
	 * work if  you are the super-user, this program sets its real and
	 * effective uid to be bin (3).
	 */
	uid=getuid()&0377;
	setuid(3);
	lock();
	allocating = 1;
	if(argc>1 && argv[1][0]=='-')
	{
		switch(argv[1][1])
		{
		case 'f':
			allocating = 0;
			if(--argc == 1)
				quit("Nothing to free");
			argv++;
			break;

		case 'p':
			if(argc>2)
				quit("Arg count");
			unlock();
			time(tvec);
			printf("Tape\tOwner\tComments\t\t%s\n\n",ctime(tvec));
			if(argv[1][2]=='u')
				execl("/bin/sh","sh","-c",
				"sort -t: +1 /usr/adm/tfile | tr \":\" \"\t\"",0);
			else
				execl("/bin/sh","sh","-c","sort -t: /usr/adm/tfile |  tr \":\" \"\t\"",0);
			quit("Can't exec sort");
		}
	}
	if(argc>2)
		quit("Arg count");
	if((tfile=open("/usr/adm/tfile",2))<0)
		quit("Can't open tape file");
	if((passwd=open("/etc/passwd",0))<0)
		quit("Can't open passwd file");
	getname(uid,uname);
	if((ts=read(tfile,tbuf,sizeof tbuf))==sizeof tbuf)
		quit("Tape file too big");
	if (ts<0)
		quit("Can't read tape file");
	tbuf[ts] = '\0';
	if(argc==2)
		update(argv[1]);
	else
		newtape();
	if((newtfile=creat("/usr/adm/newtfile",0644))<0)
		quit("Can't create new tape file");
	if(fs)
		if(write(newtfile,tbuf,fs) != fs)
			quit("Write error on new tape file");
	if(write(newtfile,newentry,newlen) != newlen)
		quit("Write error on new tape file");
	if(write(newtfile,&tbuf[ns],ts-ns)!= ts-ns)
		quit("Write error on new tape file");
	if(unlink("/usr/adm/tfile")<0)
	{
		write(2,"Disaster--unlink error\nInform a system hack\n",43);
		exit(); 	/* leave lock file still there to prevent further damage */
	}
	if(link("/usr/adm/newtfile","/usr/adm/tfile")<0)
	{
		write(2,"Disaster--link error\nInform a system hack\n",41);
		exit();
	}
	sync();
	unlink("/usr/adm/newtfile");
	if(allocating)
		printf("tape:  %s  Allocated to:  %s\n",tname,uname);
	else
		printf("tape %s is now free\n",tname);
	quit(-1);
}

lock()
{
	if(open("/usr/adm/tfilelck",0)>=0 || creat("/usr/adm/tfilelck",0444)<0)
	{
		write(2,"Tape file in use, try later.\n",29);
		exit();
	}
}

quit(qstr)
	char *qstr;
{
	if(qstr != -1)
	{
		write(2,qstr,strlen(qstr));
		write(2,"\n",1);
	}
	unlock();
	exit();
}

unlock()
{
	if(unlink("/usr/adm/tfilelck",0)<0)
		write(2,"Unlock error--tape file may not be available--call a system hack\n",65);
}

strlen(str)
	char *str;
{
	register int i;
	i = 0;
	while(*str++)
		i++;
	return(i);
}

getname(uid, buf)
int uid;
char buf[];
{
	int j, c, n, i;

	struct{
		int fdes;
		int nleft;
		char *nextp;
		char buffer[512];
		} inf;

	inf.fdes = passwd;
	seek(inf.fdes, 0, 0);
	inf.nleft = 0;
	do {
		i = 0;
		j = 0;
		n = 0;
		while((c=getc(&inf)) != '\n') {
			if (c<0)
				quit("Who are you, anyway?");
			if (c==':') {
				j++;
				c = '0';
			}
			if (j==0)
				buf[i++] = c;
			if (j==2)
				n = n*10 + c - '0';
		}
	} while (n != uid);
	buf[i++] = '\0';
}

update(name)
	char *name;
{
	register char c;
	register int i;
	char *thisname, *olduser;


	for(i=0; tname[i] = *name++; i++);
	fs = 0;
	ns = 0;
	for(;;)
	{
		fs = ns;
		thisname = getitem();
		if(eq(tname,thisname))
		{
			for(newlen=0; tname[newlen] != 0; newlen++)
				if((newentry[newlen] = tname[newlen]) == ':')
					quit("Bad tape name");
			newentry[newlen++] = ':';
			olduser = getitem();
			if(eq(olduser,uname) || eq(uname,"root") || eq(olduser,""))
			{
				if(allocating)
				{
					for(i=0; uname[i]!=0; i++)
						newentry[newlen++] = uname[i];
					if(eq(olduser,uname))
						printf("Enter new ");
				}
				else if(eq(olduser,""))
					quit("Tape is already free");
				newentry[newlen++] = ':';
			}
			else
				quit("Tape is not yours");
			if(allocating)
			{
				printf("comments:  ");
				for(i=0; (c=getchar())!= '\n' && c!='\0' && c!= -1 && c!=':'; i++)
					newentry[newlen++] = c;
			}
			newentry[newlen++] = '\n';
			getitem();
			return;
		}
		getitem();
		getitem();
	}
}

newtape()
{
	register char *ntape;
	register int i;
	char ntp[entrysize+1];
	fs = 0;
	ns = 0;
	for(;;)
	{
		ntape=getitem();
		for(i=0; ntp[i] = *ntape++; i++);
		if(eq(getitem(),""))
		{
			update(ntp);
			return;
		}
		getitem();
	}
}

getitem()
{
	static char item[entrysize];
	register int i;
	register char c;
	for(i=0; i<entrysize; i++)
	{
		if((c=tbuf[ns++]) == ':' || c=='\n')
		{
			item[i] = '\0';
			return(item);
		}
		if(c=='\0')
			quit("Search fails:  No tape");
		item[i] = c;
	}
	quit("Search fails:  No tape");
}

eq(str1,str2)
	char *str1, *str2;
{
	while(*str1 == *str2)
		if(*str1++ == '\0')
			return(1);
		else
			str2++;
	return(0);
}
