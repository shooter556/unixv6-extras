#include	"odb7.h"
#define	r2	0764
#define	ps	0777
extern	int	odbfil, header;
extern	int	errno;

getcont()  {
	register val;
	register *p;
	if (orgfil == REGIST)
		{
		if (cmode=='i')
			cmode = 'o';
		val = users[dot];
	}
	else
		{
		val = newval(orgfil, &dot);
		if (bytwrd != 1)
			{
			if (dot&1)
				{
				bytwrd = 1;
				val =& BMASK;
				}
			}
		else
			val =& BMASK;
		}
	prcont(val);
}



closloc()  {
	register b, tdot;
	register char *tp;
	int t;
	if ( (b=bytwrd) > 1 )
		b = 2;
	if (b && valfnd)
		{
		tdot = dot;
		if (orgfil == ODBINT)
			error();
		if (orgfil == REGIST && tdot>=r2 && tdot<=ps)
				{
				if (pid)
					for (b=0; b<9; b++)
						if (r[b] == tdot)
						{
						ptrace(6, pid, r[b]<<1, adrval);
						break;
						}
				users[tdot] = adrval;
				}
			else
				{
				if (pid)
					{
					t = ptrace(1, pid, tdot, 0);
					tp = &t;
					if (tdot&1)
						*++tp = adrval&0377;
					else
						if (b==1)
							*tp = adrval&0377;
						else
							t = adrval;
					ptrace(4, pid, tdot, t);
					}
				else
					{
					seek(odbfil, tdot+header, 0);
					write(odbfil,&adrval,b);
					}
				}
		}
	   }

uparlf() 
 	 {
	register int *padr;
	register adr,i;
	if (!bytwrd)
		bytwrd = 2;
	closloc();
	adr=dot;
	if (orgfil == REGIST)
		{ for (i=0; i<9; i++)
		   	if (r[i] == adr)
			{
			adr=(ch == '\n' ? r[i+1] : r[i-1]);
			break;
			}
		}
	 else
		if (ch == '\n')
			adr =+ bytwrd;
		else
			adr =- bytwrd;
	if (orgfil == ODBINT &&
	   ((padr=adr)< &bkpt || padr>= &bkpt[NOOFBP]))
			error();
	dot = adr;
	putlp("\r");
	openxt();
	}

openxt()
	{
	if (orgfil == REGIST)
		{
		regtyp();
		return;
	}
	if (cmode != 'i')
		{
		if (symfnd)
			symbolic();
		else
			putoct(dot);
		if ( bytwrd == 1 )
			putlp(" \\");
		else
			putlp(" /");
		}
	getcont();
	}


equal()
	{
	putoct( (valfnd ? adrval : dot) );
	putlp("\r\n*");
	}

regtyp()
	{
	register i,radr;
	bytwrd = 2;
	if (cmode == 'i')
		cmode = 'o';
	radr=dot;
	for (i=0; i<9; i++)
		if (r[i] == radr)
			{
			putlp(regs[i]);
			putlp(" /");
			prcont(users[r[i]]);
			return;
			}
	error();
	}

get(v)
	{
	int x;
	register t1,t2,addr;
	addr = v;
	if (pid)
		{
		t1 = ptrace(1,pid,addr,0);
		if (addr&1)			/* addr must be even  */
			{			/* if not, move bytes */
			t2 = ptrace(1,pid,addr+1,0);
			t1 = (t1>>8)&0377 | (t2<<8);
			}
		if (errno)
			error();
		return(t1);
		}
	else
		{
		seek(odbfil, addr+header, 0);
		if (read(odbfil,&x,2) != 2)
			error();
		return(x);
		}
}

newval(f, a)
int *a;
	{
	register int *p;
	switch(f)
		{
		case ODBEXT :
			return(get(*a));
		case REGIST :
			return(users[*a]);
		case ODBINT :
			if ( *a&1 )
				*a =+ 1;
			p = *a;
			return(*p);
		}
}
