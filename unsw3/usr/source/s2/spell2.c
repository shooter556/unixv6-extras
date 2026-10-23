#
int debug 0;
char dword[100], inword[100];
char insuff[15];
char innumb[10];
char inlev[10];

char dbuf[512];
char *dend { dbuf };

int infile, dfile;

main( argc, argv ) int argc; char *argv[];{

  register char *p, *q;
  register c;

  if( argc>1 ) infile = copen( argv[1], 'r', "" );
  else infile = 0;

  if( infile < 0 ){
    printf( 2, "%s ??\n", argv[1]);
    cexit(1);
    }

  /* use UNIX I/O on dictionary */
  if( argc>2 ) dfile = open( argv[2], 0 );
  else dfile = open( "/usr/dict/words", 0 );

  if( dfile<0 ){
    printf( 2, "no dictionary\n");
    cexit(1);
    }

  p = dbuf;
  for(;;){
    q = dword;
  
  more:
    while( p<dend ){
      if( (c= *p++) == '\n' ){
        *q = '\0';
        goto com;
        }
      *q++ = c;
      }
    /* refresh buffer and keep going */
    if( (dend = dbuf+read( dfile, dbuf, 512 ) ) <= dbuf ) cexit(1);
    p = dbuf;
    goto more;
  
    /* have a word; do comparisons */
  com:
  
    while( (c=comp()) >= 0 ){ /* dictionary entry is larger or equal; read more words */
      if( c == 0 ) wword(1);
      rword();
      }
    }


  }

rword(){
  if( scanf( infile, "%s\t%s\t%s\t%s\n", inword, insuff, innumb, inlev ) < 4 ) cexit(1);
  }
wword( ){
  printf("%s\t%s\t%s\n", innumb, inlev, insuff );
  }

comp(){
  register char *p1, *p2;
  register i;
  if( debug ) printf( 2, "%s\t%s\n", dword, inword );
  p1 = inword;
  p2 = dword;
  cmore:
  while( (i= *p2++ ) ){
    if( i>='A' && i<='Z' ) i =| ' ';
    else if( i < 'a' || i > 'z'  ) goto cmore;
    if( i =- *p1++ ) return( i );
    }
  return( - *p1 );
  }
