/*
 *	validate kronos job kard
 *
 *	errors:
 *		1) bad job name.
 *		2) bad delimiter.
 *		3) bad parameter specification.
 *		4) rubbish.
 *		5) too many cyber jobs.
 *		6) no cyber jobs allowed.
 */

/*
 *	this code will be commented later on
 *	(when i figure out how it works ... )
 */
char *str;

octal()
{
	register char *c;
	register flg;

	flg = 0;
	while( (c = *str) >= '0' && c < '8' ){
		flg++;
		str++;
	}
	return(flg == 0);
}

vkjk(nm,string)
char *string,*nm;
{
	register int n, *nmp;	register char *x;

	nmp = nm;
	str = string;
	*nmp++ = ' 0';	/* jobname 'x' -> 'x0' */
	*nmp = *nmp++ = *nmp++ = '  ';
	n = -1;
	while( (x = *str++,++n < 7) && (((x - 'a') < 26) || (( (x - '0') < 10) && n)) )
	{
		if( x & 0100) x =& ~040;
		*nm++ = x;
	}
	if (nocyb) return(6);
	if (ncybjobs >= maxcjobs) return(5);
	if( n == 0) return(1);
	if( (x != ',') && (x != '(') )
		if(x == '.')
			return(0);
		else	return(2);
	n = 0;
	x = *str;
	if( x>='0' && x<'8' )
	{
		for( ; n < 4; n++)
		{
			if( octal())
				return(3);
			switch(*str++)
			{
		    case ')':
		    case '.':
				return(0);
		    case ',':
				continue;
		    default:	return(2);
			}
		}
	}
	else
	{
		do
		{
			switch(*str++)
			{
		    case 't':
				if( (n&1) || octal() ) return(3);
				n =| 1;
				break;
		    case 'c':
				if( (n&2) || (*str++ != 'm') || octal() ) return(3);
				n =| 2;
				break;
		    case 'p':
				if( (n&4) || octal() ) return(3);
				n =| 4;
				break;
		    case 'd':
				if( (n&8) || octal() ) return(3);
				n =| 8;
				break;
		    default:
				return(4);
			}
		} while( *str++ == ',');
		if( ( (x = *--str) == '.') || (x == ')'))
			return(0);
	}
	return(2);
}

/*
 *	validate local job card
 *
 *	errors same as "vkjk"
 */

vljc(q, card)
struct quelt	*q;
register char	*card;
{
	register unsigned	i;
	register char	ch;

	for (i = 0; i < 7; i++)
		if (((ch = *card++) >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') )
				q->jobn[i] = ch;
		else
			break;		/* end of job name */

	for (; i < 7; i++)
		q->jobn[i] = ' ';

	if (i == 0 || q->jobn[0] < 'A')		/* bad jobname */
		return(1);

	if (noloc)			/* can't allow local jobs */
		return(6);

	q->tlimit = 20;		/* default time */

	if (ch == '.' || ch == ' ')	/* end of scan */
		return(0);

	if (ch == '(' || ch == ',')	/* continue scan */
	{
		do
			switch (ch = *card++)
			{
	
			case 't':		/* time */
				i = 0;
				while ((ch = *card) >= '0' && ch <= '9')
				{
					i = i*10 + (ch - '0');
					card++;
				}
	
				if (i > timelimit)
					return(3);
	
				q->tlimit = i;
				break;
	
			default:
				return(4);
			}

		while (*card++ == ',');

		if ((ch = *--card) == '.' || ch == '.' || ch == ' ')
			return(0);
	}
	return(2);
}
