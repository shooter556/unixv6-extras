main(argc,argv)
char **argv;
{

	close(0);

	argv[argc] = 0;

	if( execv( argv[1], &argv[2]) );
write(2,"execv failed\n",13);
}
