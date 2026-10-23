#include	<mtab.h>

main(argc, argv)
register char **argv;
{
	register struct mtab *mp;
	register char *np;
	int n, mf, ro;

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
	if (argc==1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0])
				printf("%s on %s%s\n", mp->spec, mp->file, mp->prot ? "  (read only)" : "");
		return( 0 );
	}
	if(argc < 3) {
		printf("arg count\n");
		return( 1 );
	}
	ro = ( argc > 3 );
	if(mount(argv[1], argv[2], ro) < 0) {
		perror("mount");
		return( 1 );
	}
	np = argv[1];
	while(*np++)
		;
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > argv[1] && *--np != '/')
		;
	if(*np == '/')
		np++;
	argv[1] = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			for (np = mp->spec; np < &mp->spec[NAMSIZ-1];)
				if ((*np++ = *argv[1]++) == 0)
					argv[1]--;
			for (np = mp->file; np < &mp->file[NAMSIZ-1];)
				if ((*np++ = *argv[2]++) == 0)
					argv[2]--;
			mp->prot = ro;
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
			return( 0 );
		}
	}
}
