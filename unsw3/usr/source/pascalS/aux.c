#include "nglob.h"
int *al(x,s)
char x; /* the bit to be set */
char *s;/* set to modify */
{
/*
 *	  puts 'x' into set 's'.
 */
	  register char xx;

	  s[(xx=x)>>3] =|
		(1<<(xx&007));
 	  return(s);
 }

int *as(s1,s2)
int *s1,*s2;	  /* ptrs to sets */
{
/*
 *	  merges set 's1' with 's2',
 *	  destroying the old 's2'.
 */
	  register *i,*j;

	  i = s1; j = s2;
	  *j++ =| *i++;
	  *j++ =| *i++;
	  *j++ =| *i++;
	  *j   =| *i  ;
	  return(s2);
}
int *copyset(x,y)
int *x,*y;	  /* ptrs to sets */
{
/*
 *	  copies set 'x' onto 'y'.
 */
	  register *i,*j;

	  j = x; i = y;
	  *i++ = *j++;
	  *i++ = *j++;
	  *i++ = *j++;
	  *i   = *j;
	  return(y);
}
copystr(s1p,s2p)
char *s1p,*s2p;
{
/*
 *	  copies 's1p' to 's2p'.
 */
	  register char *s1,*s2;
	  s1 = s1p; s2 = s2p;
	  while(*s2++ = *s1++);
}
differ(s1,s2)
char *s1,*s2;
{
	  register char *cc1,*cc2;

	  cc1 = s1; cc2 = s2;
	  while(*cc1 == *cc2++)
		  if(*cc1++ == '\0')
			  return(0);
	  return(*cc1 - *--cc2);
}



in(x,s)
char x;
char *s;
{
/*
 *	  returns whether 'x'th bit
 *	  of set 's' is set.
 */
	  register char xx;

	  return(s[(xx=x)>>3] 
		& (1<<(xx&007)));
}

nextch()
{
/*	  fetch next character.
 */
	  if(cc == ll)
		  getline();
	  ch = line[cc++];
}
