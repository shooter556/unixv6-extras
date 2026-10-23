/*
**	structure to describe contents of mount table
*/

#define	NMOUNT	16
#define	NAMSIZ	32

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
	int	prot;
} mtab[NMOUNT];

char	mtabf[]	"/etc/mtab";
