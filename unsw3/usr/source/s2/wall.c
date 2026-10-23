#include	<local-system>
char	mesg[3000];
int	msize;
#ifndef	AUSAM
struct
{
	char	name[8];
	char	tty[2];
	int	time[2];
	int	junk;
} utmp[50];
#endif
#ifdef	AUSAM
#include	<utmp.h>
struct	utmp	utmp[NTTYS];
#endif

main(argc, argv)
char *argv[];
{
	register i, *p;
	int f;

	f = open("/etc/utmp", 0);
	if(f < 0) {
		printf("utmp?\n");
		exit();
	}
	read(f, utmp, sizeof utmp);
	close(f);
	f = 0;
	if(argc >= 2) {
		f = open(argv[1], 0);
		if(f < 0) {
			printf("%s?\n", argv[1]);
			exit();
		}
	}
	while((i = read(f, &mesg[msize], sizeof mesg - msize)) > 0)
		msize =+ i;
	close(f);
	for(i=0; i<sizeof utmp/sizeof utmp[0]; i++) {
		p = &utmp[i];
#ifndef	AUSAM
		if(p->tty[0] == 0)
			continue;
		sleep(1);
		sendmes(p->tty[0]);
#endif	AUSAM
#ifdef	AUSAM
		if(p->u_type == 0)
			continue;
		sendmes(p->u_ttyid);
#endif	AUSAM
	}
}

sendmes(tty)
{
	register i;
	register char *s;

#ifndef	AUSAM
	i = fork();
	if(i == -1) {
		printf("try again\n");
		return;
	}
	if(i)
		return;
#endif	AUSAM
	s = "/dev/ttyx";
	s[8] = tty;
	i = open(s, 1);
	if(i < 0) {
		s = "cannot open ttyx\n";
		s[15] = tty;
		perror(s);
#ifndef	AUSAM
		exit();
#endif	AUSAM
#ifdef	AUSAM
		return;
#endif	AUSAM
	}
	write(i, "\007\007\007Broadcast Message ...\n\n", 26);
	write(i, mesg, msize);
#ifndef	AUSAM
	exit();
#endif	AUSAM
#ifdef	AUSAM
	close(i);
	return;
#endif
}
