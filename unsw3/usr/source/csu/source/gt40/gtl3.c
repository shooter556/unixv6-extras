#

/*
 * program loading routines:-
 *		loadodt
 *		loadfile
 *			extras
 */

#include "gtl.h"

loadodt()
{
	register int	flag;
	register char	*lo, *siz;

	if (freopen(ODTFILE, "r", stdin) == NULL) {
		fputs("Can't access odt file\n", stderr);
		return(-1);
	}
	flag = -1;
	if (fread(&odthdr, 1, sizeof odthdr, stdin) != sizeof odthdr)
		goto bad;
	if (odthdr.odt_magic!=0577 || odthdr.odt_pad!=1 || odthdr.odt_beg==0 || odthdr.odt_siz==0)
		goto bad;
	if ((odthdr.odt_start & 1) || (odthdr.odt_pc & 1))
		goto bad;
	if (odthdr.odt_start==0 || odthdr.odt_pc==0)
	bad:
		fputs("Bad ODT file\n", stderr);
	else {
		lo = odthdr.odt_beg;
		siz = odthdr.odt_siz;
		if ((lo + siz) > MEMSIZE)
			fputs("odt too high in memory\n", stderr);
		else if (fread(&mem[lo], 1, siz, stdin) == siz)
			flag = 0;
		else
			fputs("Can't load odt\n", stderr);
	}
	fclose(stdin);
	return(flag);
}

/*
 * loadfile: read the file into the memory buffer.
 * we determine the file format here.
 */
loadfile()
{
	register int	i;
	register char	*p;
	int	stat;

	/*
	 * We now examine the beginning of the file to determine
	 * whether or not it is an "a.out" format file or an
	 * "absolute loader" format file.
	 */
	i = fread(aouthdr, 1, sizeof aouthdr, stdin);
	if ((i==sizeof aouthdr) && (aouthdr[0]==0407 || aouthdr[0]==0410)) {
		/* a.out format */

		startadr = aouthdr[5] & ~01;	/* make even */
		stat = 0;	/* initialize for i/o errors */

		if ((i = aouthdr[1]) != 0) {
			/* text segment */
			if (i >= MEMSIZE) {
				fputs("Text too big\n", stderr);
				return(-1);
			}
			insert(0, i);
			if (fread(mem, 1, i, stdin) != i)
				stat++;
		}

		if (!stat && (i = aouthdr[2])!=0) {
			/* data segment */
			p = aouthdr[1];
			if (aouthdr[0] == 0410)
				/* separate text and data areas */
				p = (p + 017777) & ~017777;
			if ((p + i) >= MEMSIZE) {
				fputs("Data segment size\n", stderr);
				return(-1);
			}
			insert(p, i);
			if (fread(&mem[p], 1, i, stdin) != i)
				stat++;
		}

		if (stat) {
			fputs("i/o error in file\n", stderr);
			return(-1);
		}
	} else {
		/* must try for absolute loader format */
		fseek(stdin, 0l, 0);
		do {
			p = loadrec();
		} while (p == NULL);
		if (*p != '\0') {
			fputs("Input file no good: ", stderr);
			fputs(p, stderr);
			fputs("\n", stderr);
			return(-1);
		}
		absfmt++;	/* save flag */
	}
	return(0);
}

/*
 * loadrec: used by loadfile to read absolute loader format records
 */
loadrec()
{
	int	count, adr;	/* note: these must be memory locns. */
	register int	ct, c;
	register char	*p;
	int	flg;

	/* skip through leader */
	checksum = 0;
	do {
		c = getbyte();
	} while (c == 0);

	/* now for header of record */
	if (c!=01 || getbyte()!=0)
		return("bad header");

	/* and address and count fields */
	if (getword(&count) || getword(&adr))
		return("short record(header)");
	if ((count =- 6) < 0) {
		/* bad count */
		return("bad count");
	}
	if (count != 0) {
		/* real data record */
		if (adr+count > MEMSIZE)
			return("Memory limit exceeded");
		ct = count;
		p = &mem[adr];
		do {
			if ((c = getbyte()) < 0)
				return("short record(data)");
			*p++ = c;
		} while (--ct != 0);
		insert(adr, count);
		flg = 1;
	} else {
		/* start address */
		startadr = adr;
		flg = 0;
	}
	getbyte();	/* check-sum */
	if (checksum & 0377)
		return("checksum");
	if (flg)
		return(NULL);
	else
		return("");
}

/*
 * getbyte: called by loadrec and getword to read
 * one character and keep continuous checksum.
 */
getbyte()
{
	register int	c;

	if ((c = getchar()) == EOF)
		return(-1);
	checksum =+ c;
	return(c);
}

struct	{
	char	lobyte, hibyte;
	};

getword(word)
int	*word;
{
	register int	c, *p;

	p = word;
	if ((c = getbyte()) < 0)
		return(-1);
	p->lobyte = c;
	if ((c = getbyte()) < 0)
		return(-1);
	p->hibyte = c;
	return(0);
}
