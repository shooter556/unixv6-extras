#include	<local-system>

/*
mv [-d] file1 file2

unlink file2
link file1 file2
unlink file1
*/



int stbuf[42];
struct sbuf {
int dev;
int inum;
int imode;
#ifdef	AUSAM
	unsigned uid;
#endif
char nlink;
#ifndef	AUSAM
char uid;
char gid;
#endif
char siz0;
char siz1;
int addr[8];
int adate[2];
int mdate[2];
};
char strbuf[70];

main(argc,argv)
int argc;
char *argv[];
{
char **argp;
char *argp1, *argp2, *argp3, *argp4;
char *p, *p1, *p2;
char place[100];
int i;
int status;
int b;
#ifdef	AUSAM
	unsigned u_id;
#endif
argp = argv;
/*
	check for correct number
	of arguments
*/
if(argc != 3){
	prints(2, "Usage: mv name1 name2\n");
	exit( -1 );
}
/*
	is there anything to do?
*/
argp3 = argp[1];
argp4 = argp[2];
#ifndef	AUSAM
if(stat(argp[1], stbuf) < 0){
#endif
#ifdef	AUSAM
if(newstat(argp[1], stbuf) < 0){
#endif
	prints(2, "Source file non-existent\n");
	exit( -1 );
}
/*
	yes, there is a source.
	check whether file or directory
*/
if((stbuf[0].imode & 060000) == 040000){
/*
	The source is a directory, so
	we do lots of checking and
	messing around so as not to get
	into trouble.  This patch of
	code contains administrative
	policies rather than system
	restrictions.
*/
#ifndef	AUSAM
	if(stat(argp[2], stbuf) >= 0){
#endif
#ifdef	AUSAM
	if(newstat(argp[2], stbuf) >= 0){
#endif
		prints(2, "Directory target exists.\n");
		exit( -1 );
	}
	argp1 = argp[1];
	argp2 = argp[2];
	while(*argp1 == *argp2){
		argp1++;
		if(*argp2++ == 0){
			prints(2, "???\n");
			exit( -1 );
		}
	}
	while(*argp1)if(*argp1++ == '/'){
		prints(2, "Directory rename only\n");
		exit( -1 );
	}
	while(*argp2)if(*argp2++ == '/'){
		prints(2, "Directory rename only\n");
		exit( -1 );
	}
	if(*--argp1 == '.'){
		prints(2, "Can't move . or .. !\n");
		exit( -1 );
	}
}else{
/*
	the source is a file.
*/
#ifndef	AUSAM
	setuid(getuid());
#endif
#ifdef	AUSAM
	setuid(u_id = getreal());
#endif
#ifndef	AUSAM
	if(stat(argp4, &stbuf[2]) >= 0){
#endif
#ifdef	AUSAM
	if(newstat(argp4, &stbuf[2]) >= 0){
#endif
		if((stbuf[2].imode & 060000) == 040000){
			argp2 = strbuf;
			while(*argp2++ = *argp4++);
			argp2[-1] = '/';
			argp4 = argp[1];
			argp1 = argp[1];
			while(*argp4)
				if(*argp4++ == '/')
					argp1 = argp4;
			while(*argp2++ = *argp1++);
			argp4 = strbuf;
		}
#ifndef	AUSAM
		if(stat(argp4, &stbuf[2]) >= 0){
#endif
#ifdef	AUSAM
		if(newstat(argp4, &stbuf[2]) >= 0){
#endif
			if((stbuf[0]==stbuf[2]) && (stbuf[1]==stbuf[3])){
				prints(2, "Files are identical.\n");
				exit( 0 );
			}
#ifndef	AUSAM
			if((getuid()&0377) == stbuf[2].uid)
				b = 0200; else
			if((getgid()&0377) == stbuf[2].gid)
				b = 020; else
				b = 02;
#endif
#ifdef	AUSAM
			if ( u_id == stbuf[2].uid )
				b = 0200;
			else
				b = 02;
#endif
			if((stbuf[2].imode & b) == 0) {
				printf("%s: %o mode ", argp4,
					stbuf[2].imode & 07777);
				i = b = getchar();
				while(b != '\n' && b != '\0')
					b = getchar();
				if(i != 'y')
					exit(0);
			}
			if(unlink(argp4) < 0){
				prints(2, "Cannot remove target file.\n");
				exit( -1 );
			}
		}
	}
}
if(link(argp3, argp4) < 0){
	i = fork();
	if(i == -1){
		prints(2, "Try again.\n");
		exit(0);
	}
	if(i){
		while(waitx(&status) != i);
	}else{
		p = place;
		p1 = p;
		while(*p++ = *argp3++);
		p2 = p;
		while(*p++ = *argp4++);
		execl("/bin/cp","cp", p1, p2, 0);
		prints(2, "no cp\n");
		exit(1);
	}
	if((status & 0377) != 0){
		prints(2, "?\n");
		exit( -1 );
	}
	if(status != 0) exit( -1 );
}
if(unlink(argp3) < 0){
	prints(2, "Cannot unlink source file.\n");
	exit( -1 );
}
}

getchar()
{
	char c;

	if(read(0, &c, 1) != 1) return(0);
	return(c);
}
