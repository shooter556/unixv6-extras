#include	<param.h>
#include	<passwd.h>
#include	<stat16.h>

struct statbuf statb;
struct { int hiword; unsigned loword; };

main(c,v)
char *v[];
{
	struct pwent pe;
	register i;
	char buf[512];
	extern fout;
	long tdu(),l;
	unsigned used;
	int highbd,sflg;
	register char *b = buf;


	fout = dup(1);
	switch(c)
	{
    case 1:	i = 1;
		highbd = PWTABSIZE;
		break;
    case 2:
		i = atoi(v[1]);
		if( i < 0 ) goto error;
		highbd = i+1;
		if( highbd > PWTABSIZE )
			highbd = PWTABSIZE;
		sflg++;
		break;
    case 3:	i = atoi(v[1]);
		highbd = atoi(v[2]);
		if ( i < 0 ) i = 0;
		if(highbd < i ) goto error;
		if(highbd > PWTABSIZE)
			highbd = PWTABSIZE;
		break;
    default:
	error:	prints(2,"Usage: pwscan [uid] | [uidl uidh]\n");
		return(-1);
	}
	for( ; i < highbd; i++)
	{
		pe.pw_uid = i;
		if( getpwlog( &pe, b, 512) <= 0 )
		{
			if(sflg)
			{
				prints(2, "Uid not found\n");
				return(1);
			}
			continue;
		}
		if( chdir(pe.pw_strings[DIRPATH])<0 )
		{
			if(sflg)
			{
				prints(2, "No directory\n");
				return(1);
			}
			continue;
		}
		newstat( "." , &statb );
		l = tdu();
		used = l.hiword;
		if( used < pe.pw_dlimit )
		{
			if(sflg)
			{
				printf("Uid: %5u, TDU: %5u, Softlimit: %5u, Hardlimit: %5u, Name: '%s'\n",
					i,
					used,
					  pe.pw_dlimit,
					   pe.pw_dlimit + pe.pw_doverflw,
					pe.pw_strings[LNAME]);
			}
			continue;
		}
		if(pe.pw_dlimit != 0)
			{
			if(  used > (pe.pw_dlimit + pe.pw_doverflw) )
				{
					printf("** Hard");
				}
				else
				{
					printf("   Soft");
				}
				printf(": Uid: %5u, TDU: %5u, Soft: %5u, Hard: %5u, Name: '%s'\n",
					i, used, pe.pw_dlimit,
					   pe.pw_dlimit + pe.pw_doverflw, pe.pw_strings[LNAME]);
			}
	}
	flush();
	return(0);
}



/*
 *	"tdu" returns the disc usage
 *		Inodes are weighted at IWEIGHT blocks.
 *		Links are shared equally.
 */

long tdu()
{
	register fd, posn;
	long inodes, blocks;
	static struct { int f_ino; char f_name[14]; char f_end; } dirbuf;
	static char dot[]	".";
	unsigned size();


  inodes = 0X10000;
  blocks.loword = 0;
  blocks.hiword = size();

  posn = 32;
  dirbuf.f_end = '\0';

loop:
  fd = open( dot , 0 );
  seek( fd , posn , 0 );

  while ( read( fd , &dirbuf , 16 ) == 16 )  {
	posn =+ 16;
	if ( dirbuf.f_ino && newstat( dirbuf.f_name , &statb ) >= 0 )
		if ( (statb.sb_flags & IFTYP) == IFDIR )  {
			if ( chdir( dirbuf.f_name ) >= 0 )  {
				close( fd );
				blocks =+ tdu();
				chdir( ".." );
				goto loop;
			}
		}else  {
			/* fixed point arithmetic */
			inodes =+ (0X10000 / (statb.sb_nlinks&0377));
			blocks =+ ((0X10000 * size()) / (statb.sb_nlinks&0377));
		}
  }

  close( fd );
  return( blocks + inodes*IWEIGHT );
}


unsigned size()
{
	register unsigned b, ib;

  b = ((statb.sb_size0 & 0377) << 7) + (((statb.sb_size1 + 0777) >> 9) & 0177);
  if ( ib = b>>3 )  {
	b++;
	if ( ib =>> 5 )  {
		b =+ ib;	/* indirect */
		if ( ib > 8 )
			b++;	/* double indirect */
	}
  }

  return( b );
}
