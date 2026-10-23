main()
{
	int kmem,tape;
	int word[1];

	nice(-128);	/* get some service */
	if( (tape=open("/dev/rmt0",0))<0 ){
		prints("Tape unit in use/No permission\n");
	}
	if( (kmem = open("/dev/kmem",2))<0 ) {
		prints(2," can't open kmem\n");
		exit(-1);
	}
	close(tape);
	seek(kmem,0172450,0);
	read(kmem, word ,2);
	word[0] =& 0177770;		/* select drive zero */
	seek(kmem,0172450,0);
	write(kmem, word ,2);
	seek(kmem,0172440,0);
	read(kmem, word , 2);
	word[0] =& 0177700;
	word[0] =| 3;			/* rewind off-line */
	seek(kmem,0172440,0);
	write(kmem, word ,2);
}
