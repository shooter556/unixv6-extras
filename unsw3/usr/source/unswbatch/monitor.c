#include  "batch.h"
/*
 *    this program is execed by the process which forked all the batch
 *    processes.  it's purpose is to wait until all these processes die,
 *    print a message and die.
 *
 *
 *                                           ian j.  march '76
 *
 */
main(argc,argv)
char **argv;int argc;
{
       int status[1] ;
	prints("\n*** batch now active ***\n");
       while( wait(status) != -1 ) ;
       chkpt();
       actflag=0;
       sync();
       prints("\n** batch has completed\n");
}
