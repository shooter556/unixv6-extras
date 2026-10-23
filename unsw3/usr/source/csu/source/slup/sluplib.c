#

/*
 * Library manipulation
 */

#include "slup.h"

/*
 * Layout of library file:
 *
 *	1.	"lheader" structure
 *	2.	lheader.l_files * "filentry" structures
 *	3.	lheader.l_mods * "modentry" structures
 *	4.	modules referenced by "filentry" and "modentry" structures
 */

/*
 * Find a referred to library and open it.
 * Then load the directory into memory.
 */
getlib(xtend)
int	xtend;
{
	register int	i;
	register struct filentry	*fp;
	register struct modentry	*mp;

	nfiles = 0;
	mxnfiles = 0;
	nmods = 0;
	mxnmods = 0;

	if (options & OPTNEWL) {
		libfile = 0;
		lheader.l_magic = LMAGIC;
		lheader.l_crdate = now;
		lheader.l_modate = now;
		lheader.l_files = 0;
		lheader.l_mods = 0;
		for (i = 0; i < 16; i++)
			lheader.l_groups[i][0] = '\0';
		libmods = 1;
	} else {
		if ((libfile = open(libname, 0)) < 0)
			return(terror("Can't access library - %s", libname));
		if (read(libfile, &lheader, sizeof lheader) != sizeof lheader ||
		    lheader.l_magic != LMAGIC)
			return(terror("Invalid SLUP library - %s", libname));
		xtend =+ lheader.l_files;
	}

	if (xtend != 0)
		if ((i = growsrc(xtend)) < 0)
			return(i);
	if ((nfiles = lheader.l_files) != 0) {
		i = nfiles * (sizeof *filentry);
		if (read(libfile, filentry, i) != i)
			return(terror("Can't read library directory"));
	}
	for (i = 0, fp = filentry; i < nfiles; i++, fp++)
		fp->f_flags =& (FXCOMMN | FXNEW);
	if (lheader.l_mods != 0) {
		if ((i = growmods(lheader.l_mods)) < 0)
			return(i);
		nmods = lheader.l_mods;
		i = nmods * (sizeof *modentry);
		if (read(libfile, modentry, i) != i)
			return(terror("Can't read modification directory\n"));
		for (i = 0, mp = modentry; i < nmods; i++, mp++)
			mp->m_flags = 0;
	}
	return(NOERR);
}

/*
 * Produce a new copy of the library for containing the
 * updates.  When complete it is copied back.
 * Will assume D/A temporary file.
 */
putlib()
{
	register int	i;
	register struct filentry	*fp;
	register struct modentry	*mp;
	int	f;

	if (!libmods || errflag)
		return(NOERR);
	lheader.l_modate = now;
	lheader.l_files = nfiles;
	lheader.l_mods = nmods;
	lheader.l_size = sizeof lheader + (nfiles * (sizeof *filentry)) + (nmods * (sizeof *modentry));
	lseek(workfid, lheader.l_size, 0);

	for (i = 0, mp = modentry; i < nmods; i++, mp++) {
		f = (mp->m_flags & MUUPDAT) ? scrfid : libfile;
		lseek(f, mp->m_base, 0);
		if (copyf(f, workfid, mp->m_size) < 0)
			ERROR();
		mp->m_base = lheader.l_size;
		lheader.l_size =+ mp->m_size;
	}

	for (i = 0, fp = filentry; i < nfiles; i++, fp++) {
		f = (fp->f_flags & FXUPDAT) ? scrfid : libfile;
		lseek(f, fp->f_base, 0);
		if (copyf(f, workfid, fp->f_size) < 0)
			ERROR();
		fp->f_base = lheader.l_size;
		lheader.l_size =+ fp->f_size;
	}
	close(scrfid);
	scrfid = 0;
	unlink(scrtemp);
	i = lheader.l_files * (sizeof *filentry);
	f = lheader.l_mods * (sizeof *modentry);
	REWIND(workfid);
	if (write(workfid, &lheader, sizeof lheader) != sizeof lheader ||
	    write(workfid, filentry, i) != i ||
	    write(workfid, modentry, f) != f) {
errlabl:
		close(workfid);
		workfid = 0;
		unlink(worktemp);
		return(terror("Can't complete temp. library"));
	}
	if (!(options & OPTNEWL))
		close(libfile);
	if ((libfile = creat(libname, 0600)) < 0)
		return(terror("Can't update old library"));
	REWIND(workfid);
	if (copyf(workfid, libfile, lheader.l_size) < 0) {
		unlink(libname);
		terror("Can't copy temporary library into place");
		return(terror("remains at - %s", worktemp));
	}
	close(workfid);
	workfid = 0;
	close(libfile);
	libfile = 0;
	unlink(worktemp);
	return(0);
}
