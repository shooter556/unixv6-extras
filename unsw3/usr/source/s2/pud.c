#include	<defines.h>
#include	<param.h>
#include	<proc.h>
#include	<user.h>
#define	NINODE	1
#include	<inode.h>

#include	<passwd.h>

struct
{
	char *name;
	int inums[20];
}
devs []
{
	{	"/dev/rhp0"	},
	{	"/dev/rhp1"	},
	{	"/dev/rhp2"	},
	{	"/dev/rhp3"	},
};

struct	pwent pe[2];
char	buf[512];
char workarea[5000];

main(c,v)
char **v;
{
	register i,j,k;
	char *q;
	char *a, *arglist[50];
	int *argp;
	struct	pwent pex;

	nice(-40);
	i = gprocs(proc);
	while( --c )
	{
		v++;
		switch( v[0][0] )
		{
	    case '-':
			q = &v[0][1];
			k = 0;
			while( *q>='0' && *q<='9' ) k = k * 10 + *q++ - '0';
			if( *q )
			{
				printf("invalid process id %s\n",&v[0][1]);
				continue;
			}
			for(j = 1; j < i; j++)
				if( proc[j].p_pid == k)
				{
					locate( &proc[j] );
					goto cont;
				}
			printf("proc %l not found\n",k);
			continue;
	    default:
			pex.pw_strings[LNAME] = *v;
			if ( getpwuid( &pex, 0, 0 ) < 0 )
			{
				printf("%s not known ??\n",*v);
				continue;
			}
			k = 0;
			for(j = 1; j < i; j++)
				if( proc[j].p_uid == pex.pw_uid)
				{
					k++;
					locate( &proc[j] );
				}
			if( k == 0 ) printf("%s has no active procs\n",*v);
			continue;
		}
   cont:;
	} /* end of while */
	for( j=0 ; j<sizeof devs/sizeof devs[0] ; j++ )
	{
		a = workarea;
		argp = arglist;
		*argp++ = "/etc/ncheck";
		*argp++ = "-i";
		if( devs[j].inums[0] )
		{
			printf("ncheck  -i ");
			for( i=0 ; i<sizeof devs[0].inums ; i++ )
			{
				if( devs[j].inums[i] == 0 ) break;
				itoa( devs[j].inums[i] , a );
				printf(" %s",a);
				*argp++ = a;
				a =+ 6;
			}
			printf(" %s\n",devs[j].name);
			*argp++ = devs[j].name;
			if( !fork() )
			{
				*argp = 0;
				execv("/etc/ncheck", arglist );
				exit();
			}
			waitx(buf);
		}
	}
}

locate( p )
struct proc *p;
{
	register i,j,k;
	int *q;
	char *a;

	if( p->p_stat == SZOMB )
	{
		printf("process %d is dead\n", p->p_pid);
		return;
	}
	if( p->p_flag & SLOAD)
	{
		k = open("/dev/mem", 0);
		seek(k, p->p_addr>>3, 3);
		seek(k, (p->p_addr & 07) << 6, 1);
	}
	else
	{
		k = open("/dev/swap", 0);
		seek(k, p->p_addr, 3);
	}
	read(k, &u, sizeof u);
	close(k);
	k = open("/dev/kmem", 0);
	seek(k, u.u_cdir, 0);
	read(k, inode, sizeof inode);
	close(k);
	i = inode->i_uidl & 0377;
	i =| inode->i_uidh << 8;
	pe->pw_uid = p->p_uid;
	getpwlog(pe, buf+100, 200);
	pe[1].pw_uid = i;
	getpwlog(pe+1, buf+300, 100);
	if( pe[0].pw_strings[LNAME] == 0 || *pe[0].pw_strings[LNAME] == 0 )
	{
		pe[0].pw_strings[LNAME] = a = workarea;
		*a++ = '(';
		itoa( p->p_uid , a );
		while( *a ) a++;
		*a++ = ')';
		*a = 0;
	}
	if( pe[1].pw_strings[LNAME] == 0 || *pe[1].pw_strings[LNAME] == 0 )
	{
		pe[1].pw_strings[LNAME] = a = &workarea[20];
		*a++ = '(';
		itoa( i , a );
		while( *a ) a++;
		*a++ = ')';
		*a = 0;
	}
	printf("proc %d, uid = %s,\ncdir: dev = %s, ino = %d, owner = %s\n",
		p->p_pid, pe[0].pw_strings[LNAME],
		devs[inode->i_dev].name, inode->i_number,
		pe[1].pw_strings[LNAME] );
	stat(pe->pw_strings[DIRPATH], buf);
	if((buf[0] == inode->i_dev) &&
		(i = ilook(inode->i_number, pe->pw_strings[DIRPATH])))
	{
		printf("subdirectory: %s\n", i);
		return;
	}
	q = &devs[inode->i_dev].inums;
	for( i=0 ; i<sizeof devs[0].inums ; i++ , q++ )
	{
		if( *q == inode->i_number ) break;
		if( *q == 0 )
		{
			*q = inode->i_number;
			break;
		}
	}
}
itoa(n,s)
register char *s;
{
	if( n/10 )
		s = itoa(n/10, s);
	*s++ = n%10 + '0';
	*s = 0;
	return s;
}

struct
{
	int	inum;
	char	nam[15];
} dirent;
int sbuf[18];
char *wkp workarea;

cat( s1 , s2 )
register char *s1,*s2;
{
	register char *q;
	char *sav;

	sav = q = wkp;
	while( *s1 ) *q++ = *s1++;
	if( q[-1] != '/' ) *q++ = '/';
	while( *s2 ) *q++ = *s2++;
	*q++ = 0;
	wkp = q;
	return( sav );
}

ilook( ino , pathn )
int ino;
char **pathn;
{
	register fd, pos;
	int x;
	char *new,*sav;

	sav = wkp;
	pos = 0;
 again:	if( (fd = open( pathn , 0 )) < 0 ) goto fail;
	if( seek( fd , pos , 0 ) < 0 ) goto fail;
 next:	if( read( fd , &dirent , 16 ) != 16 ) goto fail;
	pos =+ 16;
	if( dirent.inum == 0 ) goto next;
	if( dirent.nam[0]=='.' && dirent.nam[1]=='.' && dirent.nam[2]==0 ) goto next;
	new = cat( pathn , dirent.nam );
	if( dirent.inum == ino )
	{
		wkp = sav;
		if( dirent.nam[0]=='.' && dirent.nam[1]==0 )
			return( pathn );
		else
			return( new );
	}
	if( dirent.nam[0]=='.' && dirent.nam[1]==0 ) goto next;
	if( stat( new , sbuf ) < 0 ) goto fail;
	if( (sbuf[2]&060000)!=040000 ) goto next;
	close( fd );
	if( (x = ilook( ino , new )) == 0 ) goto again;
	wkp = sav;
	return( x );
 fail:
	close(fd);
	wkp = sav;
	return( 0 );
}
