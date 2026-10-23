#

/*
 * Unix 6
 * Editor
 *
 *
 * Mods: George Coulouris (QMC)
 *	March, 76
 *
 *	prompts (suppress with `-p' flag)
 *	", %, &, to display a screen full of context
 *	`x' - as `s' but interactive
 *	`n' flag when appended to `s' or `x' commands
 *		prints number of replacements
 *	% == current file name in ! commands
 *	!! == repeat the last ! command you executed
 *	-e flag == "elfic" mode :-
 *		no "w", "r\n" commands, auto w before q
 *	`o' command for text input with local editing via control keys
 *	`b' to set a threshold for automatic line breaks in `o' mode.
 *	`h' displays a screen full of help with editor commands
 *		(the help is in /usr/lib/emhelp)
 *
 * Mods: Ian Johnstone (AGSM)
 *	September, 77
 *
 *	When receive terminate (14) signal, output current file
 *	to "saved.file" and tell user about this - then exit.
 *
 * Mods: Greg Rose
 *	77/09/12
 */

#define	AUTOW

/*	A new command, T (upper case t) to set the time between
 *	automatic writes of the file.
 *	Syntax: Tn (where n is an integer) will write the file
 *	every n minutes to the current file.
 *	A pair of bells are output to warn the user,
 *	and rubouts are trapped.
 *
 *	Also the pattern matching routines have been improved.
 *	If a new pattern match includes `&', the `&' is
 *	replaced with the old pattern.
 *
 * Mods: Craig McGregor (CSU)
 *	77/12/23
 *
 *	Make numerous fixes and modifications.
 *	1. Allow the % in a shell command line to be
 *	escaped with `\'.
 *	2. Perform % evaluation in a shell command line
 *	when the line is actually executed rather than
 *	the first time it is typed in.  This will allow
 *	it to reflect the current file name all of the time.
 *	3. The `o' command has some slight corrections.
 *	When the command is aborted with a rubout
 *	character it no longer loses the next line.
 *	It no longer fouls up when a null character is
 *	received; it is now ignored.  The error message
 *	has been changed from some obscure character used
 *	on ITT terminals to simply a bell.
 *	4. Always redirect the confirmation input for the
 *	`x' command so that will work even as part of the
 *	`g' command.
 *	5. Allow new lines to be substituted into lines
 *	even when the `g' command is in use.
 *	6. The use of `s', `x' and `o' commands does not
 *	remove labels from lines.  When a line is changed
 *	to more than one, the label is left attached to
 *	the first.  This is also relevent to the `c' command
 *	however only the first line of the change preserves
 *	its tag.
 *	Also it is now possible to place tags on line #1
 *	and use them there.
 *	7. The `o' command modified to use the UCASE bit
 *	before converting "\b" to upper-case B.
 *	8. Make `xl' command correctly handle tabs etc.
 *	9. A switch (-l) to select a default mode of `l'
 *	instead of `p' when a line number or newline is
 *	given without a command.
 *	11. Two new subcommands have been added to the
 *	`o' command's repertoire: ^P to skip to the end
 *	of the line; ^T to backup a single character.
 *	12. Handle long-line errors correctly.
 *	13. Allow the '.' to finish off an append, insert
 *	or change input to be escaped.  This will allow
 *	users to create lines consisting of a single dot.
 *
 * Mods: Kevin Hill
 *	April - May, 78
 */

#define JOIN

/*	1.  Added a new command for joining lines together :
 *
 *		(.,.+1)j
 *
 *	The given lines are joined, with the new line characters
 *	replaced by blanks.  If the total line exceeds the allowed
 *	linelength, the command is aborted without changing the file.
 *	May also type 'jc', for join-and-compress; that is, join the two
 *	(or more) lines together, removing all blanks and tabs at the join
 *	except for one blank.  Any label on the first line is maintained.
 *	Note for future modifiers :  ALL reading operations MUST
 *	be completed before a write is attempted.
 *
 *	2.  Removed modification 13 above, which caused more trouble
 *	than it was worth.  Suggested method of entering '.' :  enter
 *	'..' and later on do 's/.//p'.
 *
 *	3.  Stylistic improvements were made to make the program readable!
 *
 *	4.  '"l', '%l', and '&l' now working.
 */

#define SEARCH_STRING

/*	5.  Incredibly small change made so that the search string is
 *	not forgotten if accidently enter '/' instead of '//', or '?'
 *	instead of '??'.  Another change was made so that filename
 *	is not forgotten if enter 'f ' (or something similar).
 */

#define GLOBAL_DEFAULT

/*	6.  Made the default search-string for global matches '/^/'.
 *	Interesting effects of this are :  'gp' prints all lines in
 *	the file, and 'gs....' executes the substitute command on all lines
 *	BEFORE any substituting takes place (for example, the substitute
 *	may create more lines).  The drawback is that the search-string
 *	delimiters may not be alphabetics.
 */

#define GLOBAL

/*	7.  Print out '??' if no match in a global command pattern
 *	in the entire file.  Note :  do NOT seek!
 */

#define ERRORS

/*	8.  Improved error recovery for command files.  The line number
 *	at which the error occurred is printed.  Assisted by Ian J, who wrote
 *	the verbose error messages as well as catching 'init' errors, and
 *	initially requesting core for INITLINES lines to speed up init.
 *	When invoked as 'e -s file', the verbose error messages are suppressed
 *	in favour of '?' and '??'.  When invoked as 'e - file', match-fail and
 *	empty-file error messages are suppressed (useful for command files).
 */

#define	N_LINES

/*	9.   Added feature to `"', `%', and `&' commands :
 *
 *			`"n'	or	`%n'	or	`&n'
 *
 *	where `n' specifies the screen size, which is remembered until reset.
 *	May be followed by `l' or `p', as before.  The default values is LINES.
 */

#define	NULLS

/*	10.  Tell the user how many nulls have been thrown away when the file
 *	is read in.  Also, add a '\n' onto the last line, if none present,
 *	and inform the user (previously, such a line was discarded).
 */

#define	ADDRESS

/*	11.  Address-reading routine improved to reject rubbish, such as
 *	1,2,3,4,5,6,7p etc. etc. (result same as before for valid addresses).
 */

#define	CHDIR

/*	12.  Added 'cd' to the command list.  Form :  'cd pathname', where
 *	the pathname cannot contain the short-cuts '%' or '\@'.  This is
 *	because /etc/glob could not be used, for obvious (?) reasons.
 */

#define	PRINT_STRING

/*	13.  Added new command 'z' to print out the current search-string.
 *	Added '-' facility to the search string: [a-d] = [abcd].
 *	The '-' may be escaped: [abc\-f] = [abcf] + [-].  For those
 *	who want to use '\' as the first letter of a run - TUFF!
 */

#define	NICE_EXIT

/*	14.  Fixed up 'o' mode so that, if have typed '\n', and then hit
 *	DEL to exit unchanged, will not lose rest of current line.
 */

#define	BETTER_SWAP


/*	15.  Improved the 's' and 'x' algorithm - it was incredibly inefficient
 *	for 'x' and 's/../../g' commands before this change, copying the
 *	linebuffer backwards and forwards numerous times.
 *
 *
 * Mods: David Millway
 *	May, 78
 */

#define	DRM

/*	Added a new option to 'w' command :  'wa' to write onto the end
 *	of the named file.  If the file is not present, it is created.
 */

#define	SIGHUP	1
#define	SIGINTR	2
#define	SIGQUIT	3
#define	SIGTERM	14
#ifdef	AUTOW
#define	SIGCLOK	15
#endif	AUTOW
#define	FNSIZE	64
#define	LBSIZE	512
#ifdef	ERRORS
#define	INITLINES	200	/* initial number of lines */
#endif	ERRORS
#define	ESIZE	128
#define	GBSIZE	256
#define	NBRA	8
#define	EOF	-1
#define	UNIXBUFL 100

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12

#define	STAR	01

#define	READ	0
#define	WRITE	1

/* screen dimensions */
#define	LINES	24
#define	LENGTH	80
#define	SPLIT	'-'
#define	PROMPT	'>'
#define	CONFIRM	'.'
#define	SCORE	"^"
#define	TABSET	7	/* this should be determined dynamically */
#define	clearscreen	putchar(037)

#define	RAW	040
#define	ECHO	010
#define	UCASE	04

#define	OPEN	'/'
#define	BELL	'\007'
#define	ESCAPE	'\033'
#define	SPACE	' '
#define	BACKSL	'\\'
#define	RUBOUT	'\177'
#define	CTRLA	'\01'
#define	CTRLB	'\02'
#define	CTRLC	'\03'
#define	CTRLD	'\04'
#define	CTRLE	'\05'
#define	CTRLF	'\06'
#define	CTRLH	'\010'
#define	CTRLI	'\011'		/* == TAB */
#define	CTRLP	'\020'
#define	CTRLQ	'\021'
#define	CTRLR	'\022'
#define	CTRLS	'\023'
#define	CTRLT	'\024'
#define	CTRLV	'\026'
#define	CTRLW	'\027'
#define	CTRLX	'\030'
#define	CTRLZ	'\032'

int	peekc;
int	lastc;
char	savedfile[FNSIZE];
char	file[FNSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	circfl;
int	*zero;
int	*dot;
int	*dol;
int	*endcore;
int	*fendcore;
int	*addr1;
int	*addr2;
char	genbuf[LBSIZE];
int	count[2];
char	*nextip;
char	*linebp;
int	ninbuf;
int	nline;
int	io;
int	pflag;
int	onhup;
int	onquit;
int	vflag;
int	listf;
int	col;
char	*globp;
int	tfile	-1;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	*locs;
char	ibuff[512];
int	iblock	-1;
char	obuff[512];
int	oblock	-1;
int	ichanged;
int	nleft;
int	names[26];
char	*braslist[NBRA];
char	*braelist[NBRA];

int	margin	LBSIZE-40;	/* used to set threshold in 'open' */

int	elfic	0;		/* true if "elfic" (-e) flag */
int	firstime	1;	/* ugh - used to frigg initial "read" */
char	unixbuffer[UNIXBUFL];
int	xflag	0;		/* used in 'xchange' command */

int	oflag;
char	*threshold, *savethresh;
char	*lnp, *gnp, *brp;
int	savetty, tty[3];

#ifdef	AUTOW
int	writetime;
int	justwrit;	/* for the auto-write */
int	delaywrite;
int	writewaiting;	/* kludge for 'o' mode, since it flogs a buffer */
int	catchclock();
int	terminate();
char	bell	BELL;
#endif	AUTOW

#ifdef	JOIN
int	nocmpress;	/* for join-and-compress option */
#endif	JOIN

#ifdef	ERRORS
int	istty;		/* set when input commands are from a tty */
int	linenr	1;	/* line number of each command as taken from a file */
int	currlnnr;	/* for the current command line number */
int	sflag;		/* for '-s' option */
#endif	ERRORS

#ifdef	N_LINES
int	lines	LINES;		/* number of lines on screen for ", %, and & */
#endif	N_LINES

#ifdef	NICE_EXIT
int	oldline, *thestart;
#endif	NICE_EXIT

#ifdef	BETTER_SWAP
char	*ssp, *slp;	/* pointers to genbuf and linebuf */
#endif	BETTER_SWAP

char	lflag;

main(argc, argv)
char **argv;
{
	register char *p1, *p2;
#ifdef	ERRORS
	register eflag;
#endif	ERRORS
	extern int onintr();

	onquit = signal(SIGQUIT, 1);
#ifdef	AUTOW
	signal(SIGCLOK, catchclock);
#endif	AUTOW
	signal(SIGTERM, terminate);
	onhup = signal(SIGHUP, 0);
	if ((*argv)[1] == 'm') vflag = 1;
	argv++;
	if (argc > 1 && **argv=='-')
	{
		p1 = &(*argv)[1];
		while (*p1 != '\0')
			switch (*p1++)
			{

		    case 'e':	elfic = 1;
				break;

		    case 'l':	lflag++;
				break;

		    case 'p':	vflag = 0;
				break;

#ifndef	ERRORS
		    case 's':	vflag = -1;
#else
		    case 's':	sflag++;
#endif	ERRORS
				break;
			}
		if ((*argv)[1] == '\0') vflag = -1;
		argv++;
		argc--;
	}
	if (argc>1)
	{
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++);
		breaks(p1 - 3);
		globp = "r";
	}
	fendcore = sbrk(0);
	if (vflag > 0) puts("Editor");
#ifdef	ERRORS
	fstat(0,linebuf);		/* flogs line-buffer for status info */
	if ((linebuf[5] & 0140) == 040) istty++;
	eflag = 0;
	setexit();		/* catch init errors */
	if (eflag) exit(1);
	eflag = 1;
#endif	ERRORS
	init();
	if ((signal(SIGINTR, 1) & 01) == 0) signal(SIGINTR, onintr);
	setexit();
	commands(vflag);
	unlink(tfname);
	exit(0);
}

terminate()
{
	signal(SIGTERM, 1);
	if (dol != zero) writeout("saved.file");
	puts("System going down - tmp file written to \"saved.file\""); 
	unlink(tfname);
	exit(1);
}

#ifdef	AUTOW
catchclock()
{
	register int	ad1, ad2, oldintr;
	int	csav[2];

	if (delaywrite) { writewaiting++; return; }
	signal(SIGCLOK, catchclock);
	justwrit++;
	clktim(writetime);
	if (dol == zero) return;		/* empty file */
	oldintr = signal(SIGINTR, 1);
	ad1 = addr1;
	ad2 = addr2;
	csav[0] = count[0];
	csav[1] = count[1];
	addr1 = zero+1;
	addr2 = dol;
	write(2, &bell, 1);
	writeout(savedfile);
	write(2, &bell, 1);
	addr1 = ad1;
	addr2 = ad2;
	count[0] = csav[0];
	count[1] = csav[1];
	signal(SIGINTR, oldintr);
}
#endif	AUTOW

commands(prompt)
{
	int getfile(), gettty();
	register *a1, c;
	register char *p;
	int r, n;
	int fd;

	for (;;)
	{

#ifdef	ERRORS
		if (prompt != -2) currlnnr = linenr;	/* i.e., not global */
#endif	ERRORS

		if (pflag)
		{
			pflag = 0;
			addr1 = addr2 = dot;
			goto print;
		}
		if (prompt>0 && globp==0) putch(PROMPT);
		addr1 = addr2 = 0;
		xflag = 0;
#ifdef	AUTOW
		if (writewaiting)
		{
			delaywriting = writewaiting = 0;
			catchclock();
			justwrit = 0;
		}
#endif	AUTOW
#ifndef	ADDRESS
		do
		{
			addr1 = addr2;
			if ((a1 = address())==0) { c = getchar(); break; }
			addr2 = a1;
			if ((c=getchar()) == ';') { c = ','; dot = a1; }
		} while (c==',');
#else
		a1 = address();
		if ((c = getchar()) == ';' && a1)
		{ dot = addr1 = addr2 = a1; a1 = address(); c = getchar(); }
		if (a1) if (c == ',')
			{
				addr1 = a1;
				if ((a1 = address()) == 0) error(4);
				else addr2 = a1;
				c = getchar();
			}
			else addr2 = a1;
#endif	ADDRESS
		if (addr1==0) addr1 = addr2;
#ifndef	AUTOW
		if (c>='A' && c<='Z') c =| 040;
#else
		if (c>='A' && c<='Z' && c!= 'T') c =| 040;
#endif	AUTOW
		switch(c)
		{
	
	    case 'a':	setdot();
			newline();
			append(gettty, addr2);
			continue;
	
	    case 'b':	c = peekc = getchar();
			if (c=='+' || c=='-') peekc = 0;
			else if (c != '\n') error(0);
			margin = c == '-' ? LBSIZE - 40 : LENGTH - 20;
			newline();
			continue;
	
#ifndef	CHDIR
	    case 'c':	setdot();
#else
	    case 'c':	if (addr2 == 0 && (peekc = getchar()) == 'd')
			{
				getchar(); if (getchar() != ' ') error(0);
				do c = getchar(); while (c == ' ');
				if (c == '\n') error(0);
				p = linebuf;
				do *p++ = c; while ((c = getchar()) != '\n');
				*p = '\0';
				if (chdir(linebuf)) error(19);
				continue;
			}
			setdot();
#endif	CHDIR
			newline();
			fd = *addr1;		/* borrow 'fd' */
			delete();
			if (append(gettty, addr1-1) > 0) retag(*addr1, fd);
			continue;
	
	    case 'd':	setdot();
			newline();
			delete();
			continue;
	
	    case 'e':	if (elfic) error(3);
			setnoaddr();
			if ((peekc = getchar()) != ' ') error(0);
			savedfile[0] = 0;
			init();
			addr2 = zero;
			goto caseread;
	
	    case 'f':	if (elfic) error(3);
			setnoaddr();
			if ((c = getchar()) != '\n')
			{
				peekc = c;
#ifndef	SEARCH_STRING
				savedfile[0] = 0;
#endif	SEARCH_STRING
				filename(1);
			}
			puts(savedfile);
			continue;
	
	    case 'g':	global(1);
			continue;
	
	    case 'h':	newline();
			if ((fd = open("/usr/lib/emhelp", 0)) < 0)
			{
				puts("/usr/lib/emhelp not found");
				continue;
			}
			while ((n = read(fd, linebuf, 512)) > 0) write(1, linebuf, n);
			close(fd);
			continue;
	
	    case 'i':	setdot();
			nonzero();
			newline();
			append(gettty, addr2-1);
			continue;
	
#ifdef	JOIN
	    case 'j':	if (addr2 == 0) addr2 = (addr1 = dot) + 1;
			if (addr1 > addr2) error(4);
			if (addr1 == addr2) addr2++;
			nonzero();
			nocmpress = 1;
			if ((peekc = getchar()) == 'c') { nocmpress--; peekc = 0; }
			newline();
			join();
			continue;
#endif	JOIN
	
	    case 'k':	if ((c = getchar()) < 'a' || c > 'z') error(5);
			newline();
			setdot();
			nonzero();
			names[c-'a'] = *addr2 | 01;
			continue;
	
	    case 'm':	move(0);
			continue;
	
	    case '\n':	if (addr2==0) addr2 = dot+1;
			addr1 = addr2;
			if (lflag) listf++;
			goto print;

	    case 'l':	listf++;
	    case 'p':	newline();
	    print:	setdot();
			nonzero();
			a1 = addr1;
			do puts(getline(*a1++)); while (a1 <= addr2);
			dot = addr2;
			listf = 0;
			continue;
	
	    case 'o':	setdot();
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
			op(globp);
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
	    case 'q':	setnoaddr();
			newline();
			if (elfic) { firstime = 1; writeout(0); }
			unlink(tfname);
			exit(0);
	
	    case 'r':
	    caseread:	filename(0);
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
			if ((io = open(file, 0)) < 0) { lastc = '\n'; error(2); }
			setall();
			ninbuf = 0;
			append(getfile, addr2);
			exfile();
#ifdef	NULLS
			if (listf && vflag >= 0)		/* borrows 'listf' */
			{
				count[1] = listf; putchar('('); putd();
				puts(" null(s) stripped from file)");
			}
			listf = 0;
#endif	NULLS
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
	    case 'x':	xflag = 1;
	    case 's':	setdot();
			nonzero();
			substitute(globp);
			xflag = 0;
			continue;
	
#ifdef	AUTOW
	    case 'T':	fd = 0;			/* upper case t ... for timeout */
			while (((c = getchar()) >= '0') && c<='9')
				fd = fd*10 + c - '0';
			if (c != '\n') error(0);
			writetime = fd * 60;
			clktim(writetime);
			continue;
#endif	AUTOW
	
	    case 't':	move(1);
			continue;
	
	    case 'v':	global(0);
			continue;
	
	    case 'w':	if (elfic) error(3);
#ifdef	AUTOW
			delaywrite++;
#endif	AUTOW
#ifndef	DRM
			writeout(0);
#else
			if ((peekc = getchar()) == 'a') { peekc = 0; writeout(-1); }
			else writeout(0);
#endif	DRM
#ifdef	AUTOW
			delaywrite = 0;
#endif	AUTOW
			continue;
	
#ifdef	PRINT_STRING
	    case 'z':	if (getchar() != '\n' || addr2) error(0);
			if (expbuf[0]) printstring();
			else puts("Not a legal search string");
			continue;
#endif	PRINT_STRING

	    case '"':	setdot();
#ifdef	N_LINES
			fetchnr(0);
#endif	N_LINES
			newline();
			if (addr1 < zero) error(4);
			dot = addr1;
			if (dot == dol) error(4);
			addr1 = dot + 1;
#ifndef	N_LINES
			addr2 = dot + LINES - 1;
#else
			addr2 = dot + lines - 1;
#endif	N_LINES
			if (addr2 > dol) addr2 = dol;
	    outlines:	pflag = listf; listf = 0;
			clearscreen;
			listf = pflag;
			for (a1 = addr1; a1 <= addr2; a1++) puts(getline(*a1));
			dot = addr2;
			pflag = listf = 0;
			continue;
	
	    case '&':	setdot();
#ifdef	N_LINES
			fetchnr(0);
#endif	N_LINES
			newline();
			nonzero();
			dot = addr1;
#ifndef	N_LINES
			addr1 = dot - (LINES-2);
#else
			addr1 = dot - (lines-2);
#endif	N_LINES
			addr2 = dot;
			if (addr1 <= zero) addr1 = zero + 1;
			goto outlines;
	
	    case '%':
#ifdef	N_LINES
			fetchnr(1);
#endif	N_LINES
			newline();
			pflag = listf; listf = 0;
			setdot();
			nonzero();
			dot = addr1;
#ifndef	N_LINES
			addr1 = dot - (LINES/2 - 2);
			addr2 = dot + (LINES/2 - 2);
#else
			addr1 = dot - (lines/2 - 2);
			addr2 = dot + (lines/2 - 2);
#endif	N_LINES
			if (addr1 <= zero) addr1 = zero + 1;
			if (addr2 > dol) addr2 = dol;
			clearscreen;
			listf = pflag;
			for (a1 = addr1; a1 <= addr2; a1++)
			{
				if (a1 == dot) screensplit();
				puts(getline(*a1));
				if (a1 == dot) screensplit();
			}
			pflag = listf = 0;
			continue;
	
	    case '>':	if (vflag > 0) vflag = 0;
			reset();
	
	    case '<':	vflag = 1;
			reset();
	
	    case '=':	setall();
			newline();
			count[1] = (addr2-zero)&077777;
			putd();
			putchar('\n');
			continue;
	
	    case '!':	unix();
			continue;
	
#ifndef	ERRORS
	    case EOF:	if (prompt == -2 || ttyn(0) == 'x') return;
#else
	    case EOF:	if (prompt == -2 || ! istty) return;
#endif	ERRORS
			continue;
	
		}
		error(0);
	}
}

writeout(filen)
char *filen;
{
	register char *p1, *p2;

#ifdef	DRM
	register int appendf;

	appendf = 0;
	if (filen == -1) { appendf++; filen = 0; }
#endif	DRM

	setall();
	nonzero();
	if (filen == 0)
	{
		if (elfic)
		{
			p1 = savedfile;
			if (*p1 == 0) error(6);
			p2 = file;
			while (*p2++ = *p1++);
		}
		else filename(0);
		filen = file;
	}
#ifndef	DRM
	if ((io = creat(filen, 0600)) < 0) error(7);
#else
	if (appendf && (io = open(filen, 1)) >= 0) seek(io, 0, 2);
	else if ((io = creat(filen, 0600)) < 0) error(7);
#endif	DRM
	putfile();
	exfile();
}

address()
{
	register *a1, minus, c;
	int n, relerr;

	minus = 0;
	a1 = 0;
	for (;;)
	{
		c = getchar();
		if ('0'<=c && c<='9')
		{
			n = 0;
			do
			{
				n =* 10;
				n =+ c - '0';
			} while ((c = getchar())>='0' && c<='9');
			peekc = c;
			if (a1==0) a1 = zero;
			if (minus<0) n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus) relerr++;
		switch(c)
		{

	    case ' ':
	    case '\t':	continue;
	
	    case '+':	minus++;
			if (a1==0) a1 = dot;
			continue;

	    case '-':
	    case '^':	minus--;
			if (a1==0) a1 = dot;
			continue;
	
	    case '?':
	    case '/':	compile(c);
			a1 = dot;
			for (;;)
			{
				if (c=='/') { a1++; if (a1 > dol) a1 = zero; }
				else if (--a1 < zero) a1 = dol;
				if (execute(0, a1)) break;
				if (a1==dot) error(1);
			}
			break;
	
	    case '$':	a1 = dol;
			break;
	
	    case '.':	a1 = dot;
			break;

	    case '\'':	if ((c = getchar()) < 'a' || c > 'z') error(5);
			for (a1 = zero+1; a1 <= dol; a1++)
				if (names[c-'a'] == (*a1|01)) break;
			break;
	
	    default:	peekc = c;
			if (a1==0) return(0);
			a1 =+ minus;
			if (a1<zero || a1>dol) error(4);
			return(a1);
		}
		if (relerr) error(0);
	}
}

setdot()
{
	if (addr2 == 0) addr1 = addr2 = dot;
	if (addr1 > addr2) error(4);
}

setall()
{
	if (addr2==0)
	{
		addr1 = zero+1;
		addr2 = dol;
		if (dol==zero) addr1 = zero;
	}
	setdot();
}

setnoaddr()
{
	if (addr2) error(0);
}

nonzero()
{
	if (addr1<=zero || addr2>dol) error(4);
}

newline()
{
	register c;

	if ((c = getchar()) == '\n') return;
	if (c=='p' || c=='l' || c=='P' || c=='L')
	{
		pflag++;
		if (c=='l' || c=='L') listf++;
		if (getchar() == '\n') return;
	}
	error(0);
}

#ifdef	N_LINES
fetchnr(flg)
{
	register int n;
	register char c;

	if ((c = getchar()) >= '0' && c <= '9')
	{
		n = 0;
		do
		{
			n =* 10;
			n =+ c - '0';
		} while ((c = getchar()) >= '0' && c <= '9');
		lines = n;
	}
	peekc = c;
}
#endif	N_LINES

retag (newlabel, oldlabel)
{
	register int	ol, nl;
	register int	*ip;

	ol = oldlabel | 01;
	nl = newlabel | 01;
	for (ip = &names[0]; ip < &names[26]; ip++)
		if (*ip == ol) *ip = nl;
}

filename(flag)
{
	register char *p1, *p2;
	register c;

	count[1] = 0;
	c = getchar();
	if (c=='\n' || c==EOF)
	{
		if (elfic && !firstime) error(3); else firstime = 0;
		p1 = savedfile;
		if (*p1==0) error(6);
		p2 = file;
		while (*p2++ = *p1++);
		return;
	}
	if (c!=' ') error(0);
	while ((c = getchar()) == ' ');
	if (c=='\n') error(0);
	p1 = file;
	do *p1++ = c; while ((c = getchar()) != '\n');
	*p1++ = 0;
#ifndef	SEARCH_STRING
	if (savedfile[0]==0)
#else
	if (flag || savedfile[0]==0)
#endif	SEARCH_STRING
	{
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++);
		breaks(p1 - 3);
	}
}

breaks(p)
register char *p;
{
	if (*p++ == '.' && (*p == 'r' || *p == 'n')) margin = LENGTH - 20;
}

exfile()
{
	close(io);
	io = -1;
#ifndef	AUTOW
	if (vflag >= 0)
	{
#else
	if (vflag>=0 && !justwrit)
	{
#endif	AUTOW
		putd();
		putchar('\n');
	}
}

onintr()
{
	signal(SIGINTR, onintr);
	putchar('\n');
	lastc = '\n';
	error(8);
}

error(type)
{
	register c;

#ifdef	ERRORS

	register char *sp;

static struct
{
	char *message;
	int	flags;
} msgs[] {
	"Syntax"	,0,	/* type  0 == syntax (+ others)		*/
	"No match"	,0,	/* type  1 == match fail		*/
	"Open:"		,1,	/* type  2 == file non-existant		*/
	"Elfic"		,0,	/* type  3 == not allowed in elfic 	*/
	"Address"	,0,	/* type  4 == illegal address(s)	*/
	"Reg name"	,0,	/* type  5 == invalid register name	*/
	"File"		,0,	/* type  6 == elfic - no file specified */
	"Create:"	,1,	/* type  7 == creat fails		*/
	"Interrupt"	,0,	/* type  8 == interrupted		*/
	"Line size"	,0,	/* type  9 == line too big		*/
	"Write:"	,1,	/* type 10 == write error		*/
	"Out of core"	,0,	/* type 11 == out of core		*/
	"Temp size"	,0,	/* type 12 == out of temp space		*/
	"Gtty:"		,1,	/* type 13 == gtty failed for std input	*/
	"Buffer empty"	,0,	/* type 14 == null file			*/
	"Fork:"		,1,	/* type 15 == fork failed 		*/
	"Temp i/o:"	,1,	/* type 16 == i/o on temp file failed   */
	"Create temp:"	,1,	/* type 17 == creat temp file failed 	*/
	"Open temp:"	,1,	/* type 18 == open temp file failed	*/
	"Chdir:"	,1,	/* type 19 == chdir failed		*/
};

extern	char *sys_errlst[];
extern	int   sys_nerr;
extern	int   errno;

#endif	ERRORS

	listf = count[0] = 0;

#ifndef	ERRORS
	if (type==1) puts("??"); else puts("?");
	seek(0, 0, 2);
#else
	pflag = (type != 1 && type != 2);	/* flog 'pflag' as cleared later anyway */
	if (! istty)
	{
		if (vflag >= 0 || pflag)
		{ count[1] = currlnnr; putd(); putchar(' '); }
		if (pflag) seek(0,0,2);
	}
	if (sflag) { if (type == 1) puts("??"); else puts("?"); }
	else if (vflag >= 0 || pflag)
	{
		if (type == 4 && dol==zero) type = 14;
		if (type > sizeof msgs / sizeof msgs[0] - 1) sp = "ERR";
			else sp = msgs[type].message;
		while (*sp) putchar(*sp++);
		if (msgs[type].flags)
		{
			if (errno < sys_nerr)
			{
				sp = sys_errlst[errno];
				putchar(' ');
				while (*sp) putchar(*sp++);
			}
		}
		puts(" ?");
	}
#endif	ERRORS

	pflag = 0;
	if (globp) lastc = '\n';
	globp = 0;
	peekc = lastc;
	while ((c = getchar()) != '\n' && c != EOF);
	if (io > 0) { close(io); io = -1; }
	reset();
}

getchar()
{
	if (lastc=peekc) { peekc = 0; return(lastc); }
	if (globp)
	{
		if ((lastc = *globp++) != 0) return(lastc);
		globp = 0;
		return(EOF);
	}
#ifndef	AUTOW
	if (read(0, &lastc, 1) <= 0) return(lastc = EOF);
#else
reread:	if (read(0, &lastc, 1) <= 0)
	{
		if (justwrit) { justwrit = 0; goto reread; }
		return(lastc = EOF);
	}
#endif	AUTOW
	lastc =& 0177;
#ifdef	ERRORS
	if (! istty && lastc=='\n') linenr++;
#endif	ERRORS
	return(lastc);
}

gettty()
{
	register c, gf;
	register char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchar()) != '\n')
	{
		if (c==EOF) { if (gf) peekc = c; return(c); }
		if ((c =& 0177) == 0) continue;
		*p++ = c;
		if (p >= &linebuf[LBSIZE-2]) error(9);
	}
	*p++ = 0;
	if (linebuf[0]=='.' && linebuf[1]==0) return(EOF);
	return(0);
}

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do
	{
		if (--ninbuf < 0)
		{
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0)
#ifndef	NULLS
				return(EOF);
#else
				if (lp == linebuf) return(EOF);
				else
				{
					if (vflag >= 0) puts("No '\\n' on last line - '\\n' added.");
					if (lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(9); }
					*lp = 0;
					return(0);
				}
#endif	NULLS
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(9); }
#ifndef	NULLS
		if ((*lp++ = c = *fp++ & 0177) == 0) { lp--; continue; }
#else
		if ((*lp++ = c = *fp++ & 0177) == 0) { lp--; listf++; continue; }
#endif	NULLS
		if (++count[1] == 0) ++count[0];
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

putfile()
{
	int *a1;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do
	{
		lp = getline(*a1++);
		for (;;)
		{
			if (--nib < 0)
			{
				nib = fp - genbuf;
				if (write(io, genbuf, nib) != nib) error(10);
				nib = 511;
				fp = genbuf;
			}
			if (++count[1] == 0) ++count[0];
			if ((*fp++ = *lp++) == 0) { fp[-1] = '\n'; break; }
		}
	} while (a1 <= addr2);
	nib = fp - genbuf;
	if (write(io, genbuf, nib) != nib) error(10);
}

append(f, a)
int (*f)();
{
	register *a1, *a2, *rdot;
	int tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)() == 0)
	{
		if (dol >= endcore)
		{
			if (sbrk(1024) == -1) error(11);
			endcore.integer =+ 1024;
		}
		tl = putline(linebuf);
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot) *--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

unix()
{
	register savint, pid, rpid;
	int retcode;
	char c, *lp, *fp;
	char	workbuffer[UNIXBUFL];

	pid = 0;
	c = getchar();
	if (c != '!')
	{
		lp = unixbuffer;
		for (;;)
		{
			if (c == '\n')
				if (lp!=unixbuffer && lp[-1]==BACKSL) lp--;
				else break;
			*lp++ = c;
			c = getchar();
		}
		*lp = '\0';
	}
	else { pid = 1; while (getchar() != '\n'); }
	lp = workbuffer;
	fp = unixbuffer;
	while ((c = *fp++) != '\0')
	{
		if (c == '%')
			if (lp!=unixbuffer && lp[-1]==BACKSL) lp--;
			else
			{
				pid = 1;
				for (rpid = 0; savedfile[rpid] != '\0'; rpid++)
					*lp++ = savedfile[rpid];
				continue;
			}
		*lp++ = c;
	}
	*lp = '\0';
	if (pid) { putchar('!'); puts(workbuffer); }
	setnoaddr();
	if ((pid = fork()) == 0)
	{
		signal(SIGHUP, onhup);
		signal(SIGQUIT, onquit);
		execl("/bin/sh", "sh", "-c", workbuffer, 0);
		exit(1);
	}
#ifdef	ERRORS
	if( pid == -1 ) error(15);	/* fork failed */
#endif	ERRORS
	savint = signal(SIGINTR, 1);
#ifndef	AUTOW
	while ((rpid = wait(&retcode)) != pid && rpid != -1);
#else
	while ((rpid = wait(&retcode)) != pid);
#endif	AUTOW
	signal(SIGINTR, savint);
	puts("!");
}

#ifdef	JOIN
join()
{
	register char *bp, *lp;
	register nl;
	int tl, toggle, *lptr;

	lp = linebuf; toggle = 1;
	for (lptr = addr1; lptr <= addr2; lptr++)
	{
		tl = *lptr;
		bp = getblock(tl,READ); nl = nleft; tl =& ~0177;
		while (*lp = *bp++)
		{
			if (toggle || nocmpress) lp++;
			else if (*lp != ' ' && *lp != CTRLI) { toggle++; lp++; }
			if (--nl == 0) { bp = getblock(tl=+0200,READ); nl = nleft; }
			if (lp >= &linebuf[LBSIZE-2]) error(9);
		}
		if (lp >= &linebuf[LBSIZE-1]) error(9);
		while (--lp >= linebuf && (*lp==' ' || *lp==CTRLI) && ! nocmpress);
		*++lp = ' '; lp++; toggle = 0;
	}
	*(lp - 1) = 0;
	retag(*addr1 = putline(linebuf), *addr1);
	addr1++;
	if (addr2 == dol) delete(); else { delete(); dot--; }
}
#endif	JOIN

delete()
{
	register *a1, *a2, *a3;

	nonzero();
	a1 = addr1;
	a2 = addr2+1;
	a3 = dol;
	dol =- a2 - a1;
	do *a1++ = *a2++; while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol) a1 = dol;
	dot = a1;
}

getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~0177;
	while (*lp++ = *bp++)
		if (--nl == 0)
		{
			bp = getblock(tl=+0200, READ);
			nl = nleft;
		}
	return(linebuf);
}

putline(buffer)
char	buffer[];
{
	register char *bp, *buff;
	register nl;
	int tl;

	buff = buffer;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~0177;
	while (*bp = *buff++)
	{
		if (*bp++ == '\n')
		{
			*--bp = 0;
			linebp = buff;
			break;
		}
		if (--nl == 0)
		{
			bp = getblock(tl=+0200, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((buff-buffer)+07)>>2)&077776;
	return(nl);
}

getblock(atl, iof)			/* format of core words :		*/
{					/*	xbbbbbbbboooooog		*/
	extern read(), write();		/* where :  x is unused;		*/
	register bno, off;		/*	b is block nr. in temp. file;	*/
					/*	o is (offset in block) / 8;	*/
	bno = (atl>>7)&0777;		/*	g is set on global matches	*/
	off = (atl<<2)&0770;
	if (bno >= 511) error(12);
	nleft = 512 - off;
	if (bno==iblock) { ichanged =| iof; return(ibuff+off); }
	if (bno==oblock) return(obuff+off);
	if (iof==READ)
	{
		if (ichanged) blkio(iblock, ibuff, write);
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff+off);
	}
	if (oblock>=0) blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff+off);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) error(16);
}

init()
{
	register char *p;
	register pid;

	close(tfile);
	tline = 0;
	iblock = -1;
	oblock = -1;
	tfname = "/tmp/exxxxx";
	ichanged = 0;
	pid = getpid();
	for (p = &tfname[11]; p > &tfname[6];)
	{
		*--p = (pid&07) + '0';
		pid =>> 3;
	}
	if ((tfile = creat(tfname, 0600)) < 0) error(17);
	close(tfile);
	if ((tfile = open(tfname, 2)) < 0) error(18);
#ifndef	ERRORS
	brk(fendcore);
	endcore = fendcore - 2;
#else
	brk(fendcore+INITLINES);
	endcore = fendcore - 2 + INITLINES;
#endif	ERRORS
	dot = zero = dol = fendcore;
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[GBSIZE];

#ifdef	GLOBAL_DEFAULT
	int dflt;

	dflt = 0;
#endif	GLOBAL_DEFAULT

	if (globp) error(0);
	setall();
	nonzero();
	if ((c=getchar())=='\n') error(0);
#ifndef	GLOBAL_DEFAULT
	compile(c);
#else
	if (c>'z' || c<'A' || c>'Z' && c<'a') compile(c);
	else { peekc = c; dflt++; }
#endif	GLOBAL_DEFAULT
	gp = globuf;
	while ((c = getchar()) != '\n')
	{
		if (c==EOF) error(0);
		if (c==BACKSL)
		{
			c = getchar();
			if (c!='\n') *gp++ = BACKSL;
		}
		*gp++ = c;
		if (gp >= &globuf[GBSIZE-2]) error(9);
	}
	*gp++ = '\n';
	*gp++ = 0;
#ifdef	GLOBAL
	c = 0;
#endif	GLOBAL
	for (a1=zero; a1<=dol; a1++)
	{
		*a1 =& ~01;
#ifndef	GLOBAL_DEFAULT
		if (a1>=addr1 && a1<=addr2 && execute(0, a1)==k)
#else
		if (a1>=addr1 && a1<=addr2 && (dflt || execute(0, a1)==k))
#endif	GLOBAL_DEFAULT
#ifndef	GLOBAL
			*a1 =| 01;
#else
			{ *a1 =| 01; c++; }
#endif	GLOBAL
	}
#ifdef	GLOBAL
	if (c == 0) error(1);
#endif	GLOBAL
	for (a1=zero; a1<=dol; a1++)
	{
		if (*a1 & 01)
		{
			*a1 =& ~01;
			dot = a1;
			globp = globuf;
			commands(-2);
			a1 = zero;
		}
	}
}

substitute(inglob)
{
#ifdef	BETTER_SWAP
	register char *sp, *lp;
#endif	BETTER_SWAP
	register *a1;
	int gsubf;
	int nn;
	int nflag;
	int getsub();

	gsubf = compsub();
	nflag = gsubf > 1;
	nn = 0;
	gsubf =& 01;
	gsubf =| xflag;
	for (a1 = addr1; a1 <= addr2; a1++)
	{
		if (execute(0, a1)==0) continue;
		inglob =| 01;
#ifdef	BETTER_SWAP
		ssp = genbuf; slp = linebuf;
#endif	BETTER_SWAP
		if (confirmed()) { dosub(); nn++; }
		else donothing();
		if (gsubf)
		{
			while (*loc2)
			{
				if (execute(1)==0) break;
				if (confirmed()) { dosub(); nn++; }
				else donothing();
			}
		}
#ifdef	BETTER_SWAP
		sp = ssp; lp = slp;
		while (*sp++ = *lp++) if (sp >= &genbuf[LBSIZE]) error(9);
		retag(*a1 = putline(genbuf), *a1);
#else
		retag(*a1 = putline(linebuf), *a1);
#endif	BETTER_SWAP
		append(getsub, a1);
		a1 =+ nline;
		addr2 =+ nline;
	}
	if (inglob==0) error(1);
	if (nflag)
	{
		putchar(' ');
		count[1] = nn;
		putd();
		putchar('\n');
	}
}

donothing()
{
	char t1, t2;

	t1 = rhsbuf[0];
	t2 = rhsbuf[1];
	rhsbuf[0] = '&';
	rhsbuf[1] = 0;
	dosub();
	rhsbuf[0] = t1;
	rhsbuf[1] = t2;
}

confirmed()
{
#ifdef	BETTER_SWAP
	register char *ptr;
#endif	BETTER_SWAP
	int	ch;
	int	oldglobp;

	if (xflag)
	{
#ifndef	BETTER_SWAP
		puts(linebuf);
#else
		col = 0;
		for (ptr = genbuf; ptr < ssp; putchar(*ptr++));
		for (ptr = slp; *ptr; putchar(*ptr++));
		putchar('\n');
#endif	BETTER_SWAP
		underline();
		oldglobp = globp;
		globp = 0;
		ch = getchar();
		if (ch != '\n')
		{
			while (getchar() != '\n');
			if (ch != CONFIRM) puts("? '.' to confirm");
		}
		globp = oldglobp;
		return(ch == CONFIRM);
	}
	return(1);
}

underline()
{
	int	i1, i2;
	register int	i, j;

	i1 = calccol(loc1);
	i2 = calccol(loc2);
	if (!listf && i1>i2) { i = i1; i1 = i2; i2 = i; }
	j = i1 & ~7;
	for (i = 0; i < j; i =+ 8) write(1, "\t", 1);
	for (; i < i1; i++) write(1, " ", 1);
	while (i != i2)
	{
		if (++i>=LENGTH && listf) { write(1, "\r", 1); i = 1; }
		write(1, SCORE, 1);
	}
}

screensplit()
{
	register a;

	a = LENGTH - 2;
	col = 0;
	while (a--) putchar(SPLIT);
	putchar('\n');
}

				/*
				 * Find actual column number
				 */
calccol(ll)
char	*ll;
{
	register int	i;
	register char	*p;

	i = 0;
#ifndef	BETTER_SWAP
	for (p = linebuf; *p!='\0' && p<ll; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
#else
	for (p = genbuf; p < ssp; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
	for (p = slp; *p!='\0' && p<ll; p++)
	{
		if (*p=='\t' && !listf) i = (i+8) & ~7;
		else if (*p=='\b' && !listf) i--; else i++;
	}
#endif	BETTER_SWAP
	if (listf) i =% 71;
	return(i);
}

compsub()
{
	register seof, c;
	register char *p;
	int gsubf;

	gsubf = 0;
	if ((seof = getchar()) == '\n') error(0);
	compile(seof);
	p = rhsbuf;
	for (;;)
	{
		c = getchar();
		if (c==BACKSL) c = getchar() | 0200;
		if (c=='\n' && globp == 0) error(0);
		if (c==seof) break;
		*p++ = c;
		if (p >= &rhsbuf[LBSIZE/2]) error(9);
	}
	*p++ = 0;
	if (((peekc = getchar()) | 040) == 'g') { peekc = 0; gsubf =| 1; }
	if (((peekc = getchar()) | 040) == 'n') { peekc = 0; gsubf =| 2; }
	newline();
	return(gsubf);
}

getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0) return(EOF);
	while (*p1++ = *p2++);
	linebp = 0;
	return(0);
}

dosub()
{
	register char *lp, *sp, *rp;
	int c;

#ifndef	BETTER_SWAP
	lp = linebuf;
	sp = genbuf;
#else
	lp = slp;
	sp = ssp;
#endif	BETTER_SWAP
	rp = rhsbuf;
	while (lp < loc1) *sp++ = *lp++;
	while (c = *rp++)
	{
		if (c=='&') { sp = place(sp, loc1, loc2); continue; }
		else if (c<0 && (c =& 0177) >='1' && c < NBRA+'1')
		{
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[LBSIZE]) error(9);
	}
#ifndef	BETTER_SWAP
	lp = loc2;
	loc2 = sp + linebuf - genbuf;
	while (*sp++ = *lp++) if (sp >= &genbuf[LBSIZE]) error(9);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++);
#else
	slp = loc2;
	ssp = sp;
#endif	BETTER_SWAP
}

place(asp, al1, al2)
{
	register char *sp, *l1, *l2;

	sp = asp;
	l1 = al1;
	l2 = al2;
	if (sp > &genbuf[LBSIZE - (l2 - l1)]) error(9);
	while (l1 < l2) *sp++ = *l1++;
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	setdot();
	nonzero();
	if ((adt = address())==0) error(4);
	newline();
	ad1 = addr1;
	ad2 = addr2;
	if (cflag) { ad1 = dol; append(getcopy, ad1++); ad2 = dol; }
	ad2++;
	if (adt<ad1)
	{
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1) return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	}
	else if (adt >= ad2)
	{
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	}
	else error(4);
}

reverse(aa1, aa2)
{
	register int *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;)
	{
		t = *--a2;
		if (a2 <= a1) return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2) return(EOF);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[NBRA], *bracketp;
	int nbra;
	int cclcnt;
#ifdef	AUTOW
	char	tempbuf[ESIZE];
	int	savcirc;
	char	*ep1;
#endif	AUTOW
#ifdef	PRINT_STRING
	char	fstch;
#endif	PRINT_STRING

	ep = expbuf;
	eof = aeof;
#ifndef	AUTOW
	bracketp = bracket;
#endif	AUTOW
	nbra = 0;
#ifdef	AUTOW
	bracketp = bracket;
#endif	AUTOW
	if ((c = getchar()) == eof) { if (*ep==0) error(0); return; }
#ifdef	AUTOW
	ep1 = tempbuf;
	if (*ep) while (*ep != CEOF) *ep1++ = *ep++;
	*ep1++ = CEOF;
	ep = expbuf;
	savcirc = circfl;
#endif	AUTOW
	circfl = 0;
	if (c=='^') { c = getchar(); circfl++; }
	if (c=='*') goto cerror;
#ifdef	SEARCH_STRING
	if (c=='\n' && *ep) error(0);
#endif	SEARCH_STRING
	peekc = c;
	for (;;)
	{
		if (ep >= &expbuf[ESIZE]) goto cerror;
		c = getchar();
		if (c==eof) { *ep++ = CEOF; return; }
		if (c!='*') lastep = ep;
		switch (c)
		{

	    case BACKSL:if ((c = getchar())=='(')
			{
				if (nbra >= NBRA) goto cerror;
#ifndef	AUTOW
				*bracketp++ = nbra;
#endif	AUTOW
				*ep++ = CBRA;
#ifdef	AUTOW
				*bracketp++ = nbra;
#endif	AUTOW
				*ep++ = nbra++;
				continue;
			}
			if (c == ')')
			{
#ifndef	AUTOW
				if (bracketp <= bracket) goto cerror;
#endif	AUTOW
				*ep++ = CKET;
#ifdef	AUTOW
				if (bracketp <= bracket) goto cerror;
#endif	AUTOW
				*ep++ = *--bracketp;
				continue;
			}
			*ep++ = CCHR;
			if (c=='\n') goto cerror;
			*ep++ = c;
			continue;

	    case '.':	*ep++ = CDOT;
			continue;

	    case '\n':	goto cerror;

	    case '*':	if (*lastep==CBRA || *lastep==CKET)
#ifndef	AUTOW
				error(0);
#else
				goto cerror;
#endif	AUTOW
			*lastep =| STAR;
			continue;

	    case '$':	if ((peekc=getchar()) != eof) goto defchar;
			*ep++ = CDOL;
			continue;

	    case '[':	*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchar()) == '^')
			{
				c = getchar();
				ep[-2] = NCCL;
			}
			do
			{
				if (c=='\n') goto cerror;
#ifndef	PRINT_STRING
				*ep++ = c;
				cclcnt++;
#else
				if (c=='-' && (fstch=ep[-1]))
					if (fstch == BACKSL) ep[-1] = '-';
					else
					{
						if ((c=getchar())=='\n' || c <= fstch) goto cerror;
						if (ep >= &expbuf[ESIZE-2]) goto cerror;
						*ep++ = '\0'; *ep++ = c; cclcnt++; cclcnt++;
					}
				else { *ep++ = c; cclcnt++; }
#endif	PRINT_STRING
				if (ep >= &expbuf[ESIZE]) goto cerror;
			} while ((c = getchar()) != ']');
			lastep[1] = cclcnt;
			continue;

	    case '&':	if (savcirc)
				if (ep == expbuf) circfl++;
				else { lastep = ep; *ep++ = CCHR; *ep++ = '^'; }
			ep1 = tempbuf;
			while ((*ep = *ep1++) != CEOF)
			{
				if (ep >= &expbuf[ESIZE]) goto cerror;
				switch (*ep)
				{

			    case CBRA:	ep1++;
					if (nbra >= NBRA) goto cerror;
					*bracketp++ = nbra;
					*++ep = nbra++;
					break;

			    case CKET:	ep1++;
					if (bracketp <= bracket) goto cerror;
					*++ep = *--bracketp;
					break;

			    case CCL:
			    case NCCL:
			    case CCL|STAR:
			    case NCCL|STAR:
					*++ep = *ep1++;
					break;

			    case CDOT:
			    case CDOT|STAR:
			    case CCHR:
			    case CCHR|STAR:
					lastep = ep++;
					continue;

			    default:	ep++;
					continue;
				}

				ep++;
				lastep = ep-2;
			}
			if (ep[-1] == CDOL)
				if ((peekc = getchar()) != eof)
				{
					ep[-1] = CCHR;
					*ep++ = '$';
				}
			continue;

	    defchar:
	    default:	*ep++ = CCHR;
			*ep++ = c;
		}
	}
cerror:	expbuf[0] = 0;
	error(0);
}

#ifdef	PRINT_STRING
printstring()
{
	register char *ep, flag;
	register int count;

	putchar('/'); if (circfl) putchar('^');
	for (ep = expbuf; *ep != CEOF; )
		switch(*ep++)
		{

	    case CCHR:	putchar(*ep++); continue;

	    case CCHR|STAR:
			putchar(*ep++); putchar('*'); continue;

	    case CBRA:	ep++; putchar(BACKSL); putchar('('); continue;

	    case CKET:	ep++; putchar(BACKSL); putchar(')'); continue;

	    case CDOT:	putchar('.'); continue;

	    case CDOT|STAR:
			putchar('.'); putchar('*'); continue;

	    case CDOL:	putchar('$'); continue;

	    case NCCL|STAR:
	    case CCL|STAR:
	    case NCCL:
	    case CCL:	flag = *(ep-1); count = *ep++; putchar('[');
			if ((flag & ~STAR) == NCCL) putchar('^');
			while (--count)
				if (*ep=='\0') { ep++; putchar('-'); }
				else putchar(*ep++);
			putchar(']');
			if (flag & STAR) putchar('*');
		}
	puts("/");
}
#endif	PRINT_STRING

execute(gf, addr)
int *addr;
{
	register char *p1, *p2, c;

	if (gf)
	{
		if (circfl) return(0);
#ifndef	BETTER_SWAP
		p1 = linebuf;
		p2 = genbuf;
		while (*p1++ = *p2++);
#endif	BETTER_SWAP
		locs = p1 = loc2;
	}
	else
	{
		if (addr==zero) return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) { loc1 = p1; return(advance(p1, p2)); }
	if (*p2==CCHR)				/* fast check for first character */
	{
		c = p2[1];
		do
		{
			if (*p1!=c) continue;
			if (advance(p1, p2)) { loc1 = p1; return(1); }
		} while (*p1++);
		return(0);
	}
	do					/* regular algorithm */
		if (advance(p1, p2)) { loc1 = p1; return(1); }
	while (*p1++);
	return(0);
}

advance(alp, aep)
{
	register char *lp, *ep, *curlp;
	char *nextep;

	lp = alp;
	ep = aep;
	for (;;)
		switch (*ep++)
		{

	    case CCHR:	if (*ep++ == *lp++) continue; return(0);
	
	    case CDOT:	if (*lp++) continue; return(0);
	
	    case CDOL:	if (*lp==0) continue; return(0);
	
	    case CEOF:	loc2 = lp; return(1);
	
	    case CCL:	if (cclass(ep, *lp++, 1)) { ep =+ *ep; continue; } return(0);
	
	    case NCCL:	if (cclass(ep, *lp++, 0)) { ep =+ *ep; continue; } return(0);
	
	    case CBRA:	braslist[*ep++] = lp; continue;
	
	    case CKET:	braelist[*ep++] = lp; continue;
	
	    case CDOT|STAR:
			curlp = lp;
			while (*lp++);
			goto star;
	
	    case CCHR|STAR:
			curlp = lp;
			while (*lp++ == *ep);
			ep++;
			goto star;
	
	    case CCL|STAR:
	    case NCCL|STAR:
			curlp = lp;
			while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)));
			ep =+ *ep;
	
		star:	do
			{
				lp--;
				if (lp==locs) break;
				if (advance(lp, ep)) return(1);
			} while (lp > curlp);
			return(0);
	
	    default:	error(0);
		}
}

cclass(aset, ac, af)
{
	register char *set, c;
	register n;

	set = aset;
	if ((c = ac) == 0) return(0);
	n = *set++;
#ifndef	PRINT_STRING
	while (--n) if (*set++ == c) return(af);
#else
	while (--n)
		if (*set == '\0')
		{
			if (c > set[-1] && c <= set[1]) return(af);
			set++; set++; n--;
		}
		else if (*set++ == c) return(af);
#endif	PRINT_STRING
	return(!af);
}

putd()
{
	register r;
	extern ldivr;

	count[1] = ldiv(count[0], count[1], 10);
	count[0] = 0;
	r = ldivr;
	if (count[1]) putd();
	putchar(r + '0');
}

puts(as)
{
	register char *sp;

	sp = as;
	col = 0;
	while (*sp) putchar(*sp++);
	putchar('\n');
}

char	line[70];
char	*linp	line;

putchar(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if (listf)
	{
		col++;
		if (col >= LENGTH) { col = 1; *lp++ = BACKSL; *lp++ = '\n'; }
		if (c=='\t') { c = '>'; goto esc; }
		if (c=='\b')
		{
			c = '<';
		esc:	*lp++ = '-';
			*lp++ = '\b';
			*lp++ = c;
			goto out;
		}
		if (c<' ' && c!= '\n')
		{
			*lp++ = BACKSL;
			*lp++ = (c>>3)+'0';
			*lp++ = (c&07)+'0';
			col =+ 2;
			goto out;
		}
	}
	*lp++ = c;
out:	if(c == '\n' || lp >= &line[64])
	{
		linp = line;
		write(1, line, lp-line);
		return;
	}
	linp = lp;
}

/*
 * Get process ID routine if system call is unavailable.
getpid()
{
	register f;
	int b[1];

	if((f = open("/dev/kmem", 0)) < 0) return(-1);
	seek(f, 0140074, 0);
	read(f, b, 2);
	seek(f, b[0]+8, 0);
	read(f, b, 2);
	close(f);
	return(b[0]);
}
 */

op(inglob)
{
	register int	*a1;
	register char	*lp, *sp;
	char	seof, ch;
	int	t, nl;
	int	getopen();
	int	getnil();

	threshold = genbuf + margin;
	savethresh = 0;

	ch = peekc = getchar();
	switch (ch)
	{

    case BACKSL:t = 1;
		delete();
		addr2 = addr1;
		break;

    case ';':
    case '+':	t = 0;
		break;

    case '-':	t =1;
		break;

    default:	goto normal;
	}

	peekc = 0;
	if (addr1 != addr2) error(4);
	oflag = 0;
	append(getnil, addr2-t);
	addr1 = addr2 =- (t-1);
	setdot();
	nonzero();

normal:	if (addr1 == zero) error(4);
	seof = getchar();
	if (seof == '\n') { loc2 = linebuf-1; seof = '\0'; }
	else compile(seof);
	setraw();	/* terminal into raw mode*/

	for (a1 = addr1; a1 <= addr2; a1++)
	{
		if (seof != '\0')
		{
			if (execute(0, a1) == 0) continue;
		}
		else getline(*a1);
		puts("\\\r");
		sp = genbuf;
		inglob =| 01;
		for (lp = linebuf; lp < loc2;)
		{
			putch(*lp);
			*sp++ = *lp++;
		}
		lnp = lp;
		gnp = sp;

#ifdef	NICE_EXIT
		oldline = *a1; thestart = a1; nline = 0;
#endif	NICE_EXIT
		oflag = gopen();	/* open the current line */
		retag(*a1 = putline(linebuf), *a1);	/* write revised line */
		nl = append(getopen, a1);
		a1 =+ nl;
		addr2 =+ nl;
#ifdef	AUTOW
		if (writewaiting)
		{
			delaywrite = writewaiting = 0;
			catchclock();
			delaywrite++;
		}
#endif	AUTOW
	}
	setcook();	/* terminal into cooked mode */
	putchar('\n');
	if (inglob == 0) error(1);
}

getnil()
{
	if (oflag == EOF) return(EOF);
	linebuf[0] = '\0';
	oflag = EOF;
	return(0);
}

setraw()
{
	if (gtty(0, tty) == -1) error(13);
	savetty = tty[2];
	tty[2] =| RAW;
	stty(0, tty);
}

setcook()
{
	tty[2] = savetty;
	stty(0, tty);
}

inword(c)
char c;
{
	if (c>='0' && c<='9') return(1);
	c =& 0137;				/* convert to upper case */
	if (c>='A' && c<='Z') return(1);
	return(0);
}

rescan()
{
	register char *lp, *sp;

	if (savethresh) { threshold = savethresh; savethresh = 0; }
	lp = linebuf;
	sp = genbuf;
	while ((*lp++ = *sp++) != '\0')
		if (lp > linebuf+LBSIZE) { *(--lp) = '\0'; break; }
}

				/*
				 * Leaves revised line in linebuf,
				 * returns 0 if more to follow,
				 * EOF if last line.
				 */
gopen()
{
	register char *lp, *sp, *rp;
	char ch, *br, *pr;
	int tabs;
	int retcode, savint, pid, rpid;

	lp = lnp;
	sp = gnp;
	tabs = 0;
	for (rp = genbuf; rp < sp; rp++) if (*rp == CTRLI) tabs =+ TABSET;

	for (;;)
	{
		switch (ch = getchar())
		{

	    case CTRLD:
	    case ESCAPE:	/* close the line (see case '\n' also) */
	    close:
			putb(lp);
			while (*sp++ = *lp++);
			rescan();
			return(EOF);

	    case CTRLA:				/* verify line */
	    verify:	puts("\\\r");
			*sp = '\0';
			putb(genbuf);
			continue;

	    case CTRLB:				/* back a word */
			if (sp == genbuf) goto backquery;
			while ((*--lp = *--sp) == SPACE)
				if (sp < genbuf) goto out;
			if (inword(*sp))
			{
				while (inword(*--lp = *--sp))
					if (sp < genbuf) goto out;
				if (*sp == SPACE)
					while ((*--lp = *--sp) == SPACE)
						if (sp < genbuf) goto out;
			}
			else while (sp >= genbuf && !inword(*sp))
					if ((*lp-- = *sp--) == CTRLI) tabs =- TABSET;
	    out:	sp++;
			lp++;
			goto verify;

	    case CTRLC:
	    case CTRLQ:				/* forward one char */
			if (*lp == 0) goto backquery;
			putch(*lp);
	    forward:	if (*lp==SPACE && sp+tabs > threshold)
			{
				putch('\r');
				ch = '\n';
				putch(ch);
				lp++;
				*sp++ = ch;
				br = sp;
				break;
			}
			if (*lp == CTRLI) tabs =+ TABSET;
			*sp++ = *lp++;				/* one character */
			if (sp+tabs == threshold) putch(BELL);
			continue;

	    case CTRLE:	putb(lp);
			goto verify;

	    case CTRLF:	while (*lp++);			/* delete forward */
			lp--;
			goto verify;

	    case CTRLH:	help();				/* help */
			goto verify;

	    case CTRLP:	while (*lp != '\0')		/* skip to eol */
			{
				if (*lp == CTRLI) tabs =+ TABSET;
				putch(*sp++ = *lp++);
			}
			continue;

	    case CTRLR:					/* margin release */
			if (threshold-genbuf < LBSIZE-40)
			{
				savethresh = threshold;
				threshold = genbuf+LBSIZE-40;
			}
			else goto backquery;
			continue;

	    case CTRLS:	while (*sp++ = *lp++);		/* re-set to start of line */
			rescan();
			lp = linebuf;
			sp = genbuf;
			tabs = 0;
			goto verify;

	    case CTRLT:					/* backup one character */
			if (sp == genbuf) goto backquery;
			if ((*--lp = *--sp) == CTRLI) tabs =- TABSET;
			goto verify;

	    case CTRLV:	rp = sp;			/* verify spelling */
			pr = unixbuffer + UNIXBUFL - 2;
			*pr = 0;
			while (*(--rp) == SPACE);
			while (inword(*rp) && rp >= genbuf) *--pr = *rp--;
			if (*pr == 0) goto backquery;
			puts("!!");
			setcook();
			if ((pid = fork()) == 0)
			{
				signal(SIGHUP, onhup);
				signal(SIGQUIT, onquit);
				execl("/bin/spell", "spell", pr, 0);
				puts("Sorry, can't spell today");
				exit(1);
			}
			savint = signal(SIGINTR, 1);
			do rpid = wait(&retcode); while (rpid!=pid && rpid!=-1);
			signal(SIGINTR, savint);
			setraw();
			puts("!!");
			goto verify;

	    case CTRLW:					/* forward one word */
			if (*lp == '\0') goto backquery;
			while (*lp == SPACE) putch(*sp++ = *lp++);
			if (inword(*lp))
			{
				while (inword(*lp))
				{
					putch(*sp++ = *lp++);
					if (sp+tabs == threshold) putch(BELL);
				}
				if (*lp == SPACE)
				{
					if (sp+tabs > threshold)
					{
						ch = '\n';
						lp++;
						*sp++ = ch;
						br = sp;
						putch('\r');
						putch('\n');
					}
					if (*lp == SPACE)
						while (lp[1] == SPACE)
							putch(*sp++ = *lp++);
				}
			}
			else while (*lp && !inword(*lp))
				{
					if (*lp == CTRLI) tabs =+ TABSET;
					putch(*sp++ = *lp++);
					if (sp+tabs == threshold) putch(BELL);
				}
			break;

	    case CTRLZ:					/* delete a word */
			if (sp == genbuf) goto backquery;
			while (*--sp == SPACE) if (sp < genbuf) goto zout;
			if (inword(*sp))
			{
				while (inword(*--sp)) if (sp < genbuf) goto zout;
				if (*sp == SPACE)
					while (*--sp == SPACE) if (sp < genbuf) goto zout;
			}
			else while (sp>=genbuf && !inword(*sp))
					if (*sp-- == CTRLI) tabs =- TABSET;
	    zout:	sp++;
			goto verify;

	    case '@':	sp = genbuf;			/* delete displayed line backward */
			tabs = 0;
			goto verify;

	    case RUBOUT:
#ifdef	NICE_EXIT
			*thestart++ = oldline;
			if (nline)		/* some lines added */
			{
				addr1 = thestart;
				addr2 = thestart + nline - 1;
				if (addr2 == dol) delete(); else { delete(); dot--; }
			}
#endif	NICE_EXIT
			puts("\\\r");
			setcook();
			lastc = '\n';
			error(0);

	    case CTRLX:	putch('#');

	    case '#':	if (sp == genbuf) goto backquery;
			if (*--sp == CTRLI) tabs =- TABSET;
			if (ch == CTRLX) goto verify;
			continue;

	    case '\n':
	    case '\r':				/*
						 * split line; actually handled at
						 * end of switch block
						 */
			ch = '\n';
			*sp++ = ch;
			br = sp;
			break;

	    case '\0':	continue;

	    case BACKSL:			/* special symbols */
			ch = getchar();
			if (ch!=BACKSL && ch!='#' && ch!='@')
				if (savetty & UCASE)
					switch (ch)
					{
	
				    case '(':	ch = '{'; break;
	
				    case ')':	ch = '}'; break;
	
				    case '!':	ch = '|'; break;
	
				    case '^':	ch = '~'; break;
	
				    case '\'':	ch = '`'; break;
	
				    default:	if (ch>='a' && ch<='z') ch =- 040;
						else { peekc = ch; ch = BACKSL; }
						break;
					}
				else { peekc = ch; ch = BACKSL; }
	    default:	*(--lp) = ch;
			goto forward;
		}

		if (ch == '\n')
		{
								/* split line */
			if (*(br-1) != '\n') puts("!!");	/* debugging only */
			lnp = sp;
			while (*sp++ = *lp++);			/* move the rest over */
			brp = linebuf + (br - genbuf);
			lnp = linebuf + (lnp - br);
			rescan();
			*(brp - 1) = '\0';
			return(0);
		}
		else continue;

backquery:	putch(BELL);
	}				/* end of forloop block */
}					/* end of gopen */


				/*
				 * calls gopen, deals with multiple lines etc.
				 */
getopen()
{
	register char *lp, *sp;

	if (oflag == EOF) return(EOF);

				/* otherwise, multiple lines */

	lp = linebuf;
	sp = brp;
	while (*lp++ = *sp++);			/* move it down */
	sp = genbuf;
	lp = linebuf;
	while (lp < lnp) *sp++ = *lp++;
	gnp = sp;
					/* should check whether empty line returned */
	oflag = gopen();
	return(0);
}

putch(ch)
char ch;
{
	write(1, &ch, 1);
}

putb(ptr)
char *ptr;	/*display string */
{
	register char *p;

	p = ptr;
	if (*p == '\0') return;
	while (*(++p));
	write(1, ptr, p-ptr);
}

help()
{
	puts("\n");
	puts("	^A	display Again		^Q, ^C	next character");
	puts("	^B	backup word		^R	release margin");
	puts("	ESCAPE				^S	re-scan from Start");
	puts("	or ^D	close line and exit	^V	verify spelling");
	puts("	^E	display to End		^W	next Word");
	puts("	^F	delete line Forward	^Z	delete word");
	puts("	^H	Help			# or ^X delete character");
	puts("	^P	skip to End		^T	backup one character");
	puts("	RUBOUT	exit unchanged		@	delete line backward");
	puts("\n	Other characters (including RETURN) inserted as typed");
}
