#
#include  "batch.h"
/*
 *    this code will be execed by the "unreal" command to start local
 *    batch facilities and remote job submission.
 *
 *
 *                                           ian j.  march '76
 *
 *	modded by daveh sep 21 76 to incorporate "local"
 *
 */
       char  rbuff[40] 0 ; int null[1] 0;
       int   fbuff[9] 0 ;
                              /********/
                              /********/
main(argc,argv)
char *argv[]; int argc;
{      register struct proc *p; int register i;
	struct quelt *pq,*vq; int xt;
        /*
         *   if 'unreal' already active exit
         */
        if( ++actflag ) { actflag--; error("already active\n\n",null); }
       /*
        *    ensure correct working directory.
        */
       if(chdir("/spool/batch/.work")<0)error("working directory unattainable\n",null);
	/*
	 * daveh   see how called
	 */
	if( argv[0][0] == 'l' ) local++;	/* local */
	if( argc>1 ) nocyb++;		/* no cyber jobs please */
        /*
         *   ensure all necessary files can be opened
         */
        /******/  bopen(".checkpoint", 2 ) ;  /** will be '3' **/
	if( !local ){	/* daveh */
		cupfd  =  bopen("/dev/cyb.cmdsu", 1 ) ;
		jupfd  =  bopen("/dev/cyb.jobsu", 1 ) ;
		jdnfd  =  bopen("/dev/cyb.jobsd", 0 ) ;
		cdnfd  =  bopen("/dev/cyb.cmdsd", 0 ) ;
	}
	crdfd  =  bopen("/dev/cr0"    , 0 ) ;
        statfd  =  bopen(".rawstatistics"    , 1 ) ;
	seek(statfd,0,2);	/* seek to end of the file */
	/*
	 *  what are the system parms ?
	 */
 loop:
	seek(3,0,0);
	if(read(3,chkptp,chkptl)!=chkptl) goto coldgold;
	if(njobs<=0) goto coldgold;	/* may as well re-init */
	if( !fork() ) { execl("/bin/dq","dq",0); exit(); }
	while( wait(&xt) != -1 );
	if( xt & 0200 ) error("dq has crashed...\n",null);
	prints ("re-initialize the batch system ? ");
       read(0,rbuff,sizeof rbuff);
       if( rbuff[0]=='c' && rbuff[1]=='o' &&
           rbuff[2]=='l' && rbuff[3]=='d' &&
           rbuff[4]=='\n' )  {

  coldgold:
/* cold */     prints("directory being cleared & queues reinitialized\n");
               if( (i=open(".",0))<0 ) error("can't access batch directory\n",null);
               seek(i,32,0);  /* skip . .. */
               while(read(i,fbuff,16)==16) {
		       if(fbuff[0] && fbuff[1].lowbyte=='$' )
				if(unlink(&fbuff[1])<0) {
                                       prints("batch: ");
                                       prints(&fbuff[1]);
                                       prints(" can't remove\n");
				}
               }
               close(i);
		basetime = time();
		write(statfd,null,2); write(statfd,&basetime,4);
		qinit();
/* reset various indicators */
		filen = 1; nspj = nqs; njobs = 0; ncybjobs = 125; nblks = 2000;
               chkpt();
       } else  {

/* warm */	prints("queues being restored\n");
		/* reconstitute the queues */
		for(pq=queue;pq<=queuend;pq++)
		   /* only do xt=pq.. then &1 so it works */
		if( (xt=pq->qlink)&1 ) switch( xt=(pq->qtype) ){
		case free:	/* was reading/receiving */
			write(2,pq->jobn,7);
			prints(" - was being read/received\n");
		  freeit:
			if(pq->ndfile) {nunlink(pq->ndfile);pq->ndfile=0;}
			if(pq->npfile) {nunlink(pq->npfile);pq->npfile=0;}
			xt = free;	njobs--;
		  putit0:
			if(pq->nofile[0]) {nunlink(pq->nofile[0]);pq->nofile[0]=0;}
		   putit:
			qhdr[xt].qnum++;
			pq->qlink = qhdr[xt].qptr;
			qhdr[xt].qptr = pq;
			break;
		  case forcyber:/* was sending to cyber */
			write(2,pq->jobn,7);
			prints(" - was being transmitted\n");
			goto putit;
		  case print:	/* was printing */
			write(2,pq->jobn,7);
			prints(" - was printing\n");
			goto putit;
		  case atcyber:	/* was receiving */
			write(2,pq->jobn,7);
			prints(" - was being received\n");
			goto putit0;
	          default:
			write(2,pq->jobn,7);
			prints(" - was executing\n");
			goto putit;
	       } else if( (pq->qtype==atcyber) && (!pq->jobn[7]) ) {
			vq = &qhdr[atcyber].qptr;
			while( vq->qlink != pq ) vq = vq->qlink;
			vq->qlink = pq->qlink;
			write(2,pq->jobn,7);
			prints(" - was unidentified\n");
			goto freeit;
		        }
	}
       /*
        *    now produce a child that will live on .....
        */
	if( i=fork() ){if(i<0) error("couldn't fork initially\n", null);
				else monid = i; exit(0); }
       /*
        *    activate standard processes
        */
       p =  &procs[0];
	do {
		sleep(2);	/* let things settle down */
		while( (i=fork())<0 ){	/* couldn't fork */
			prints("fork failed - retrying\n");
			sleep(1);
		}
		/* fork finally forked */
		if( i>0 ){	/* parent */
			procids[ p - &procs[0] ] = i;
			continue;
		}
		/* child */
		close(p->pfd1);   dup(*p->pfd2);   close(!p->pfd1);
		for(i=4;i<16;close(i++));   /* close all unwanted files */
		execl(p->pfile,p->pfile,0);
		error(p->pfile," exec failed\n");
	} while( (++p)->pfile) ;
	/*
	*    now let the monitor watch these tasks
	*/
	for(i=4;i<16;close(i++));   /* close all unwanted files */
	close(0);close(1);
	sync();
	execl(".monitor","* batch *",0);
	error("can't exec monitor\n",null);
}
                              /********/
                              /********/
bopen(fn,md)
char *fn; int md;
{
	int register i;
	if((i=open(fn,md))<0)  error(fn," can't open \n");
	return(i);
}
                              /********/
                              /********/
error(a,b)
{
	prints("batch: ");  prints(a);  prints(b);
	exit(-1);
}
