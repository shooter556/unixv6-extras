#
/*
 *	crd - take cards from reader, buffer them
 *	12 at a time and send them to cyber
 */

#define	EOF	006
#define	EOR	022

#define	QESCP	0276
#define	QEOR	0327
#define	QEOI	0326

char	*jobsu	"/dev/cyb.jobsu";
char	*cr	"/dev/cr";
char	*cmdsu	"/dev/cyb.cmdsu";

main()
{
	char	buffer[81*12];
	register char	*card;
	register	n;
	extern	fout;

	fout = 2;

	close(0); close(1);

	if ( open(cr, 0) != 0 )
		error("Can't open card reader\n");

	if ( open(jobsu, 1) != 1 )
		error("Can't open cyber line\n");

	for (;;)
	{
		while ( (n=open(cmdsu, 1)) < 0 ) sleep(1);
		while ( write(n, "r", 1) != 1 ) sleep(1);
		close(n);

		card = buffer;
		while ( !read(0, buffer, 81) );	/* flush leading eoi's */

		card =+ 80;
		while ( read(0, card, 81) )
			switch ( *card )
			{
			case EOF:
				card[2] = card[3] = '*';
			case EOR:
				*card++ = QESCP;
				*card   = QEOR;
				card[79] = ' ';
			default:
				card =+ 80;
				if (card >= &buffer[960])	/* buffer runneth over */
				{
					while (!write(1, buffer, card-buffer));
					card = buffer;
				}
			}
		/* eoi */
		*card++ = QESCP;
		*card++ = QEOI;
		n = 80;
		do
			*card++ = ' ';
		while (--n);

		while (!write(1, buffer, card-buffer));

		write(1, 0, 0);	/* wait till all gone */
	}
}

error(s)
char	*s;
{
	printf("Crd: %s", s);
	flush();
	exit(1);
}
