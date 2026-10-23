
/*	terminal group definitions	*/

struct
{
	char	  tchar;	/* this char identifies this terminal group */
	unsigned  tmask;	/* 16 bit mask for this group */
}
tty_groups[]
{
#ifdef	AGSM
	'a', 0000001,	/* room 422 */
	'b', 0000002,	/* room 436 - not qume */
	'c', 0000004,	/* the qume */
	'd', 0000010,	/* room 437 - ian J */
	'e', 0000020,	/* 662-1666 dialup lines(4) */
	'f', 0000040,	/* 662-1073, 662-1047 dialup lines(4) */
	'g', 0000100,	/* console - tty8 */
	'h', 0000200,	/* library terminals */
	'i', 0000400,	/* LOG line from elec */
	'j', 0001000,	/* LOG line from csu */
	'k', 0002000,	/* admin lines */
	'l', 0004000,	/* pges */
	'm', 0010000,	/* psychology */
	'n', 0020000,	/* mining */
	'o', 0040000,	/* commerce */
	'p', 0100000,	/* the qume during priority periods */
#endif	AGSM
};

#define	NTTYGRPS	(sizeof tty_groups/sizeof tty_groups[0])
