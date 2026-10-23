#include "sd.h"
main()
{
	for(;;){
		pp(lock1);
		printf("the following was passed from sd1\n`%s'\n",sdbuf);
		vv(lock2);
	}
}
