char word[100], suff[15];
int wdfile, sfile, numb, innumb, inlev;
int vflag;

main( argc, argv ) int argc; char *argv[]; {
  wdfile = copen( argv[1], 'r' );
  sfile = 0;  /* standard input */
  if( argc>2 ) vflag = 1;

  numb = 0;

  if( scanf( sfile, "%d%d%s", &innumb, &inlev, suff ) < 0 ) innumb = 32000;

  while( gets( word, wdfile ) ) {
    ++numb;
    if( innumb > numb ) printf( "%s%s\n", vflag? "****\t" : "", word );
    if( innumb == numb ){
      if( inlev && vflag ) printf( "%s\t%s\n", suff, word );
      while( innumb == numb )
        if( scanf( sfile, "%d%d%s", &innumb, &inlev, suff ) < 0 ) innumb = 32000;
      }
    }
  }
