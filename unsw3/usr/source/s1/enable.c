char buffer[4];
char *ttyfile           "/etc/ttys";
int fildes;
char writechar '1';	/* changed to '0' for disable */
main (argc, argv)
int argc; char *argv[];
{
	register int i;
	register char *ptr;
	
	if ((fildes = open (ttyfile, 2)) == -1)
	{
		printf ("can't open%s\n", ttyfile);
		exit(1);
	}
	if  (strcomp(*argv, "disable"))
		writechar = '0';

	if (strcomp(*++argv, "all"))  /* all known ttys except console */
	{	ptr = "9abcdefghijklmnoz";
		while (*ptr)
			process (*ptr++);
		kill (1, 1); /* send init a hangup signal */
	}
	else   --argv;

	i = 1;
	for (;--argc;argv[i++])   /* loop on groups of args: abc def gh */
	{       ptr = argv[i];
		while (*ptr) process ( *ptr++);/* loop within group: abcde */
		kill (1, 1); /* send init a hangup signal */
	}
}

process (c)  /* asks if you really want to process "strange" ttys */
char (c);    /* nonstrange ttys are 89abcdefghihjklmnoz  */
{

	if (c < '8' || (c > '9' && c < 'a') || (c > 'o' && c < 'z')  || c > 'z')
	{
		printf ("update tty%c (y/n)? ", c);
		read (0, buffer, 2);
		if (buffer[0] == 'y');
		else return; /* don't process this tty */
	}
	update (c);
}

update (c)      /* edits /etc/ttys */
char c;
{
	while (read (fildes, buffer, 4) == 4) /* until EOF */
	if (buffer[1] == c)
	{
		seek (fildes, -4, 1);      /* backup to beginning of line */
		buffer[0] = writechar;
		write (fildes, buffer, 4); /* write updated line in file */
		seek (fildes, 0, 0);
		return; /* all done with this char, look no further */
	}
}
strcomp(s,t)
	      /* s,t are pointers to char strings;  returns 1
		 if s == t; 0 otherwise 			  */
   char *s, *t;
     {while (*s++ == *t) if (*t++ == '\0') return(1); return(0);}

