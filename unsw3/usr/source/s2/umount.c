#include	<mtab.h>



main(argc, argv)
char **argv;
{
	register struct mtab *mp;
	register char *p1, *p2;
	int mf, n;

	if(argc < 2)
	{
		printf("arg count\n");
		return( 1 );
	}

	sync();

	if ( (mf = open(mtabf, 0)) < 0 )
	{
		perror( mtabf );
		return( 1 );
	}

	n = read(mf, mtab, sizeof mtab);
	if( n % sizeof mtab[0] )
	{
		printf("/etc/mtab: wrong format\n");
		return( 1 );
	}

	for( ; --argc ; argv++ )
	{
		if (umount(argv[1]) < 0)
		{
			perror("umount");
			continue;
		}

		p1 = argv[1];
		while(*p1++)
			;
		p1--;
		while(*--p1 == '/')
			*p1 = '\0';
		while(p1 > argv[1] && *--p1 != '/')
			;
		if(*p1 == '/')
			p1++;
		argv[1] = p1;
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
		{
			p1 = argv[1];
			p2 = &mp->spec[0];
			while (*p1++ == *p2)
				if (*p2++ == 0)
				{
					for (p1 = mp->file; p1 < &mp->file[NAMSIZ*2];)
						*p1++ = 0;
					mp = &mtab[NMOUNT];
					while ((--mp)->file[0] == 0);
					close(mf);
					if( (mf = creat(mtabf, 0604)) < 0 )
					{
						perror("/etc/mtab");
						return( 1 );
					}
					n = (mp-mtab+1)*(sizeof mtab[0]);
					if( write(mf, mtab, n) != n )
					{
						perror("write");
						return( 1 );
					}
					goto loop;
				}
		}

		printf("%s not in mount table\n", argv[1]);
loop:		;
	}
}
