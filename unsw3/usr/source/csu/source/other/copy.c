/*
 * the copy command
 *	copy [-a] [-ad] [-c] [-l] [-lr] [-m] [-n] [-o] [-p] [-r] [-s] [-v] sources destination
 */


int	parent;		/* the parent process id for the mkdir forks */

struct {
	int	dev;	/* the device type */
	int	inumber;/* the inode number */
	int	flags;	/* the flags */
	char	nlinks;	/* number of links to the file */
	char	uid;	/* the uid of the file */
	char	gid;	/* the group id of the file */
	char	size0;	/* high byte of the 24 bit size */
	int	size1;	/* low word of the 24 bit size */
	int	addr;	/* first address */
	int	jnk[7];	/* other info */
	long	actime;	/* access time */
	long	mtime;	/* modification date/time */
	} stat1, stat2;

int	type1, type2;

char	name1[256], name2[272];	/* buffers for source and destination names */
char	*name1l, *name2l;	/* points to the last char '\0' in name1 and name2 */

int	direntry[9];	/* buffer for directory entries */

struct	linkdescr {
	struct linkdescr *l_next;	/* next descriptor */
	int	l_olddev;	/* source device */
	int	l_oldino;	/* source i-number */
	int	l_newdev;	/* object device */
	char	*l_name;	/* first object name */
	} *linkdescr;

int	dirdev;		/* current directory device */
int	dirino;

int	askflag;	/* ask before doing anything */
int	askdirflag;	/* ask before doing anything to directories */
int	linkflag;	/* if on then links are used where possible */
int	linkremote;	/* if on then links are preserved as much as possible on remote volumes */
int	newonlyflag;	/* if on then override will not be done */
int	ownerflag;	/* if on and super user then will change owner on created files etc */
int	modeflag;	/* if on then mode will be set for new file (sticky bits etc.) */
int	recursiveflag;	/* if on then will branch down directory structure */
int	compressflag;	/* compress the files (use virtual blocks) */
int	moveflag;	/* move mode (delete after copy) */
int	smdatflag;	/* set modification date */
int	verboseflag;	/* prints lots of information */
int	suflag;		/* 1 if super user otherwise 0 */

main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	register char *s, *t;
	char *name2ptr;	/* "to" name */
	int fcount;

	parent = getpid();
	fcount = 0;
	suflag = (getuid() & 0377) == 0;	/* set suflag id */
	for (i = 2; i < 16; i++)
		close(i);	/* close unneeded files */

	for (i = 1; i < argc; i++)
		if (argv[i][0] == '-')
			switch (argv[i][1]) {

			case 'a':	/* prompt */
				askdirflag = 1;
				if (argv[i][2] != 'd')
					askflag = 1;
				break;

			case 'c':	/* compress */
				compressflag = 1;
				break;

			case 'l':	/* links */
				linkflag = 1;
				if (argv[i][2] == 'r')
					linkremote = 1;
				break;

			case 'm':	/* move mode */
				moveflag = 1;
				break;

			case 'n':	/* new files */
				newonlyflag = 1;
				break;

			case 'o':	/* set owner */
				if (suflag)
					ownerflag = 1;
				else
					printf("only super user may set -o flag\n");
				break;

			case 'p':	/* preserve mode */
				modeflag = 1;
				break;

			case 'r':	/* recursive */
				recursiveflag = 1;
				break;

			case 's':	/* smdate */
				if (suflag)
					smdatflag = 1;
				else
					printf("only super user may set -s flag\n");
				break;

			case 'v':	/* verbose */
				verboseflag = 1;
				break;

			default:	/* urk!! */
				printf("Unknown switch: '%c'\n", argv[i][1]);
				break;
			}
		else
			argv[fcount++] = argv[i];	/* copy the arg */

	if (--fcount <= 0) {	/* fcount is now the number of sources */
		if (fcount < 0) {
			printf("Error: arg count\n");
			return(0);
		}
		fcount++;

		/* if only one source, use current directory for object */
		name2ptr = ".";
	} else
		name2ptr = argv[fcount];

	for (i = 0; i < fcount; i++) {	/* this code solves the multiple sources problem */

		/* get a source */
		s = name1;
		t = argv[i];
		while ((*s++ = *t++) != '\0');
		name1l = --s;	/* point to the '\0' in char string */
		if (*--s == '/') {
			/* extra '/' */
			*s = '\0';
			name1l = s;
		}

		/* get the destination */
		s = name2;
		t = name2ptr;
		/* always the same file */
		while ((*s++ = *t++) != '\0');
		name2l = --s;
		if (*--s == '/') {
			/* extra '/' */
			*s = '\0';
			name2l = s;
		}

		getstat();	/* 1+2 */
		if (type1==2 && type2!=0)
			cd();	/* do directory trans */
		else {
			/* do the cp command */
			/* make entry in the direntry for the cp command */
			s = &direntry[1];
			for (t = name1; *t != '\0'; t++)
				if (*t == '/') {
					if (t[1] != '\0')
						s = &direntry[1];
				} else if (s < &direntry[8])
					*s++ = *t;
			while (s < &direntry[8])
				*s++ = '\0';
			cp();	/* do the cp command */
		}
	}
	return(0);
}

/*
 * types:
 *	0 file
 *	1 character device
 *	2 directory
 *	3 block device
 *	-1 non-existent
 */
/*
 * gets the status on the two files
 */
getstat()
{
	if (stat(name1, &stat1) < 0)
		type1 = -1;
	else
		type1 = ((stat1.flags) >> 13) & 3;
	if (stat(name2, &stat2) < 0)
		type2 = -1;
	else
		type2 = ((stat2.flags) >> 13) & 3;
}

/*
 * Test for a file already there
 */
ifexist()
{
	if (type2 < 0)
		return(0);
	if (type1 == type2) {
		/* same file type */
		if (stat1.dev==stat2.dev && stat1.inumber==stat2.inumber)
			return(1);	/* same file anyway */
		if (type1==1 || type1==3)
			/* special file */
			if (stat1.addr == stat2.addr)
				return(1);	/* same major/minor */
	}
	if (newonlyflag) {
		/* don't clobber */
		printf("Error: cannot overwrite %s\n", name2);
		return(1);
	}
	switch (type2) {

	case 0:	/* file */
	case 1:	/* char special */
	case 3:	/* block special */
		if (type1==0 && !linkflag)
			break;	/* preserve links */
		unlink(name2);
		break;

	case 2:	/* directory */
		callsys("/bin/rmdir", "rmdir", name2, 0);
		break;
	}
	return(0);
}

/*
 * set mode, owner, modification date
 */
setall(mode, owner, modtime)
int	mode;
int	owner;
long	modtime;
{
	if (mode != 0)
		/* inode allocated bit on! */
		chmod(name2, mode);
	if (ownerflag)
		chown(name2, owner);
	if (smdatflag)
		mdate(name2, modtime);
}

/*
 * get next useful directory entry
 */
getentry(df)
int df;	/* the directory file descriptor */
{
	do {
		if (read(df, direntry, 16) != 16)
			return(0);
	} while (direntry[0] == 0);
	return(1);
}

/*
 * append last directory entry to names
 */
push_name()
{
	register char *s, *t;

	/* add entry onto name1 */
	if (name1l+17 > &name1[256]) {
		printf("Error: file name too long %s\n", name1);
		exit(1);
	}
	s = name1l;
	t = &direntry[1];
	*s++ = '/';
	while ((*s++ = *t++) != '\0');
	name1l = --s;

	/* add entry onto name2 */
	if (name2l+17 > &name2[256]) {
		printf("Error: file name too long %s\n", name2);
		exit(1);
	}
	s = name2l;
	t = &direntry[1];
	*s++ = '/';
	while ((*s++ = *t++) != '\0');
	name2l = --s;
}

/*
 * copy a directory
 */
cd()
{
	register int dirfd;	/* source-directory file descriptor */
	int	newdir;	/* new directory created */
	int	oldmode1;	/* preserve old permission */
	int	oldown1;	/* preserve old owner */
	long	oldmodtime1;	/* preserve old modification time */
	int	lasttype;	/* flag for knowing about sick accesses */
	int	name1s, name2s;	/* save current state of names */
	int	sdirdev;
	int	sdirino;

	oldmode1 = stat1.flags;	/* save for later use */
	oldown1 = (stat1.gid<<8)|stat1.uid;
	oldmodtime1 = stat1.mtime;
	newdir = 0;
	if (type2 < 0) {	/* does not exist so create the directory */
		newdir++;
		callsys("/bin/mkdir", "mkdir", name2, 0);
		getstat();	/* 2 */
	}
	if (type2 != 2) {	/* if two then directory otherwise error */
		if (type2 < 0)
			printf("Error: could not mkdir %s\n", name2);
		else
			printf("Error: %s is not a directory\n", name2);
		return;
	}
	if ((dirfd = open(name1, 0)) < 0) {
		printf("Error: could not open directory %s\n", name1);
		return;
	}

	name1s = name1l;	/* save name status */
	name2s = name2l;
	/*
	 * save directory status - for remote-links
	 */
	sdirdev = stat2.dev;
	sdirino = stat2.inumber;
	getentry(dirfd);	/* skip first two entries */
	getentry(dirfd);	/* because they are the "." & ".." entries */

	while (getentry(dirfd)) {	/* get an entry */
		push_name();	/* and push it onto the name1 & name2 */
		getstat();	/* 1+2 */
		dirdev = sdirdev;
		dirino = sdirino;
		switch (lasttype = type1) {

		case 0:	/* just a file */
			if (ask(askflag, "copy file", name1))
				cp();
			break;

		case 1:	/* character-type special file */
		case 3:	/* block-type special file */
			if (ask(askdirflag, "copy special file", name1))
				sf();
			break;

		case 2:	/* directory */
			if (recursiveflag)	/* ignore if not */
				if (ask(askdirflag, "examine directory", name1))
					cd();	/* copy the directory */
			break;

		default:	/* error */
			printf("Error: %s does not exist", name1);
			break;
		}
		name1l = name1s;
		*name1l = '\0';	/* pop name1 */
		name2l = name2s;
		*name2l = '\0';	/* pop name2 */
		if (lasttype < 0)
			break;	/* error accessing file???? */
	}
	close(dirfd);	/* close the file source-directory */
	if (newdir)
		/* if directory was created then change */
		setall(oldmode1, oldown1, oldmodtime1);
	if (moveflag)
		callsys("/bin/rmdir", name1, 0);
}

/*
 * copy a special file
 */
sf()
{
	if (!suflag) {
		printf("Error: Only super user can copy special files. %s\n", name1);
		return;
	}
	if (ifexist())
		return;
	mknod(name2, stat1.flags, stat1.addr);	/* make the node */
	setall(0, (stat1.gid<<8)|stat1.uid, stat1.mtime);
	getstat();	/* 2 */
	if (type1!=type2 || stat1.flags!=stat2.flags || stat1.addr!=stat2.addr) {
		printf("Error: could not create special file %s\n", name2);
		unlink(name2);
		return;
	}
	if (moveflag)
		unlink(name1);
}

/*
 * works just like the cp command
 */
cp()
{
	static char buf[512];
	int fold, fnew;
	register int n;
	register char *s, *t;

	/* is target a directory? */
	if (type2 == 2) {	/* if so then pad name2 with direntry */
		s = name2l;	/* set to last char in destination name */
		*s++ = '/';	/* push a '/' */
		t = &direntry[1];	/* points to begin of source string */
		while ((*s++ = *t++) != '\0');	/* add last part of name2 */
		getstat();	/* 2 */
	}
	if (linkflag) {
		if (link(name1, name2) >= 0)
			goto ok;
		if (linkremote)
			if (remotelink())
				goto ok;
	}
	if ((fold = open(name1, 0)) < 0) {
		printf("Error: cannot open %s\n", name1);
		return;
	}
	if ((fnew = creat(name2, stat1.flags)) < 0) {
		printf("Error: cannot create %s\n", name2);
		close(fold);
		return;
	}
	t = 0;	/* error flag */
	while ((n = read(fold, buf, 512)) > 0) {
		if (compressflag) {
			for (s = &buf[n]; *--s == '\0';)
				if (s == buf) {
					seek(fnew, n, 1);
					goto again;
				}
		}
		if (write(fnew, buf, n) != n) {
			printf("Error: write error on %s\n", name2);
			t++;
			break;
		}
	again:;
	}
	if (n < 0) {
		printf("Error: read error on %s\n", name1);
		t++;
	}
	close(fold);
	close(fnew);
	if (t) {
		unlink(name2);
		return;
	}
ok:
	if (type2 < 0)
		setall(modeflag ? stat1.flags : 0, (stat1.gid<<8)|stat1.uid, stat1.mtime);
	if (moveflag)
		unlink(name1);
}

/*
 * interrogate user about operation
 */
ask(f, s, t)
int f;
{
	char ch;
	register flg;

	if (f) {	/* if flag true then ask question */
		printf("%s %s? ", s, t);
		if (read(0, &ch, 1) != 1)
			return(0);
		flg = ch=='y';
		while (ch != '\n')
			if (read(0, &ch, 1) != 1)
				break;
		return(flg);
	} else if (verboseflag)
		printf("%s %s\n", s, t);
	return(1);
}

/*
 * handle remote links
 */
remotelink()
{
	register struct linkdescr	*p;
	char	*name;

	if (stat1.nlinks < 2)
		return(0);
	name = 0;
	{
		register struct linkdescr	**pp;

		for (pp = &linkdescr; (p = *pp) != 0; pp = &p->l_next) {
			if (p->l_olddev != stat1.dev ||
			    p->l_oldino != stat1.inumber)
				continue;
			name = p->l_name;
			if (p->l_newdev != dirdev)
				continue;
			if (link(p->l_name, name2) < 0) {
				printf("Error: link failed? %s, %s\n", p->l_name, name2);
				return(0);
			}
			return(1);
		}
		if ((p = getmain(sizeof *p)) == 0)
			goto nomem;
		*pp = p;
	}
	p->l_olddev = stat1.dev;
	p->l_oldino = stat1.inumber;
	p->l_newdev = dirdev;
	if (name == 0) {
		register char	*s1, *s2;

		for (s1 = name2; *s1++ != '\0';);
		if ((s2 = getmain(s1 - name2)) == 0) {
	nomem:
			printf("Error: Insufficient memory for remote-links\n");
			exit(1);
		}
		name = s2;
		for (s1 = name2; (*s2++ = *s1++) != '\0';);
	}
	p->l_name = name;
	return(0);
}

/*
 * allocate memory
 */
getmain(size)
unsigned size;
{
	extern char	end[];
	static char	*mnow	&end;
	static char	*mtop	&end;
	register char	*base;

	base = mnow;
	mnow =+ (size + 1) & ~1;
	while (mnow > mtop) {
		if (mtop == 0160000)
			return(0);
		mtop = (mtop + 04000) & ~03777;
		if (brk(mtop) == -1)
			return(0);
	}
	return(base);
}

/*
 * call system programs to do your dirty work
 */
callsys(nm, args)
char	*nm;
char	*args;
{
	register char	**arg;
	int	pid;
	static int	status;

	arg = &args;
	do {
		if ((pid = fork()) == 0) {
			execl(nm, arg[0], arg[1], arg[2], arg[3]);
			printf("Error: could not exec %s\n", arg[0]);
			kill(9, parent);
			exit(1);
		}
	} while (pid == -1);
	while (pid != wait(&status));
	if (status != 0)
		printf("Error: %s failed\n", arg[0]);
	return(status != 0);
}
