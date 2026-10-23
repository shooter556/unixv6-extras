#include "batch.h"

#define	CARD_READER

/*
 */
#define m1 "cancel: args ??\n",16
#define m2 "restart: args ??\n",17

main(argc,argv)
char **argv; int argc;
{	char register *p;

	switch(argv[0][0]){
	case 'r':
		if(argc<=2)
			execl("/usr/bin/restart", "restart", 0);
		  else  write(2,m2);
		break;
	case 'c':
		p=argv[1];
		if(argc==2)
#ifdef	CARD_READER
		    if((p[0]=='c')&&(p[1]=='r')) scrdflag = 1;
			else
#endif
				if((p[0]=='l')&&(p[1]=='p')) execl("/usr/bin/prstop", "prstop", 0);
			           else write(2,m1);
		    else write(2,m1);
		break;
	}
}
