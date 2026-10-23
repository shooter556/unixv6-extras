/* words ... extract the words from the document */
/*  there are various glitches in this algorithm...
/*  in particular, it would be polite to recognize troff escapes */

int vflag 0;  /* verbose flag */
char	*t1, *t2, *t3;	/* temp file names */
char *s1 { "/usr/lib/spell1" };
char *s2 { "/usr/lib/spell2" };
char *s3 { "/usr/lib/spell3" };

main( argc, argv ) char *argv[]; {
  int c, fno;
  char sybuf[200];
  extern int cin, cout, getout();

  signal( 2, getout );   /* catch interrupts */
	t1 = mktemp("saXXXXX");
	t2 = mktemp("/tmp/sbXXXXX");
	t3 = mktemp("/tmp/scXXXXX");

  if( (cout=copen( t1, 'w' )) < 0) {
    printf( 2, "cannot open tempfile\n" );
    getout();
    }

  if( argv[1][0] == '-' ){ /* verbose option */
    vflag = 1;
    ++argv;
    --argc;
    }

  fno = 0;

  filebeg:  /* beginning of new input file */

  if( ++fno > 1 && cin != 0 ) cclose(cin);
  if( fno >= argc && fno>1 ) goto proceed;  /* get on with the rest */
  if( argc <= 1 ) cin = 0; /* use standard input */
  else if( (cin=copen( argv[fno], 'r' )) < 0 ){
    printf( 2, "cannot open %s\n", argv[fno] );
    goto filebeg;
    }

linebeg:  /* beginning of a line; check for roff control */

  c = getchar();
  if( c == '.' || c == '\'' ){ /* delete the line */
    while( getchar() != '\n' )   ;
    goto linebeg;  /* try the next line */
    }

  check:
  while( brkch(c) ){  /* delete break characters */

	if(c == '\\') {
		c = getchar();
		if(c == 'f' || c == '*')
			c = getchar();
	}
    if( c == '\n' ) goto linebeg;
    if( c == '\0' ) goto filebeg;
    c = getchar();
    }

  /* we have the beginning of a word now */

  wloop:
  while( !brkch(c) ){
    putchar(c|' ');
    c = getchar();
    }
	if(c == '\\') {
		c = getchar();
		if(c == 'f' || c == '*')
			c = getchar();
		c = getchar();
		goto wloop;
	}
  if( c == '-' ) {
    c = getchar();
    if( c == '\n' ) {
      c = getchar();
      while( c == ' ' || c == '\t' || c == '\n' ) c = getchar();
      goto wloop;
      }
    putchar( '\n' );
    goto check;
    }
  if( c == '\'' ) {
    c = getchar();
    goto wloop;
    }
  if( c == '' ) { /* backspace */
    c = getchar();
    c = getchar();
    goto wloop;
    }

  /* end of word */

  putchar( '\n' );
  goto check;

  proceed:  /* do the rest of the job */

  cclose(cout);
	printf(-1, sybuf, "sort -d -u %s -o %s", t1, t1);
	system(sybuf);
	printf(-1, sybuf, "comm -23 %s /usr/lib/w2006 >%s", t1, t2);
	system(sybuf);
	printf(-1, sybuf, "%s <%s >%s", s1, t2, t1);
	system(sybuf);
	printf(-1, sybuf, "sort -d %s -o %s", t1, t1);
	system(sybuf);
	printf(-1, sybuf, "%s <%s >%s", s2, t1, t3);
	system(sybuf);
	printf(-1, sybuf, "sort -d %s -o %s", t3, t3);
	system(sybuf);
	printf(-1, sybuf, "%s %s %s <%s", s3, t2, vflag?"-v":"", t3);
	system(sybuf);

  getout();
  }

brkch(c){ /* is c a break character */
  if( c>='a' && c <='z' ) return(0);
  if( c>='A' && c<='Z' ) return(0);
  return(1);
  }

getout(){ /* remove temps and stop */

	unlink( t1 );
	unlink( t2 );
	unlink( t3 );
	cexit(0);
	}
