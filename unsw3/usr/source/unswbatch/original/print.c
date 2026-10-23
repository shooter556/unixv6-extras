#include "batch.h"
/*
 */
#define m1 "\n\n\n#### print deleted by operator\n",34
#define m2 "\n\n\n#### no listing file\n",26
char m3[]
"\013\376\210\r\376\210\r\014\n\n\n\n\n\377\020unix edition-6 local batch\n\n\377\060";
char m4[]
"\377\000***** ???????\n\n";

main(argc,argv)
char **argv; int argc;
{
	extern ijtime();
	int buff[1024]; int register oflag,n; register struct quelt *q;
	if(argc) exit();
	oflag=1;
	for(;;) {
		q=qget(print);
		q->prtime = dtime(); /* note starting time */
		n=nopen(q->nofile[0],0);
		if(oflag){
			while(open("/dev/lp",1)<0) sleep(10); /* fd=4 */
			oflag=0;
		}
		if(n<0) { write(4,m2,sizeof m2); goto finish; }; /* fd = 0 */
	if( !(q->origin & tycyb) ){
/* start the banner page  */
  againloc:
		write(4,m3,sizeof m3);
		write(4,ijtime(),25);
		write(4,"\n\n",2);
		m4[8] = q->jobn[0]; m4[9] = q->jobn[1];
		m4[10] = q->jobn[2]; m4[11] = q->jobn[3];
		m4[12] = q->jobn[4]; m4[13] = q->jobn[5];
		m4[14] = q->jobn[6];
		for(m4[1]=40;m4[1]<64;m4[1]++)
				write(4,m4,sizeof m4);
		write(4,m3,8);
/* end the banner page */
	} else ncybjobs++;
  againcyb:
		sprtflag=0;
		while( !sprtflag && (n=read(0,buff,sizeof buff)))
				write(4,buff,n);
		if(sprtflag==1) write(4,m1);
			else if(sprtflag==2) {
				seek(0,0,0);  /* go to start of file */
				write(4,"\n\n... print repeated by operator\014",33);  /* skip to new page */
				if( !(q->origin & tycyb) )
						goto againloc;
					else    goto againcyb;
			}
	finish:	close(0);
		nunlink(q->nofile[0]);
		q->extime = dtime();
		write(1,q->jobn,statrln);
		qput(q,free);
		if( !stat("/tmp/lpd/lock",buff) || !qhdr[print].qnum){
			close(oflag=4);
			sleep(10);	/* give "lpd" a look in */
		}
	}
}
