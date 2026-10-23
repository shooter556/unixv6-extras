int n1,n2,n3,n4;
char dline[150];
char ffile[50] "../updat/";
char *file { &ffile[9] };

int xg 0;
int lg 0;

int ifbuf[259], inbuf[259] ;

ung()
{
	xg++;
}

g()
{
	char *q,*p;

	if( xg ) {
		xg = 0;
		return lg;
	}
	n1=n2=n3=n4=0;
	if( getline( inbuf , dline ) == 0 ) { lg = 0 ; goto ret; }

	if( cmp("----",dline) ) {
		q = dline;
		while( *q++ != ' ');
		p = file;
		while( (*p++ = *q++) != '\n');
		*--p = 0;
		{ lg = 1 ; goto ret; }
	}
	if( cmp("---\n",dline ) ) { lg = 7 ; goto ret; }
	if( cmp("< ",dline) ) { lg = 6 ; goto ret; }
	if( cmp("> ",dline) ) { lg = 5 ; goto ret; }
	if( (dline[0]>'9') || (dline[0]<'0') ) { lg = 8 ; goto ret; }
	p = dline;
	n1 = gnumb( &p );
	if( *p == ',' ) {
		p++;
		n2 = gnumb( &p );
	} else n2 = n1;
	q = p;
	p++;
	n3 = gnumb( &p );
	if( *p == ',' ) {
		p++;
		n4 = gnumb( &p );
	} else n4 = n3;
	switch( *q ) {
	case 'a':
		{ lg = 2 ; goto ret; }
	case 'd':
		{ lg = 3 ; goto ret; }
	case 'c':
		{ lg = 4 ; goto ret; }
	}
	lg = 8;
 ret:
	printf("%d : %s",lg,dline);
	return lg;
}


int	ilin 0;
char iline[200];


#define ifd ifbuf[0]
int ofd -1;

main()
{
	register n;
	extern int fout;

	fout = dup(1);
	if( g()==1 ) goto case1;
	printf(" eh no file name %d\n",lg);
	xx(1);
 for(;;)
 {
	switch( g() )
	{
    case 0:
		copy (0);
		xx(0);
    case 1:
		copy( 0 );
		close( ofd );
		close( ifd );
	case1:
		ofd = creat(ffile,0600);
		if( ofd < 0 ) {
			printf("%s creat error\n",ffile);
			xx();
		}
		ilin = 0;
		if ( fopen( file, ifbuf) < 0 )
		{
			printf("%s not found\n",file);
			ifd = -1;	/* dud file descr */
		}
		break;
    case 2:
		copy ( n1 );
	case2:	while( g()==5 ) prints(ofd,&dline[2]);
		ung();
		break;
    case 3:
		copy( n1-1 );
		skipc( n2 );
		break;
    case 4:
		copy( n1-1 );
		skipc( n2 );
		if( g() != 7 ) {
			printf("bad change format no --- %d\n",lg);
			xx(1);
		}
		goto case2;
   default:
		printf("illegal code %d\n",lg);
		xx(1);
	}
 }
}

getline( fb , line )
char *fb;
char *line;
{
	register char *p;

	p = line;
	if( p==iline ) ilin++;
	while( (*p = getc( fb )) != -1)
	{
		if( *p == '\n' ) {
			*++p = 0;
			return 1;
		}
		p++;
	}
	return 0;
}

gnumb( p )
char **p;
{
	register n;
	register char *q;

	q = *p;
	n = 0;
	while( (*q<='9') && (*q>='0') ) n = n*10 + *q++ - '0';
	*p = q;
	return n;
}

cmp( s1 , s2 )
register char *s1,*s2;
{
	while( *s1 ) if( *s1++ != *s2++ ) return 0;
	return 1;
}

copy( n )
register int n;
{
	while( n != ilin ) {
		if( n && n<ilin ) {
			printf("n=%d ilin=%d\n",n,ilin);
			xx(1);
		}
		if( getline( ifbuf , iline )==0 ) {
			if( n == 0) return;
			printf("not enough input\n");
			xx(1);
		}
		prints( ofd , iline );
	}
}
skipc( n )
register int n;
{
	while( n != ilin ) {
		if( n<ilin ) {
			printf("n=%d ilin=%d\n",n,ilin);
			xx(1);
		}
		if( getline( ifbuf , iline )==0 ) {
			printf("not enough input\n");
			xx(1);
		}
		if( g()!=6) {
			printf("additions expected got %d\n",lg);
			xx(1);
		}
		if( cmp(iline,dline+2) == 0 ) {
			printf(" mismatch deletions %d \n",ilin);
			xx(1);
		}
	}
}


xx( n )
{
	flush();
	exit ( n );
}
