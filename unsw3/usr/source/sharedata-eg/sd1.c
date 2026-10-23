#include "sd.h"
main()
{
	register n;

	for(;;){
		printf(" respond 'x' to terminate\n whatever else is typed will be passed to sd2\n");
		if( (n=read(2,sdbuf,200))<=0 ) exit(0);
		if( (sdbuf[0]=='x')&&(sdbuf[1]=='\n') ) exit(0);
		sdbuf[n-1] = 0;
		vv(lock1);
		pp(lock2);
	}
}
