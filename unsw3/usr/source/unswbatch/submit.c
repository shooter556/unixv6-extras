#include  "batch.h"
/*
 *    this program enables users to enter jobs into the queues. it separates local
 *    jobs and cyber jobs queueing them for the local processor concerned
 *    and for jtd. local jobs
 *
 *    filedescriptors -
 *                  1 -  work
 *                  2 -  initiating terminal
 *                  3 -  checkpoint file
 *
 *
 *						chris m. with help from
 *                                           ian j.  march '76
 *
 *    qtypes-
 *                  0 - free.
 *                  1 - jobs for cyber.
 *                  n - local processors.
 */
#define  checkpointfile	"/spool/batch/.work/.checkpoint"
#define  workdir	"/spool/batch/.work"
#define   pfd       1
#define qescp 0276
#define qeor  0327
#define qeoi  0326
                              /********/
                              /********/
char m1[] "\n\n\n####  %: illegal job card \n\n";
char m2[] "\n\n\nsubmit: usage:  submit file ...\n\n";
char m3[] "\n\n\n####  unknown batch function\n\n";
char m4[] "\n\n\nsubmit: file create error\n\n";
char m6[] "\n\n\n####  next line too long\n\n";
char m7[] "\n\n\nsubmit: file lines too long\n\n";
char m8[] "\n\n\nsubmit: batch not active\n\n";

int ibuf[259];
int	eof	0;		int longline 0;

			/************
			 *
			 * getcard
			 *
			 * reads a line and pads it out to 80 characters
			 *
			 ************/
getcard(card)
char *card;
{
	register char  *c;
	register int    i;
	i = 81;
	c = card;
	while( --i > 0 )
	{
		if(((*c = getc(ibuf)) == '\n') || (eof = (*c <= 0))) goto  eoln;
		if( *c == '\t' ){	/* daveh. expand tabs */
			do	*c++ = ' ';
			while( (81 - i--) &07 );
				/* last column blanked is mult of 8 */
			i++;
		}
		else c++;
	}
	if( (*c = getc(ibuf)) != '\n') 
	{
		longline++;
		write(pfd,m6,sizeof m6);
		return(81);
	}
  eoln:	for( ;i>0; i--) *c++ = ' ';
	*c = '\n';
	if( !eof ) return(81);
	return(0);
}
main(argc,argv)
char **argv; int argc;
{     register struct quelt *q;  int register n; char register *card;	int qtp;
       char buffer[2+81*12];     struct proc *p;   int    *buffer0;
	int i;
       /*
        *
        */
	if( actflag!=0 ) { 
		write(2,m8,sizeof m8);
		exit(1);
	}
	open(checkpointfile,2);	/*  always    3   */
	if(argc < 2)
	{
		write(2,m2,sizeof m2);
		exit(1);
	}
	buffer0 = buffer;
	for( i=1; i<argc; i++ ){
		card = &buffer[2];
		if( fopen(argv[i],ibuf) < 0)
		{
			prints(argv[i]);
			prints(": file not found\n");
			continue;
		}
		if( fork() ) { wait(buffer); continue; } /* parent */
		chdir(workdir);
		if( !getcard(card) ){		/* child */
			prints(argv[i]);
			prints(": empty file\n");
			continue;
		}
               q= qget(free); /* proceed when queue element obtained  */
		q->ndfile = q->nofile[0] = 0;
		close(pfd);
               if((q->npfile = crfnm())<0) { prints(m4); exit(1); };     /* pfd opened */
	loop2:	q->rdtime=dtime();
               if(card[0]=='%') {      
   /* local */ for(n=0;n<8;n++) q->jobn[n]= card[n+1];
		q->origin = ogterm|tyloc;
               if(!getcard(card)) goto loop2;
               p = &procs[0];
               while(p->pfile) 
                       if( p->pdesc && card[1]==p->pdesc[0] &&
                           card[2]==p->pdesc[1] && card[3]==p->pdesc[2] )
                               { qtp= p->pqtype;  goto found; }
			else p++;
		 write(pfd,card,81);
		 write(pfd,m3,sizeof m3);
                 goto flush;
      found:   for(;;) {
                       if(!getcard(card)){ q->ndfile=0; goto enque;}
                       if(card[0]==022 || card[0]==006) break;  else write(pfd,card,81);
               }
               close(pfd);
               if((q->ndfile=crfnm())<0) { prints(m4); exit(1); };       /* allocate and create data file */
     swoosh:   while( getcard(card) ) write(pfd,card,81);
               } else {
   /* cyber */ qtp=forcyber;  card = &buffer[2];
               if(m1[9]=vkjk(q->jobn,card)) {
                       write(pfd,card,81); 	m1[9] =| '0';
		       write(2,m1,sizeof m1); write(pfd,m1,sizeof m1);
                       goto flush ;
		}
		q->origin = ogterm|tycyb;
		card=+80;
		while( getcard(card)) {
			if( card[0] == 022) {
				card[0] = qescp;
				card[1] = qeor ;
				card[80]= ' '  ;
				card++;
			}
			else if( card[0] == 006 ){
				card[0] = qescp;
				card[1] = qeor;
				card[2] = '*';
				card[3] = '*';
				card[80] = ' ';
				card++;
			}
			card =+80;
			if( card >= &buffer[962] ) {
				*buffer0 = card-&buffer[2];
				write(pfd,buffer,1024);
				card = &buffer[2];
			}
		}
		*card++ = qescp;  *card++ = qeoi;
		for(n=0;n<79;n++)  *card++ = ' ';
		*buffer0 = (card - &buffer[2] );
		write(pfd,buffer,2+*buffer0);
	       }
     enque:	close(pfd);
		q->jobn[7]=0;
		if( longline )
		{
			qtp = print;
			q-> origin = ogterm | tyerr;
			q-> nofile[0] = q-> npfile;
			q-> npfile = 0;
			write(2,m7,sizeof m7);
		}
               qput(q,qtp);
		prints(argv[i]);
		prints( qtp==print ? ": flushed\n"
					: ": submitted\n" );
		close( ibuf[0] );
		continue;
	flush:	qtp=print;	q->origin = ogterm|tyerr;
	      	q->nofile[0]=q->npfile; q->npfile=0; goto swoosh;
	}
}
/*
 *	validate kronos job kard
 *
 *	errors:
 *		1) job name too long.
 *		2) bad delimiter following job name.
 *		3) time 
 *		4) cm
 *		5) prty
 *		6) rubbish.
 *		7) delimiter.
 *		8) dayfile.
 *		9) too many cyber jobs, or no cyber jobs allowed.
 */
	char *str,c;
	int *tab[6] {  017,5,  077770,3,  0120000,4   };
#define nextchar while((c= *(str++))==' '); if(c=='\n') return(6)
octal(num)
char *num;
{
	char register *m ,*flg;	
	m=0;
	flg=1;
	for(;;)
	{
		nextchar;
		if((c < '0') || (c > '7'))  return((!m)&flg);
		m = (m<<3) + (c&07);flg=0;
		if(m>num) return(6);
	}
}
vkjk(nm,string)
char *string,*nm;
{
	register int n, *x;	register char *nmp;

	if( nocyb ) return(9);
	if( --ncybjobs <= 0 ) { ncybjobs++; return(9); }
	str = string;
	n=0;	nmp=nm;
 	x=nmp; x[0]=x[1]=x[2]=x[3]=020040; /* a word of spaces */
 again:	nextchar;
	if((('a'<=c) && (c<='z')) || ((('0'<=c) && (c<='9'))&&(n!=0))){
	     if( c>'9' ) c=& ~('a'-'A');
	     if(++n<=7) { *nmp++=c; goto again; } else return(1);
	}
	if( (c!='(') && (c!=',') )
	      if(c=='.') return(n==0); else return(2);
	  else if( n==0 ) return(1);
	n=0;
	nextchar;
	if(('a'<=c)&&(c<='z'))
	     for(;;)
	     {
		switch(c)
		{
		case 't':	if((n&1) || octal(tab[2])) return(3);
				n=|1;
				break;
		case 'c':	nextchar;
				if((n&2)||(c!='m')||(octal(tab[4]))) return(4);
				n=|2;
				break;
		case 'p':	if((n&4) || octal(tab[0])) return(5);
				n=|4;
				break;
		case 'd':	if((n&8) || octal(02000)) return(8);
				n=|8;
				break;
		default:	return(6);
		}
		if((c==')') || (c=='.')) return(0);
			  else if(c!=',') return(7);
		nextchar;
	     }
	else {
		str--;
		for(n=0;n<6;n=+2)
		{
		if(octal(tab[n])) return(tab[n+1]);
		if((c==')') || (c=='.')) return(0);
			  else if(c!=',') return(7);
		}
	     }
}
