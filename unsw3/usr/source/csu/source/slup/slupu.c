#

/*
 * Update/Modify section of "slup" the new super-duper
 * librarian/archiver.
 */

#include "slup.h"

/*
 * Update directives.
 *
 *	address format:
 *		[modset.]lineno
 *		if "modset." is left out then the modset used is
 *		the last one defined.  if a null modset is used,
 *		this is taken to mean the original file.
 *
 *
 *	*deck filename
 *	*dk filename
 *		select next file to modify
 *
 *	*newdeck filename
 *	*nd filename
 *		add new file for modification
 *
 *	*ident modsetname
 *	*id modsetname
 *		enter the name of this modify set
 *
 *	*modname defaultname
 *	*m defaultname
 *		this name is used when none is specified
 *
 *	*after adr
 *	*a adr
 *	*insert adr
 *	*i adr
 *		sets the position in the file.  any insertion text
 *		is added at this point.
 *
 *	*before adr
 *	*b adr
 *		sets the position in the file.  any insertion text
 *		is added immediately before this point.
 *
 *	*append
 *	*ap
 *		Sets the position in the file to the very end.
 *
 *	*delete adr1[,adr2]
 *	*d adr1[,adr2]
 *		the addressed text is deleted whenever this modset is used
 *
 *	*restore adr1[,adr2]
 *	*r adr1[,adr2]
 *		the addressed text is restored (if previously deleted)
 *
 *	*exit
 *	*e
 *		exit from this update (used mainly for shell files)
 *
 *	*prefix [char]
 *	*p [char]
 *		change escape character.
 *
 *	*yank modsetname[,*]
 *	*y modsetname[,*]
 *	*unyank modsetname[,*]
 *	*u modsetname[,*]
 *		yank/unyank the named modification set
 *
 *	* /text.....	(that blank between '*' and '/' shouldn't be there)
 *		Documentation comment
 *
 *	\*text.....
 *		used to enter text with the escape character
 *		as the first character on the line.
 *
 *	**text.....
 *		Flushed comment
 *
 *	*call filename
 *	*c filename
 *		include the named module at extraction time
 *
 *	*prefixc [char]
 *	*pc [char]
 *		change extraction time escape character
 */

/*
 * Format of source files:
 *
 *	all blank strings (even single blanks)
 *		0200 + #blanks (upto 127)
 *
 *	all update codes are strings commencing with a null:
 *
 *	000,xxx,yyy	xxx is the index within the update table
 *			(uentry) of the current update.
 *			yyy - command (1 = after, 2 = before,
 *					3 = append,
 *					4 = delete, 5 = restore)
 *
 *	after:
 *	before:	followed by:
 *		iii	index of modification set that text was
 *			inserted into.
 *		cccccc	count of number of lines inserted here.
 *
 *	append: followed by: nothing
 *
 *	delete:
 *	restore: followed by:
 *		iii	index of modification set that terminates
 *			delete/restore operation
 *		llllll	line number within specified modification
 *			set for termination.
 *
 *	comment: followed by:
 *		string	just another text line in appearance.
 */

struct cmds	ucmds[] {
	IDENT,	"id",	"ident",
	DECK,	"dk",	"deck",
	NEWDECK,"nd",	"newdeck",
	AFTER,	"a",	"after",
	BEFORE,	"b",	"before",
	AFTER,	"i",	"insert",
	PREFIX,	"p",	"prefix",
	RESTORE,"r",	"restore",
	UDELETE,"d",	"delete",
	MODNAME,"m",	"modname",
	EXIT,	"e",	"exit",
	YANK,	"y",	"yank",
	UNYANK,	"u",	"unyank",
	UTEXT,	"c",	"call",
	UTEXT,	"pc",	"prefixc",
	UAPPEND,"ap",	"append",
	-1
	};

	char	uescape;		/* current escape character */
	char	cmdline[150];		/* command line buffer */

/*
 * update processor
 */
update()
{
	register char	*s;
	register struct filentry	*ufp;
	register int	i;
	int	errrtn;
	int	errmode;
	char	namebuf[NAMLENG];
	struct modentry	*ump;
	int	echo;
	int	oldmode;

	errmode = 0;
	uescape = dfltescape;
	linenum = 0;

	clrname(thismod);
	clrname(dfltmod);

	updtid = 0;
	ufp = NULL;
	savlines = 0;
	ump = NULL;

	upeekc = '\0';

	oldmode = 0;
	if (subopts & OPTUPRT)
		oldmode = 2;

	for (;;) {
		echo = 0;
		setmode(oldmode);
		for (s = cmdline; (i = getc(&fin)) != '\n';) {
			if (i < 0)
				goto l1;
			if (s == &cmdline[150 - 1]) {
				while ((i = getc(&fin)) != '\n')
					if (i < 0) goto l1;
				nerror(ERRLINE);
				break;
			}
			*s++ = i;
		}
		*s++ = '\0';
		linenum++;

		i = udirective(namebuf);

		if (i < 0)
			/* error */
			errmode = 1;

		if ((i==IDENT || i==DECK || i==NEWDECK) && errmode>0)
			/* partial recovery */
			errmode = -1;

		if (errmode > 0) {
			errrtn = i;
			goto errx;
		}

		switch (i) {

		case COMMK:	/* update comment */
			if (ump == NULL)
				ERROR(errrtn = ERRNMOD);
			if (subopts & OPTUPRT) {
				kickline();
				printf("%-14.14s", thismod);
				printf(".******\t");
				println(cmdline);
				echo++;
			}
			if ((errrtn = modcompress(&cmdline[2], cmdline)) < 0)
				ERROR();
			for (i = 0; cmdline[i] != '\0'; i++);
			cmdline[i++] = '\n';
			if (write(docfile, cmdline, i) != i)
				ERROR(errrtn = ERRIO);
			docleng =+ i;
			continue;

		case UTEXT:	/* text lines */
			if (savlines++ == 0) {
				if ((errrtn = updtok(ufp, ump)) < 0)
					ERROR();
				if ((errrtn = useek('\0', 1)) < 0)
					ERROR();
			}
			if (subopts & OPTUPRT) {
				kickline();
				printf("%-14.14s", thismod);
				printf(".%6u\t", ++uwork[updtid].w_lines);
				println(cmdline);
				echo++;
			}

			s = cmdline;
			if (*s=='\\' && (s[1]==uescape || (s[1]=='\\' && s[2]==uescape)))
				s++;

			if ((errrtn = modcompress(s, cmdline)) < 0)
				ERROR();
			for (s = cmdline; *s != '\0';) {
				putc(*s++, &ufsav);
				ulngsav++;
			}
			putc('\n', &ufsav);
			ulngsav++;
			continue;

		case MODNAME:	/* default modset name */
			if (subopts & OPTUPRT) {
				kickline();
				printf("\t      .******\t");
				println(cmdline);
				echo++;
			}
			continue;

		case PREFIX:	/* set escape character */
		case COMMF:	/* flushed comment */
		case EXIT:	/* exit */
			if (subopts & OPTUPRT) {
				kickline();
				printf("**************.******\t");
				println(cmdline);
				echo++;
			}
			if (i == EXIT)
				goto l1;
			else
				continue;
		}

		/* directive */

		if ((errrtn = uunsave()) < 0)
			ERROR();

		switch (i) {

		case DECK:	/* specify file name */
		case NEWDECK:	/* specify file name */
			if ((errrtn = ucopy(ufp)) < 0)
				ERROR();
			if (subopts & OPTUPRT) {
				skipln(4, 2);
				kickline();
				printf("\t      .******\t");
				println(cmdline);
				echo++;
			}
			if ((ufp = findfile(namebuf)) == NULL) {
				if (i == DECK)
					ERROR(errrtn = ERRFFND);
				if (nfiles == mxnfiles)
					if ((errrtn = growsrc(nfiles + 10)) < 0)
						ERROR();
				ufp = newfile(namebuf);
				ufp->f_flags =| FXNEW;
			} else {
				if (i == NEWDECK)
					ERROR(errrtn = ERRFDUP);
			}
			unewflag = ufp->f_flags & FXNEW;

			continue;

		case IDENT:	/* define modification name */
			if ((errrtn = ucopy(ufp)) < 0)
				ERROR();
			if (!cmpname(namebuf, thismod)) {
				if (ump != NULL)
					if ((errrtn = putmods(ump)) < 0)
						ERROR();
				copyname(namebuf, thismod);
				if ((ump = findmod(thismod)) != NULL) {
					if (!(ump->m_flags & MUUPDAT))
						ERROR(errrtn = ERRDONE);
				} else {
					if (nmods == mxnmods)
						if ((errrtn = growmods(mxnmods + 10)) < 0)
							ERROR();
					ump = &modentry[nmods++];
					copyname(thismod, ump->m_name);
					ump->m_date = now;
					ump->m_refs = 0;
					ump->m_flags = 0;
					ump->m_base = 0;
					ump->m_size = 0;
				}
				if ((errrtn = getmods(ump, 1)) < 0)
					ERROR();
			}
			setprompt(thismod);
			if (verbose)
				doprompt();
			if (subopts & OPTUPRT) {
				settitle(thismod);
				kickline();
				printf("%-14.14s.******\t", thismod);
				println(cmdline);
				echo++;
			}
			continue;

		case YANK:	/* yank */
		case UNYANK:	/* unyank */
			if (ufp == NULL)
				ERROR(errrtn = ERRNFIL);
			if ((errrtn = ucopy(ufp)) < 0)
				ERROR();
			if (ump != NULL)
				if ((errrtn = putmods(ump)) < 0)
					ERROR();
			if (subopts & OPTUPRT) {
				setmode(3);
				settitle(NULL);
				kickline();
				printf("\t      .******\t");
				println(cmdline);
				echo++;
			}
			if ((errrtn = yank(ufp, i == YANK)) < 0)
				ERROR();
			continue;
		}

		if (subopts & OPTUPRT) {
			needln(2);
			kickline();
			printf("%-14.14s.******\t", thismod);
			println(cmdline);
			echo++;
		}

		/* positioning directives */

		if ((errrtn = updtok(ufp, ump)) < 0)
			ERROR();

		switch (i) {

		case AFTER:	/* after */
		case BEFORE:	/* before */
			if ((errrtn = ldeschk(&tag1, i != AFTER)) < 0)
				ERROR();
			break;

		case UAPPEND:	/* at end */
			posnmode = -1;
			break;

		case UDELETE:	/* delete */
		case RESTORE:	/* restore */
			if ((errrtn = ldeschk(&tag1, 1)) < 0)
				ERROR();
			if ((errrtn = useek('\0', 0)) < 0)
				ERROR();
			if ((errrtn = ldeschk(&tag2, 0)) < 0)
				ERROR();
			putc('\0', &ftmp2);
			putc(updtid, &ftmp2);
			putc(i==UDELETE ? UDEL : URESTOR, &ftmp2);
			putc(srchid, &ftmp2);
			putw(srchlin, &ftmp2);
			if ((errrtn = useek(i==UDELETE ? '-' : '+', 0)) < 0)
				ERROR();
			ulngo =+ 6;
			break;
		}
		continue;
	errlabl:
		errmode = 1;
	errx:
		uerror(!echo, errrtn);
	}
l1:
	errrtn = uunsave();
	if ((errrtn = ucopy(ufp)) < 0)
		return(errrtn);
	if (ump != NULL)
		if ((errrtn = putmods(ump)) < 0)
			return(errrtn);
	if (subopts & OPTUPRT)
		setmode(0);
	if (options & OPTXTR)
		if ((errrtn = doextractions()) < 0)
			return(errrtn);
	return(errmode != 0 ? ERR : NOERR);
}

/*
 * Handle extractions of updated files
 */
doextractions()
{
	register int	i;
	register struct filentry	*fp;
	int	errrtn, errflag;
	char	namebuf[NAMLENG+1];

	namebuf[NAMLENG] = '\0';
	errflag = 0;
	for (i = 0, fp = filentry; i < nfiles; i++, fp++)
		if (fp->f_flags & UXRUN) {
			copyname(fp->f_name, namebuf);
			if ((errrtn = error(extract(fp, namebuf))) != NOERR)
				errflag++;
		}
	return(errflag ? ERR : NOERR);
}

/*
 * handle modification directives
 */
udirective(namebuf)
char	namebuf[NAMLENG];
{
	register int	i;
	register char	*s;
	char	*tag1p, *tag2p;
	int	errrtn;

	if (cmdline[0] != uescape)
		return(UTEXT);

	switch (cmdline[1]) {

	case '*':
		return(COMMF);

	case '/':
		return(COMMK);
	}

	copys(cmdline, workb);
	workp = &workb[1];

	tag1p = getstring(&workp);

	if ((i = match(ucmds, tag1p)) <= 0)
		return(i);

	tag1p = getstring(&workp);
	tag2p = getstring(&workp);

	if (getstring(&workp) != NULL)
		return(ERR2MNY);

	switch (i) {

	case DECK:	/* filename */
	case NEWDECK:	/* filename */
		if (tag2p != NULL)
			return(ERR2MNY);
		if (tag1p == NULL)
			return(ERR2FEW);
		if ((errrtn = getname(endpath(tag1p), namebuf)) < 0)
			ERROR();
		break;

	case IDENT:	/* modset */
		if (tag2p != NULL)
			return(ERR2MNY);
		if (tag1p == NULL)
			return(ERR2FEW);
		if ((errrtn = getname(tag1p, namebuf)) < 0)
			ERROR();
		break;

	case MODNAME:	/* [modset] */
		if (tag2p != NULL)
			return(ERR2MNY);
		if (tag1p == NULL)
			clrname(dfltmod);
		else
			if ((errrtn = getname(tag1p, dfltmod)) < 0)
				ERROR();
		break;

	case PREFIX:	/* [character] */
		if (tag2p != NULL)
			return(ERR2MNY);
		if ((s = tag1p) == NULL)
			uescape = dfltescape;
		else if (s[1] == '\0')
				uescape = s[0];
			else
				return(ERRPREF);
		break;

	case AFTER:	/* linetag */
	case BEFORE:	/* linetag */
		if (tag2p != NULL)
			return(ERR2MNY);
		if (tag1p == NULL)
			return(ERR2FEW);
		if ((errrtn = gettag(tag1p, &tag1)) < 0)
			ERROR();
		break;

	case UAPPEND:	/* */
		if (tag1p != NULL)
			return(ERR2MNY);
		break;

	case UDELETE:	/* linetag[,linetag] */
	case RESTORE:	/* linetag[,linetag] */
		if (tag1p == NULL)
			return(ERR2FEW);
		if ((errrtn = gettag(tag1p, &tag1)) < 0)
			ERROR();
		if (tag2p == NULL) {
			copyname(tag1.t_name, tag2.t_name);
			tag2.t_line = tag1.t_line;
		} else
			if ((errrtn = gettag(tag2p, &tag2)) < 0)
				ERROR();
		break;

	case YANK:	/* modset[,*] */
	case UNYANK:	/* modset[,*] */
		if (tag1p == NULL)
			return(ERR2FEW);
		if ((errrtn = getname(tag1p, tag1.t_name)) < 0)
			ERROR();
		tag1.t_line = 0;	/* set yank type */
		if ((s = tag2p) != NULL) {
			if (s[0]!='*' || s[1]!='\0')
				tag1.t_line = 1;
			else {
				if ((errrtn = getname(tag2p, tag2.t_name)) < 0)
					ERROR();
				tag1.t_line = 2;
			}
		}
		break;

	case EXIT:	/* */
		if (tag1p != NULL)
			return(ERR2MNY);
		break;
	}
	return(i);

errlabl:
	return(errrtn);
}
