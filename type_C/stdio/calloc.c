#include <stdio.h>

char *
calloc(num, size)
{
	register int p1, *p2, nbyte;

	nbyte = (num*size+1) & ~01;
	if ((p1 = alloc(nbyte)) == -1 || p1==0)
		return(NULL);
	p2 = p1;
	nbyte =>> 1;		/* 2 bytes/word */
	do {
		*p2++ = 0;
	} while (--nbyte);
	return(p1);
}

cfree(p)
char *p;
{
	free(p);
}
