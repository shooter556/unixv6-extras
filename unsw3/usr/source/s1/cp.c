#
/*
 * cp oldfile newfile
 *
 *****	null blocks are now skipped, not allocated
 *****				daveh, jan 78
 *
 *	changed to know about locking, and use perror
 *				Piers Lauder	Jul '78
 *	altered to really check for copying to same file
 *	and to ensure modes of a previously existing file
 *	are truly preserved.
 *
 *				Ian Johnstone	AUG '78
 */

#define	LOCKING		02010
#define	FULL_LOCK	02000




main(argc,argv)
char **argv;
{
	static int buf[256];
	int fold, fnew, n;
	register char *p1, *p2, *bp;
	int mode;
	struct
	{
		char hibyte;
		char lobyte;
	};
	char name[200];

	if(argc != 3) {
		printf( "Usage: cp oldfile newfile\n" );
		return(1);
	}

	if((fold = open(argv[1], 0)) < 0) {
		perror( argv[1] );
		return(1);
	}

	fstat(fold, buf);
	mode = buf[2];	/* remember mode of old file */

	/* is target a directory? */

	if( stat(argv[2], buf+50) != -1 )
	{
		if ( (buf[52]&060000)==040000  )
		{
			/* a directory */
			p1 = argv[1];
			p2 = argv[2];
			bp = name;
			while(*bp++ = *p2++);
			bp[-1] = '/';
			p2 = bp;
			while(*bp = *p1++)
				if(*bp++ == '/')
					bp = p2;
			argv[2] = name;
			if( stat(argv[2], buf+50) != -1 )
				mode = buf[52];
			else
				goto create;
		}

		if (buf[0]==buf[50] && buf[1]==buf[51])
		{
			printf( "Copying file to itself.\n" );
			return(1);
		}
	}

   create:
	if ((fnew = creat(argv[2], mode)) < 0)
	{
		perror( argv[2] );
		return(1);
	}

	if ( (mode & LOCKING) == FULL_LOCK )
		readlock( fold );

	while(n = read(fold,  buf,  512))
	{
		if(n < 0)
		{
			perror( argv[1] );
			return(1);
		}
		else
		{
			/* see if zero block read */

			p1 = buf;
			while( p1<buf+256 && *p1++ == 0 );	/* n.b.  256, not 512 ! */
			if( p1==buf+256 )			/* zero block */
				seek(fnew, 1, 4);		/* skip block */
			else
				if(write(fnew, buf, n) != n)
				{
					perror( argv[2] );
					return(1);
				}
		}
	}
	return(0);
}
