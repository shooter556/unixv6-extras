#

/*
 * New super-duper incredibly improved librarian/archiver
 */

/*
 *	For information:
 *
 *		Craig McGregor
 *		c/o Computing Services Unit.
 *		University of New South Wales
 */

/*
 * Usage:
 *	slup library_name switches files ...
 *
 * Valid switches:
 *  note that any switch (except gd, gn, n, u) may
 *  be followed by 'v' for verbose mode.
 *
 * Some switches may be followed by a set of groupnames
 * to specify which groups are to be enabled.
 *	[/groupname]...
 *
 *	-d
 *		delete named files
 *	-du/modset(s)
 *		delete named modsets
 *	-g/groupname(s)
 *		reference the modules in the named group(s)
 *	-gm/groupname(s)
 *		create the named group(s)
 *	-gr/groupname(s)
 *		remove the named group(s)
 *	-gi/groupname(s)
 *		insert the listed subfiles in the named group(s)
 *	-gd/groupname(s)
 *		remove the named subfiles from the named group(s)
 *	-i
 *		insert named files
 *	-l[a][g][m][x][d]
 *		list the named files.  if no names then
 *		everything is listed.
 *	-n
 *		create new library
 *	-r
 *		replace named files
 *	-u[p]
 *		update.  update directives come from standard input.
 *	-x[s]
 *		extract named files. if no names then
 *		extract all.
 *	-p[a][d][c][l][s][0]
 *		print files
 *	-xo
 *		extract the original source
 *	-xu/modset(s)
 *		extract the update directives only
 *	-t
 *		Similar to '-u' except that library is not updated
 *	-c[+/-]
 *		Set or reset common status on file.
 *	-e(x)
 *		set escape character
 *	-o filename
 *		output file for listing
 */
/*
 *	WANT::::::
 *
 *	ERR's
 *	yank
 *	libtest
 *	-p current
 *	update line numbers/error messages
 *	insert: drop pre-stuff
 *	newdeck for old file
 *	{
 *		*oplfile f1,f2..	merge with library
 *		*copy file,deck		add a deck
 *		*read file		include
 *		*define			conditional
 *		*ifcall			"
 *	}
 *	-x[..] ~~ -p[..]
 *	useek : ungetc()
 *	extract: options
 *	refs to unewflag?
 *	setmode(3)
 *	yank
 *	cmdline/aline/alinep
 *	yank
 *	setmode(2/3)
 *	cc mode 'xu'
 *	UTEXT/cmd defns get moved up file
 *	p[a][c] vs x[a][c]
 *	ufsav?
 *	-du: doesn't do #1
 *	   : no deletions??
 *	getlist/getitem
 *	broken line (-i/-u)
 *	putlib/getlib
 *	listmod
 *	-u -> -x
 *	-du/-d/-r : drop mod entries.
 *	-du gives core image:
 *		-du/initial restor.c
 *	slup mylib -n -iv a b c?
 *		a,b,b???
 *	*edit?
 *	-xo to handle *newdeck??
 *	-u not picking up inconsistencies:
 *		*delete.. (modset a)
 *		*before ..(modset b)
 *
 * avail:
 *	a,b,f,h,j,k,m,q,s,w,y,z
 */

#include "slup.h"

main(argc, argv)
int	argc;
char	*argv[];
{
	register char	*s;
	int	i;
	int	errrtn;
	int	xargc;
	char	**xargv;
	int	trapped();

	errflag = 0;

	libmods = 0;

	errrtn = NOERR;

	FOUTSET(&fout, 1);
	promptdone = 1;

	if (argc <= 1) {
		printf("Usage: slup libname [switches] [files]\n");
		printf("Switches: d,du,e,g,gm,gr,gi,gd,i,l,n,o,p,r,t,v,u,x,xo,xu\n");
		ERROR(/* none */);
	}

	/* get library name */
	libname = argv[1];
	tmpfiles();		/* temporary file-names */

	if ((errrtn = switches(argc - 2, argv + 2, &xargc, &xargv)) < 0)
		ERROR();

	if ((s = getsuf(libname)) == NULL) {
		s = setsuf(libname, "slb");
		if (stat(s, statbuf)>=0 || stat(libname, statbuf)<0) {
			copys(s, libnbuf);
			libname = libnbuf;
		}
	}

	/* connect interrupt to trapped() */
	if (signal(2, 1) != 1)
		signal(2, &trapped);

	/* go to work */
	now = time();

	if (func == 0) {
		if (options & (OPTXTR | OPTPRNT))
			func = EXTRACT;
		else
			if (xargc!=0 && !(options & OPTLIST))
				func = INSERT;
	}

	if ((errrtn = getlib(func==INSERT ? xargc : 0)) < 0)
		ERROR();

	if ((errrtn = dogroups()) < 0)
		ERROR();

	/*
	 * create temporary files for library updates
	 */
	if ((workfid = openu(worktemp)) < 0)
		ERROR(errrtn = ERRTEMP);
	if ((docfile = openu(docname)) < 0)
		ERROR(errrtn = ERRTEMP);
	switch (func) {

	case INSERT:
	case UPDATE:
	case DELETE:
	case DELUPDT:
		if ((scrfid = openu(scrtemp)) < 0)
			ERROR(errrtn = ERRTEMP);
		break;

	default:
		scrfid = 0;
		break;
	}
	if ((errrtn = binit()) < 0)
		ERROR();

	switch (func) {

	case INSERT:
		if ((errrtn = insert(xargc, xargv)) < 0)
			ERROR();
		break;

	case EXTRACT:
	case EXORIG:
	case DELETE:
	case DELUPDT:
	case COMMON:
	case INSGRP:
	case DELGRP:
		if ((errrtn = callall(func, xargc, xargv)) < 0)
			ERROR();
		break;

	case EXMODS:
		if ((errrtn = exupdts(xargc, xargv)) < 0)
			ERROR();
		break;

	case UPDATE:
		if ((ufid = openu(utemp)) < 0)
			ERROR(errrtn = ERRTEMP);

		FINSET(&fin, 0);

		descrip = options&OPTTEST ? "t" : "u";

		errrtn = update();

		unread(&fin);

		if (options & OPTTEST)
			libmods = 0;

		if (errrtn < 0)
			ERROR();
		break;
	}

	if (options & OPTLIST)
		list(func!=DELETE ? xargc : 0, xargv);
	errrtn = putlib();

errlabl:
	errrtn = error(errrtn);

	cleanup();
	exit(errrtn < 0);
}

/*
 * Do individual calls for most processors
 */
callall(fn, argc, argv)
int	fn;
int	argc;
char	*argv[];
{
	int	(*fnadr)();
	char	*fnm;
	char	namebuf[NAMLENG+1];
	register struct filentry	*fp;
	int	i;
	int	errrtn;
	int	errmode;

	extern int	extract();
	extern int	exoriginal();
	extern int	delete();
	extern int	delupdts();
	extern int	common();
	extern int	insgroup();
	extern int	delgroup();

	namebuf[NAMLENG] = '\0';

	switch (func) {

	case EXTRACT:
		fnadr = &extract;
		if (options & OPTXTR)
			descrip = (options & OPTPRNT) ? "xp" : "x";
		else
			descrip = "p";
		break;

	case EXORIG:
		fnadr = &exoriginal;
		descrip = "xo";
		break;

	case DELETE:
		fnadr = &delete;
		descrip = "d";
		break;

	case DELUPDT:
		fnadr = &delupdts;
		descrip = "du";
		break;

	case COMMON:
		fnadr = &common;
		descrip = "cx";
		descrip[1] = (options & OPTCOMM) ? '+' : '-';
		break;

	case INSGRP:
		fnadr = &insgroup;
		descrip = "gi";
		break;

	case DELGRP:
		fnadr = &delgroup;
		descrip = "gd";
		break;
	}

	errmode = 0;
	if (argc == 0)
		for (i = 0, fp = filentry; i < nfiles; i++, fp++) {
			if (func==EXORIG && fp->f_flags&FXNEW)
				continue;
			if (!(options & OPTGRPS) || grpmask&fp->f_groups) {
				copyname(fp->f_name, namebuf);
				fnm = namebuf;
				setprompt(fnm);
				if (verbose)
					doprompt();
				if ((errrtn = (*fnadr)(fp, fnm)) < 0) {
					error(errrtn);
					errmode++;
				}
			}
		}
	else
		for (i = 0; i < argc; i++) {
			fnm = argv[i];
			setprompt(fnm);
			if (verbose)
				doprompt();
			if ((errrtn = getname(endpath(fnm), namebuf)) < 0) {
				error(errrtn);
				errmode++;
				continue;
			}
			if ((fp = findfile(namebuf)) == NULL) {
				error(ERRFFND);
				errmode++;
				continue;
			}
			if ((errrtn = (*fnadr)(fp, fnm)) < 0) {
				error(errrtn);
				errmode++;
			}
		}
	return(errmode ? ERR : NOERR);
}

/*
 * Routine to cleanup after aborting slup
 */
trapped()
{
	cleanup();
	exit(2);
}

/*
 * close down temporary files etc.
 */
cleanup()
{
	fflush(&fout);
	FOUTSET(&fout, 1);
	if (insfile > 0) {
		close(insfile);
		insfile = 0;
	}
	if (scrfid > 0) {
		close(scrfid);
		scrfid = 0;
		unlink(scrtemp);
	}
	if (workfid > 0) {
		close(workfid);
		workfid = 0;
		unlink(worktemp);
	}
	if (ufid > 0) {
		close(ufid);
		ufid = 0;
		unlink(utemp);
	}
	if (docfile > 0) {
		close(docfile);
		docfile = 0;
		unlink(docname);
	}
	if (lstfile > 0) {
		if (fileno != 0)
			write(lstfile, "\n", 1);
		close(lstfile);
		if (!(options & OPTOFIL))
			if (fork() == 0)
				execl("/bin/lpr", "lpr", "-r", lstname, 0);
	}
}
