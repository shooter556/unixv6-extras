#include "batch.h"
/*
 *	this program controls all the print output that comes of the
 *	line.  it identifies all jobs and queues them for printing.
 *
 *	filedescriptors -
 *		      0 - <- cyber
 *		      1	- work
 *		      2 - initiating terminal
 *		      3 - checkpoint file
 *
 *
 *						ian j. march '76
 *
 *
 *	qtypes-
 *		      0 - free
 *		      4 - print
 *
 */


#ifdef	QEI
#define	FIRSTPOS 18
#define	CHARSIZ	14
#define	CJN0	4
#define	CJN1	5
#define	CJN2	6
#define	CJNC	7
#define	SKIPWM	0511
#endif
#ifdef	NOS
#define	CJN0	4
#define	CJN1	5
#define	CJN2	6
#define	CJNC	7
#define	SKIPWM	0
#endif
#ifndef	QEI | NOS
#define	FIRSTPOS 20
#define	CHARSIZ	10
#define	CJN0	0
#define	CJN1	1
#define	CJN2	2
#define	CJNC	3
#define	SKIPWM	01200
#endif

#define	LOWER_C		/* allow lower case conversion and local files */
#ifdef	LOWER_C		
int	lowerflg;
int	localfile;
#endif


char m1[] "\n\n\nspl: file create error\n";
#ifdef	QEI
char m3[]
 "\377\011unix\377\026????????\377\015????????\377\015????????\377\047unix\n";
#endif


#ifdef	QEI
int	forfg;
#endif


#ifndef	NOS
struct quelt *jnget(ptr , ep)
  char *ptr, *ep;
{
	char jname[CJNC],x; register char *c; int register i;
	int pos,limit;
	struct quelt *r,*sp,*sr; register struct quelt *p;

	pos=sr=sp=0; c=ptr;
	limit = FIRSTPOS;
#ifdef	QEI
	if(((*c)&0377)!=0377) {
#endif
#ifndef	QEI
	while ( ((((i = *c++ & 0377)==0377) || (i==0376)) && c++) || (i != '\n') )	/* search for start of banner line */
		if ( c >= ep )  {
#endif
ferr:
		prints("spl: format error??\n");
#ifdef	QEI
		forfg++;
#endif
		i = 7;
		do jname[i-1] = "format?"[i-1]; while(--i);
		goto ufo;
	}
/*	{	/* TRACE */
/*		struct { int integ; };	/* TRACE */
/*		static char spaces[] "                                                    ";	/* TRACE */
/*		static char zeros[] "0000000000000000000000000000000000000000000000000000";	/* TRACE */
/*		/* TRACE */
/*		for ( i=0 ; (p.integ = c[i] & 0377) != '\n' ; i++ )	/* TRACE */
/*			if ( p.integ == 0376 )  write( 2 , zeros , c[++i] & 0377 );	/* TRACE */
/*			else if ( p.integ == 0377 )  write( 2 , spaces , c[++i] & 0377 );	/* TRACE */
/*			else write( 2 , c+i , 1 );	/* TRACE */
/*		write( 2 , c+i , 1 );	/* TRACE */
/*	}	/* TRACE */

	for(i=0;i<CJNC;)  {
	   switch((x= *c++)&0377){
		case ' ':	pos++; goto check;
		case 0376:	x='0'; pos=+*c++; goto l1;
		case 0377:	pos=+*c++;
		  check:	while(pos > limit+CHARSIZ) limit =+ CHARSIZ; break;
		default:	pos++;
		     l1:	if(pos>limit) {
					jname[i++]=x; limit=+CHARSIZ;
				}
	   }
	   if ( c >= ep )  goto ferr;
	}
#endif

#ifdef	NOS
char *jnget1( cp , ep )
  register char *cp;
  char *ep;
{
	static char job[] "\377\003**";
	register char *id = job;
	static char jname[CJNC];
	int found = 0;
	register i = 0;

  while ( cp < ep )
	if ( !found )  {
		if ( *cp++ != *id++ )
			id = job;
		else  if ( id >= &job[(sizeof job)-1] )
			found++;
	}else  if ( (jname[i++] = *cp) < '0' || (*cp++ > 'Z') || (i >= CJNC) )
		goto out;

  prints( "\nspl: format error?\n" );

out:
  while ( i < CJNC )
	jname[i++] = ' ';
  return( jname );
}

struct quelt *jnget( cp , ep )
{
	register char *jname = jnget1( cp , ep );
	register struct quelt *p, *r;
	int i;
#endif

	pp(qlock);	/* obtain exclusive use of the queues */
	r = &qhdr[atcyber];
	if( p=r->qlink )   {
	  do {
#ifdef	QEI
		if( jname[0] == p->jobn[0] ) 
		 if( jname[1] == p->jobn[1] )
		{
		    if( !p->cjobn[0] ) {
			sp=p;	sr=r;
		    }
#endif
		    if( jname[CJN0] == p->cjobn[0] ) 
		     if( jname[CJN1] == p->cjobn[1] ) 
		      if( jname[CJN2] == p->cjobn[2] )  {
			r->qlink = p->qlink;
		uchn:	p->qlink = 1;
			qhdr[atcyber].qnum--;
			vv(qlock); goto found;
		     }
#ifdef	QEI
		}
#endif
	  } while(p = (r=p)->qlink);

#ifdef	QEI
	  if(sp) {
		sr->qlink = (p=sp)->qlink;
		p->cjobn[0]=jname[CJN0];p->cjobn[1]=jname[CJN1];
		p->cjobn[2]=jname[CJN2];
		goto uchn;
	  }
#endif
	}
	vv(qlock);
ufo:
	p=qget(free);	p->origin = ogcyb|tycyb;		/* disposed jobs here */
	p->qtype = atcyber;	ncybjobs--;
	for(i=0;i<CJNC;i++)
		p->jobn[i]=p->cjobn[i-(CJNC-3)]=jname[i];
found:
	return(p);
}


main(argc,argv)
  char **argv; int argc;
{
	register n,j,*p; char *x; struct quelt *q; int *m2;
	char buff[1100] ;
	extern restart();
	extern fout;

	fout=2;

	if( local ) exit( 0 );	/* daveh */
	if(fstat(3,ccbuf) < 0) exit( 0 );
	signal( 1, restart );

	for(;;){
		if((j=crfnm())<0) {prints(m1);exit( 0 );}; /* allocate a new output file */

		while((n=read(0,buff,sizeof buff))<=0); /* banner page */
#ifdef	QEI
		forfg = 0;
#endif
		q = jnget(&buff[SKIPWM] , &buff[n]);
#ifdef	LOWER_C
		lowerflg = 0;
		localfile = 0;
#endif
		q->nofile[0] = j;
#ifdef	QEI
		if( forfg) {
#endif
			write(1,buff,n);
#ifdef	QEI
			goto flush;
		}

		buff[0202] = '\r';
		write(1,buff,0511);

	loop:	while(!(n=read(0,buff,sizeof buff)));
		for(j=1;j<n;j++)
		    if((buff[j-1]=='\n')&&(buff[j]=='\n')) goto found;
		goto loop;

	found:	n =- j; x= &buff[j];	p = q->jobn;	/* insert real name */
		for(j=0;j<4;j++)
		  m2[j+4]=m2[j+9]=m2[j+14]=p[j];
		m3[15]=m3[25]=m3[35]=' ';
		m3[7] = 026; m3[37] = 047;
		for(j=0;j<12;j++){
		  m3[7]++;m3[37]--;
		  write(1,m2,sizeof m3);
		}
		write(1,x,n);
#endif	QEI

flush:		while((n=read(0,buff,sizeof buff))>=0) {
#ifdef	LOWER_C
			n = lower( buff , n , q );
#endif
			if(n>0)
			while(write(1,buff,n) != n)
					sleep(30);
		}

		close(1);
		q->ndtime = dtime();
#ifndef	LOWER_C
		qput(q,print);
#endif
#ifdef	LOWER_C
		qput(q, localfile?free:print);
#endif
	}
}


#ifdef	LOWER_C

#define	BLOCK1		1
#define	BLOCK2		2
#define	LOWER		0400
#define	LOCALF		01000
#define	NAMEZ		64

char	localname[NAMEZ];


char	maptab[0200]
{
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
015,026,024,020,013,023,027,030,
011,012,007,005,016,006,017,010,
'{','|','}','~','%','^',001,002,
003,004,0140,037,032,014,033,031,
034,'a','b','c','d','e','f','g',
'h','i','j','k','l','m','n','o',
'p','q','r','s','t','u','v','w',
'x','y','z',021,035,022,036,025,
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
'?','?','?','?','?','?','?','?',
};


lower( buf , n , q )
  char *buf;
  struct quelt *q;
{
	register char *cp, *sp, *ep;
	int i;
	static count, state;

  if ( lowerflg < 3 )  {	/* first 3 blocks */
	cp = buf;
	sp = cp;
	ep = cp + n;
	if ( !lowerflg )   state = 0;	/* first block */

	while ( cp < ep )  switch ( state )  {

	 case 0:  case 1:  case 2:  case 3:
	 case 4:  case 5:  case 6:  case 7:
		if ( *cp++ != '?' )  state = 0;
		else  state++;
		continue;

	 case 8:
		if ( *cp++ == '?' )  continue;
		if ( *(cp-1) != '<' )  state = 0;
		else  {  state++;  count = 0;  }
		continue;

	 case 9:
		if ( *cp++ == '>' )  state++;
		else
			if ( count >= NAMEZ )  state = 0;
			else
				localname[count++] = *(cp-1)|040;
		continue;

	 case 10:
		state = 0;
		if ( *cp++ == 'L' )  {
			lowerflg =| LOWER;
			cp++;
		}
		if ( count )  {
			char statbuf[36];

			localname[count] = '\0';
			if ( stat(localname,statbuf) >= 0 || (i = creat(localname,0666)) < 0 )  continue;
			else  {
				lowerflg =| LOCALF;
				localfile++;
				close( 1 );
				nunlink( q->nofile[0] );
				q->nofile[0] = 0;
				dup( i );
				close( i );
			}
		}
		while( cp < ep )  *sp++ = *cp++;
		n = sp - buf;
		goto out;
	}

out:
	lowerflg++;
  }

  if ( lowerflg & LOWER )  {
	cp = buf;
	ep = cp+n-1;
	sp = cp;
	while ( cp < ep )
		if ( *cp++ == '%' )  *sp++ = maptab[*cp++];
		else
			*sp++ = *(cp-1);
	return( sp - buf );
  }

  return( n );
}

#endif	LOWER_C




restart()	/* on E/I crashes */
{
	signal( 1, restart );
#ifdef	LOWER_C
	if ( localfile )
		unlink( localname );
	localfile++;
#endif	LOWER_C
}
