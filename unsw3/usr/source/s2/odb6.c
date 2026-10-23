#include	"odb7.h"

#define	TBIT	020
extern	int	errno;
extern	char	*signam[];
breakp()
	{
	if ( !svalfnd )
		rmvbkp();
	else
		setbkp();
}
setbkp()
	{
	register struct list *p,*bl;
	bl = &bkpt[NOOFBP];
	if (valfnd)
		{
		if (adrval>8)
			error();
		for (p=bkpt; p<bl; p++)
			if (p->badr == tadrval)
				return;
		(bkpt+adrval)->badr = tadrval;
		}
	else
		{
		for (p=bkpt; p<bl; p++)
			{
			if (p->badr == tadrval)
				return;
			if ( p->badr < 0 )
				{
				p->badr = tadrval;
				return;
				}
			}
		error();
		}
}

rmvbkp()
	{
	register struct list *p;
	p=bkpt+adrval;
	if (!valfnd) rmvall();
	else
		if (adrval > 8) error();
		else
		if (p->badr >= 0 && pid)
			remove(p);
}

cbplist()
	{
	register struct list *p;
	for (p=bkpt; p < &bkpt[NOOFBP]; p++)
			{
			p->badr = -1;
			p->bcnt = p->bcount = 0;
			}
}

rmvall()
	{
	register struct list *p,*bl;
	if (pid==0)
		{
		cbplist();
		return;
		}
	bl = &bkpt[NOOFBP];
	for (p=bkpt; p < bl; p++)
		if (p->badr >= 0)
			remove(p);
}

remove(p)
struct list *p;
	{
	if (p->badr == savebp)
		{		/* it is the one that */
				/* stopped us         */
		savebp = 0;
		ptrace(6, pid, r[8]<<1, users[r[8]]& ~TBIT);
		}
				/* restore original  */
	ptrace(4, pid, p->badr, p->bcnt);
	p->badr= -1;
	p->bcnt = p->bcount= 0;
}

go()
	{
	savebp = 0;
	scflg--;
	if ( pid )
		{
		ptrace(8, pid, 0, 0);		/* terminate traced process */
		pid = 0;
		}
	if ((pid=fork()) == 0)
		{
		ptrace(0, 0, 0, 0);		/* trace me ! */
		signal(2,0);
		signal(4,0);
		vexec();
		printf("EXEC ERROR on %s\n",sfil);
		exit(0);
		}
}


vexec()
	{
	extern _exectrap;
	register char **vp, *p;
	int inbuf[10];
	char *argve[32];
	register c;
	vp = argve;
	*vp++ = sfil;
	getlin(1);
	p = lp;
	do {
		while (*p == ' ')
				p++;
		if (match(*p, "\n\0"))
				break;
		*vp++ = p;
		while( !match(*p,"\n "))
					p++;
		c = *p;
		*p++ = '\0';
	} while (!match(c,"\n"));
	*vp++ = 0;
	printf("\n");
	_exectrap++;
	if (tadrval>0)
		{
		seek(symfil,0,0);
		read(symfil,inbuf,020);
		if (inbuf[0] == 0407)
				{
				seek(symfil,10,0);
				write(symfil,&tadrval,2);
				}
		}
	execv(sfil, argve);
}


/* restore breakpoints, */
/* save contents, and replace with trap */
bpoint()
	{
	register struct list *p,*bl;
	register addr;
	bl = &bkpt[NOOFBP];
	for (p=bkpt; p<bl; p++)
		{
		addr = p->badr;
		if ( addr>=0 && p->bcnt==0)
			{
			p->bcnt = ptrace(1, pid, addr, 0);
			ptrace(4, pid, addr, 3);
			if (errno)
				{
				putlp("\nBreakpoint not possible at ");
				putoct(p->badr);
				putlp("\n");
				linput();
				}
			}
		}
}


/* notify the parent via wait */
waitin()
	{
	int setraw();
	int stat;
	register p, i;
loop:
	signal(2,1);
	while((p=wait(&stat)) != pid && p != -1);
				/* wait until end of child */
	signal(2,setraw);
	if (p == -1)		/* no children */
		{
		ptrace(8, pid, 0, 0);
		pid = 0;
		putlp("\nWAIT ERROR\n");
		linput();
		setraw();
		}
	if ((stat&0377) != 0177) 	/* if process terminated */
		{
		if (sign=stat&0177)
			{
			putlp("\n");
			putlp(signam[sign]);
			}
			putlp("\nProcess terminated.\n");
			linput();
			if (pid == p)
				{
				pid = 0;
				cbplist();
				linput();
				setraw();
				}
			goto loop;
		}
	sign = stat>>8;			/* we have a stoppage */
	for (i=0; i<9; i++)
		users[r[i]] = ptrace(3, pid, r[i]<<1, 0);
	if (sign!=5)
		{			/* stoppage is not due to bpoint */
		putlp("\n");
		putlp(signam[sign]);
		putlp("\n");
		linput();
		setraw();
		}
}


prosid()
	{
	register addr;
	register struct list *p;
	if ((addr=savebp))
		{
			/* ignore the signal that caused the interrupt */
		ptrace(7, pid, 0, 0);
		waitin();
			/* set a trap where last bpoint was */
		ptrace(4, pid, addr, 3);
			/* clear t-bit */
		ptrace(6, pid, r[8]<<1, users[r[8]]& ~TBIT);
		savebp = 0;
		}
	ptrace(7, pid, 0, 0);
	waitin();
	addr = users[r[7]] - 2;		/* get addr where bpoint is */
	for (p=bkpt; p< &bkpt[NOOFBP]; p++)
		if (p->badr==addr)
			{		/* interrupt due to bpoint */
			if ( tadrval )
				{
				p->bcount = tadrval;
				tadrval = 0;
				}
			savebp = addr;	/* save it's address */
					/* restore original contents */
			ptrace(4, pid, addr, p->bcnt);
					/* pc gets that address */
			ptrace(6, pid, r[7]<<1, addr);
					/* set the t-bit */
			ptrace(6, pid, r[8]<<1, users[r[8]] | TBIT);
					/* return control to user */
			if (--p->bcount > 0)
				{
				bpoint();
				prosid();
				}
			p->bcount = 1;
			putlp("\nB");
			*lbp++ = p-bkpt+'0';
			putlp(";");
			goto set;
			}
	putlp("\n");		/* we have some other type of interrupt */
	putlp(signam[sign]);
	putlp(" at ");
	set:
	putoff(addr);
	linput();
	setraw();
}
setraw()
	{
	signal(2, 1);
	if (cfdes > 1)
		close(cfdes);
	setty(040);
	reset();
}
