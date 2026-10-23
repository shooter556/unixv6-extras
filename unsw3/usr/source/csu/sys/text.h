/*
 * Text structure.
 * One allocated per pure
 * procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	int	x_daddr;	/* disk address of segment */
	int	x_caddr;	/* core address, if loaded */
	int	x_size;		/* size (*64) */
	int	*x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	int	*x_spid;	/* to record special r/w text id */
	int	x_flags;	/* flags for locking etc. */
} text[NTEXT];

#define	XBUSY	01
#define	XWANT	02
