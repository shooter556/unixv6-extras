#

/*
 * Extraction routines
 */

#include "slup.h"

/*
 * Extract an individual original file
 */
exoriginal(fp, fnm)
struct filentry	*fp;
char	*fnm;
{
	register int	c;
	register unsigned int	skips;
	long	inleng;

	/* skip over update directory */
	skips = fp->f_mods * (sizeof *uentry);
	inleng = fp->f_size - skips;
	lseek(libfile, fp->f_base + skips, 0);

	FINSET(&ftmp1, libfile);

	if (fcreat(fnm, &ftmp2) < 0)
		return(ERRNEW);

	skips = 0;
	while (--inleng >= 0)
		if ((c = getc(&ftmp1)) == '\0') {
			getc(&ftmp1);	/* id */
			switch (getc(&ftmp1)) {

			case UOWN0:
			case UOWN1:
				getc(&ftmp1);
				skips =+ getw(&ftmp1);
				inleng =- 5;
				break;

			case UOWN2:
				/* break out of while */
				inleng = 0;
				break;

			case UDEL:
			case URESTOR:
				getc(&ftmp1);	/* search id */
				getw(&ftmp1);	/* search lineno */
				inleng =- 5;
				break;
			}
		} else {
			if (skips == 0)
				/* keep line */
				for (;;) {
					if (c & 0200) {
						for (c =& 0177; c > 0; c--)
							putc(' ', &ftmp2);
					} else {
						putc(c, &ftmp2);
						if (c == '\n')
							break;
					}
					c = getc(&ftmp1);
					inleng--;
				}
			else {
				/* drop line */
				while (c != '\n') {
					c = getc(&ftmp1);
					inleng--;
				}
				skips--;
			}
		}
	fflush(&ftmp2);
	close(ftmp2.io_fid);
	return(NOERR);
}
