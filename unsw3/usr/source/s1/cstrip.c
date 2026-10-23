/*
 *	strip c programs of comments & extraneous spaces & newlines
 */

struct	buf
{
	int	fildes, nleft;
	char	*nextp, buff[512];
}
	inbuf,
	oubuf;

int	sep	'\n';
int	string;
int	slosh;
int	hash;

main()
{
	register c;
	register state = 0;
	register struct buf *ou = &oubuf;

  ou->fildes = 1;

  while ( (c = getc( &inbuf )) != -1 )
	switch ( state )
	{
	 case 1:
		if ( c == '*' )
		{
			state++;
			continue;
		}
		else
		{
			state = 0;
			putc( '/', ou );
		}

	 case 0:
		if ( slosh )
			slosh = 0;
		else
		if ( string )
		{
			if ( c == '\\' )
				slosh++;
			else
			if ( c == string )
			{
				string = 0;
				sep = c;
				continue;
			}
		}
		else
		switch ( c )
		{
		 case '/':
			state++;
			continue;
		 case '\n':
			hash = 0;
			goto csep;
		 case '\t':
			c = ' ';
		 case ' ':
			if ( sep )
				continue;
csep:
		 case '(': case ')': case '[': case ']': case '{': case '}':
		 case ',': case ';': case ':': case '?':
			if ( sep && (( c != sep && ( sep != ' ' || hash )) || c != '\n' ) )
				putc( sep, ou );
			sep = c;
			continue;
		 case '#':
			if ( sep == '\n' )
				hash++;
			break;
		 case '\\':
			slosh++;
			break;
		 case '"':
		 case '\'':
			string = c;
			if ( sep == ' ' )
				sep = 0;
		}

		if ( sep )
			putc( sep, ou );
		sep = 0;
		putc( c, ou );
		continue;

	 case 3:
		if ( c == '/' )
		{
			state = 0;
			continue;
		}
		else
			state--;

	 case 2:
		if ( c == '*' )
			state++;
		continue;
	}

  putc( '\n', ou );

  fflush( ou );

  if ( state|slosh|string || sep != '\n' )
  {
	write( 2, "Syntax error: ", 14 );
	sep = '0' + state + (string ? 4 : 0) + (slosh ? 010 : 0) + (sep != '\n' ? 020 : 0);
	write( 2, &sep, 1 );
	write( 2, "\n", 1 );
  }
}
