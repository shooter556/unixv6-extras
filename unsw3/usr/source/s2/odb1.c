#include	"odb7.h"
#define	RAW	040
#define	ESC	033

char *signam[]
	{
	"",
	"Hangup",
	"Interrupt",
	"Quit",
	"Illegal instruction",
	"Trace/BPT",
	"IOT",
	"EMT",
	"FP exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad argument to sys call",
	"Write on a pipe with no one to read it",
	"",""
	};
extern	int	namfil;
cmndch()
	{
	register cnts;
	register *p;
	while(1)
	switch(ch) {
	case ESC:
		putchar(032);
		if ((ch=getchar()) == RUB)
				{
				printf("####  ");
				return;
				}
		rdline();
		switch(ch)
		{
		case 'i' :
		case 'o': case 'd': case 'c':
		if (bytwrd>2)
			bytwrd = 2;
		cmode=ch;
		if (rch()==';' && bytwrd)
			{
			putlp(" ");
			getcont();
			return;
		}
		continue;
		case 't':
			write(1,"\r\n", 2);
			getnfil();
			prstab();
			return;
		default:
			error();
		}
	case '\\':
	case '/' :
		if (valfnd) 
			{
			dot=adrval;
			if ((orgfil=curfil) != 0) ch='/';
			}
		bytwrd = ((ch=='/') || (cmode=='i') ? 2 : 1);
		getcont();
		return;
	case '^':
		putlp("\n");
	case '\n':
		uparlf();
		return;
	case '\r':
		closloc();
		clear();return;
	case '<': case '*': case '>':
		closloc();
		if (bytwrd < 2)
			error();
		else {
			cnts = newval(orgfil, &dot);
			orgfil = ODBEXT;
			 switch(ch)
			{
			case '<':
			 dot =+ (cnts+2);
			 break;
			case '*':
			 dot = cnts;
			 break;
			case '>':
			 dot = (( cnts & BMASK) << 1)+2;
			 break;
			}
		}
		newline();
		openxt();
		return;
	case '=':
		equal();
		return;
	case '%':
		if (pid)
			{
			printf("\n\rProcess not terminated, yet!!\n\r%%");
			reset();
			}
		setty(1);
		scall();
		write(1,"%\n*",3);
		setty(RAW);
		  return;
	case ';':
		++scflg;
		tadrval=adrval;
		svalfnd=valfnd;
		rdline();
		rch();
		valfnd = expr();
		continue;
	case 't':
		if (pid)
			{
			ptrace(8, pid, 0, 0);
			pid = 0;
			cbplist();
			printf("\n\rProcess terminated.");
			}
		reset();
	case 'g':
		if ( adrval&1 )
			error();
		go();
		waitin();
		bpoint();
		ptrace(6, pid, r[8]<<1, 0170000);
				/* sets user mode */
	case 'p':
		if (!pid)
			error();
		bpoint();
		setty(1);
		printf("\n");
		prosid();
		return;
	case 'b':
		breakp();
		clear();
		return;
	case ':' :
		getlin(1);
		setty(RAW);
		colon();
		continue;
	default:
		error();
		   }
	}
