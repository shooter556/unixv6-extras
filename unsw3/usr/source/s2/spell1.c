/* possible improvements:
    ation
    ize
    prefixes: non, sub, un
    */
char word[50];
int numb 0;
main(){
  char *p;
  while( gets( word ) ) {
    ++numb;
    wword( "." , 0 );
    for( p=word; *p != 0; ++p ) ;
    /* now, p points to the end of the word */
    if( p - word  <= 2 ) continue;
    switch( *--p ){
    case 's':
      *p = 0;
      wword( "+s", 1 );
      if( *--p != 'e' ) continue;
      if( *(p-1) == 'i' ){
        *p = 0;
        *--p = 'y';
        wword( "-y+ies" , 2 );
        continue;
        }
      *p = 0;
      wword( "+es" , 2 );
      continue;
    case 'd':
      *p = 0;
      if( *--p != 'e' ) continue;
      if( *(p-1) == 'i' ){
        *p = 0;
        *--p = 'y';
        wword( "-y+ied" , 2 );
        continue;
        }
      wword( "+d" , 1 );
      *p = 0;
      wword( "+ed" , 2 );
      continue;
    case 'g':
      if( *--p != 'n' ) continue;
      if( *--p != 'i' ) continue;
      *p = 0;
      wword( "+ing" , 1 );
      *p = 'e';
      *++p = 0;
      wword( "-e+ing", 2 );
      continue;
    case 'r':
      if( *--p != 'e' ) continue;
      if( *(p-1) == 'i' ){
        *p = 0;
        *--p = 'y';
        wword( "-y+ier" , 2 );
        continue;
        }
      *p = 0;
      wword( "+er" , 1 );
      continue;

    case 'y':
      if( *--p != 'l' ) continue;
      *p = 0;
      wword( "+ly", 1 );
      continue;

    case 't':
      if( *--p != 's' ) continue;
      if( *--p != 'e' ) continue;
      if( *(p-1) == 'i' ){
        *p = 0;
        *--p = 'y';
        wword( "-y+iest" , 1 );
        continue;
        }
      *p = 0;
      wword( "+est" , 1 );
      continue;
      }
    }
  }

wword( s, l ) char *s; {
  char *p;
  for( p = word; *p != 0 ; ++p ){
    if( *p == 'a' || *p == 'e' || *p == 'i' || *p == 'o' || *p == 'u' || *p == 'y' ) goto prt;
    }
  if( *s == '.' && word[0] != '\0' ) goto prt;
  return;


  prt:
  printf("%s\t%s\t%05d\t%d\n", word, s, numb, l );
  }
