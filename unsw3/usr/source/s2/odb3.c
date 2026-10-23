#include	"odb7.h"
extern	char	*signam[];
expr()
	{
	register tadval;
	register char lastop;
	curfil = 0;
	if ( !term(0) )
		return(0);
	for (;;)
		{
		if ( !match(lastop=ch,"+-") )
				return(1);
		rch();
		skip();
		tadval = adrval;
		if ( !term(0) )
			return(0);
		if ( lastop=='+' )
			adrval =+ tadval;
		else
			{
			tadval =- adrval;
			adrval = tadval;
		}
	}
}


term(i)
	{
	register int *p;
	register char op, j;
	adrval = 0;
	if (alpha(ch))
		{
		if (scflg)
			return(0);
		if ( !getnam() )
			error();
	}
	else
		{
		if (digit(ch))
			adrval = rdnum();
		else
		switch(ch)
			{
		case '!' :
			curfil = REGIST;
				switch(rch())
				{
				case 'b' :
					curfil = ODBINT;
					adrval =+ bkpt;
					break;
				case '\r' :
				case '\n' :
					newline();
					putlp(signam[sign]);
					linput();
					for (j=0; j < 9; j++)
						{
						newline();
						putlp(regs[j]);
						putlp("\t");
						putoct(users[r[j]]);
						linput();
						}
					reset();
				case 's' :
					adrval = r[8];
					break;
				default :
					if ('0' <= ch && ch <= '7' )
							adrval = r[ch&07];
					else	error();
				}
			rch();
			break;
		case '+' :
		case '-' :
			op = ch;
		case ' ' :
		case '\t' :
			rch();
			skip();
			if ( !term(0) )
				return(0);
			if ( op == '-' )
				adrval = -adrval;
			break;
		case '@' :
			if ( i )
				error();
			rch();
			if ( !term(1) )
					return(0);
			adrval = newval(curfil, &adrval);
			curfil = ODBEXT;
			break;
		case '"':
			adrval = (rch()<<8) | rch();
			goto endch;
		case '\'' :
			adrval =| rch();
   		endch:
			rch();
			break;
		default :
			skip();
			return(0);
		}
	}
	skip();
	return(1);
}


rdnum()
	{
	register octv,decv;
	octv = decv = ch-'0';
	while ( digit(rch()) )
		{
		octv = (octv<<3) | (ch - '0');
		decv = decv*10 + ch-'0';
		}
	if ( ch == '.' )
		{
		rch();
		return(decv);
		}
	return(octv);
}
alpha(a)
	{
	register c;
	c = a;
	 if (c >='a' && c <='z' || 
	     c >='A' && c <='Z' ||
	     c == '_' ||
		     c=='$' || c=='.')
		return(1);
	return(0);
}


digit(c)
	{
	if ( c >= '0' && c <= '9' )
		return(1);
	return(0);
}
skip()
	{
	while (match(ch," \t")) rch();
}
