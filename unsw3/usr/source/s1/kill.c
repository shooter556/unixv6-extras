
/*
 * kill .. an updated version with ranges implemented.
 *
 *			andrew hume 7/1/77
 */

char nonnum[] "non-numeric arg\n";
int sig 9 ;

main(argc,argv)

char **argv;
{
register char **args;
register pid,upper;

	args = ++argv;
	argc--;

	while(argc--)
		if( **args == '-' )
			{
			*args =+1 ;
			sig = atoi(args);
			if(**args++) error(nonnum);
			if(sig >= 20)
				error("signal too large\n");
			}
		else {
			pid = atoi(args);
			if(**args == '-') {
				*args =+1 ;
				upper = atoi(args);
				if(**args++) error(nonnum);
				if(pid > upper)
					error("lower limit > upper limit\n");
				for(;pid <= upper;pid++) killit(pid,0);
			}
			else	if(**args++) error(nonnum);
				else killit(pid,1);
		}
}

error(s)

char *s;
{
	register char *j;

	for(j = s; *j; j++);
	write(1,s,j-s);
}

killit(pid,flag)

int pid,flag;
{
	if((kill(pid,sig) == -1) && flag)
		printf("%d: not found\n",pid);
}

atoi(s)

char **s;
{
	register char *c;
	register i;

	c = *s;
	i = 0;
	while((*c <= '9') && (*c >= '0')) i = i*10 + *c++ - '0';
	*s = c;
	return(i);
}
