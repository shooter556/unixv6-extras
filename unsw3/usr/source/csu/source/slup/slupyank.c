#

/*
 * yank/unyank feature of update
 */

#include "slup.h"

yank(fp, flag)
struct filentry	*fp;
int	flag;		/* yank/unyank */
{
	register struct uwork	*wp, *wdp, *wyank;
	int	i;
	int	msetbuf, id;
	long	inleng;
	long	xinleng;
	int	errrtn;
	int	cmd;
/***/	long	_u_l;

	if ((errrtn = getupdt(&ftmp1, fp, 0)) < 0)
		return(errrtn);

	if ((wp = findupdt(tag1.t_name, uwork, works)) == NULL)
		ERROR(errrtn = ERRMFND);

	switch (tag1.t_line) {

	case 0:	/* name */
		wdp = wp;
		break;

	case 1:	/* name,* */
		wdp = &uwork[works - 1];
		break;

	case 2:	/* name1,name2 */
		if ((wdp = findupdt(tag2.t_name, uwork, works)) == NULL)
			ERROR(errrtn = ERRMFND);
		if (wdp < wp)
			ERROR(errrtn = ERRIMPL);
		break;
	}

	for (;; wp++) {
		if (flag)
			/* yank */
			wp->w_flags =| UXYANKD;
		else
			/* unyank */
			wp->w_flags =& ~UXYANKD;
		wp->w_flags =| UXRUN;
		if (wp == wdp)
			break;
	}

/***	inleng = fp->f_size - fp->f_mods*(sizeof *uentry);	*/
/***/	_u_l = fp->f_mods * (sizeof *uentry);
/***/	inleng = fp->f_size - _u_l;
	REWIND(ufid);
	if ((errrtn = copyf(ftmp1.io_fid, ufid, inleng)) < 0)
		ERROR();

	if (subopts & OPTUPRT) {

		REWIND(ufid);
		FINSET(&ftmp1, ufid);

		wp = uwork;
		if (fp->f_flags & FXNEW)
			wp++;
		wyank = NULL;
		wdp = wp;
		for (xinleng = inleng; --xinleng >= 0;) {
			if ((i = getc(&ftmp1)) == '\0') {
				id = getc(&ftmp1);
				cmd = getc(&ftmp1);
				switch (cmd) {

				case UOWN0:
				case UOWN1:
					wp = &uwork[id];
					wp->w_cmd = 0;
					msetbuf = getc(&ftmp1);
					wp->w_count = getw(&ftmp1);
					xinleng =- 5;
					if (wp->w_flags & UXRUN) {
						kickline();
						printf("%-14.14s.******", wp->w_name);
						printf(" %c\t", wp->w_flags&UXYANKD ? '-' : '+');
						printf(cmd==UOWN0 ? "*after" : "*before");
						printf(" %.14s", uwork[msetbuf].w_name);
						printf(".%u", uwork[msetbuf].w_lines + (cmd != UOWN0));
					}
					if (!(wp->w_flags & UXYANKD) && uwork[msetbuf].w_flags&UXYANKD)
						error(ERRYANK);
					break;

				case UOWN2:
					wp = &uwork[id];
					wp->w_cmd = 0;
					xinleng =- 2;
					wp->w_flags =| UXEND;
					if (wp->w_flags & UXRUN) {
						kickline();
						printf("%-14.14s.******", wp->w_name);
						printf(" %c\t*append", wp->w_flags&UXYANKD ? '-' : '+');
					}
					break;

				case UDEL:
				case URESTOR:
					wdp = &uwork[id];
					wdp->w_cmd = cmd;
					msetbuf = getc(&ftmp1);
					wdp->w_count = getw(&ftmp1);
					xinleng =- 5;
					if (wdp->w_flags & UXRUN) {
						kickline();
						printf("%-14.14s.******", wdp->w_name);
						printf(" %c\t", wdp->w_flags&UXYANKD ? '-' : '+');
						printf(cmd==UDEL ? "*delete" : "*restore");
						printf(" %.14s", wp->w_name);
						printf(".%u", wp->w_lines + 1);
						if (msetbuf != (wp - uwork) ||
						    wdp->w_count != (wp->w_lines + 1)) {
							printf(",%.14s", uwork[msetbuf].w_name);
							printf(".%u", wdp->w_count);
						}
					}
					wdp->w_modfind = msetbuf;
		setwyank:
					wyank = NULL;
					for (wdp = &uwork[works - 1]; wdp != uwork; wdp--)
						if (wdp->w_cmd != 0) {
							if (uwork[wdp->w_modfind].w_lines >= wdp->w_count) {
								wdp->w_cmd = 0;
								wdp->w_count = 0;
								continue;
							}
							if (wdp->w_flags&UXRUN && wyank==NULL)
								wyank = wdp;
							if (!(wdp->w_flags & UXYANKD))
								break;
						}
					break;
				}
			} else {
				for (alinep = aline; i != '\n';) {
					*alinep++ = i;
					i = getc(&ftmp1);
					xinleng--;
				}
				*alinep = '\0';
				wp->w_lines++;
				wp->w_count--;
					
				if (wp->w_flags&UXRUN || wyank!=NULL) {
					kickline();
					printf("%-14.14s", wp->w_name);
					printf(".%6u", wp->w_lines);
					printf(" %c\t", wp->w_flags&UXYANKD ? '-' : '+');
					println(aline);
				}

				if ((wdp!=uwork && wdp->w_count==wp->w_lines && wdp->w_modfind==(wp-uwork)) ||
				    (wyank!=NULL && wyank->w_count==wp->w_lines && wyank->w_modfind==(wp-uwork)))
					goto setwyank;
			}
			while (!(wp->w_flags & UXEND) && (wp->w_count==0 || wp->w_cmd!=0))
				wp--;
		}
	}
	return(putupdt(fp, inleng, ufid));

errlabl:
	free(uwork);
	mxworks = 0;
	works = 0;
	return(errrtn);
}
