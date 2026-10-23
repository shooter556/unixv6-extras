/*
 * ltest -- check the long arithmetic routines, cret, lseek, mktemp and itol
 * prints "long ops ok" or the number of the first failing test
 */

long	a, b, c;
long	itol();
long	lnext();
char	tname[]	"/tmp/ltXXXXX";

main()
{
	register f;
	char buf[2];

	a = 123456L;
	b = 789L;
	if (a*b != 97406784L) fail(1);
	if (a/b != 156) fail(2);
	if (a%b != 372) fail(3);
	if (-a/b != -156) fail(4);
	if (-a%b != -372) fail(5);
	if (a/-b != -156) fail(6);
	if (a%-b != 372) fail(7);
	c = a; c =* b;
	if (c != 97406784L) fail(8);
	c = a; c =/ b;
	if (c != 156) fail(9);
	c = a; c =% b;
	if (c != 372) fail(10);
	if (itol(1, 2) != 65538L) fail(11);
	if (lnext(65535L) != 65536L) fail(14);

	f = creat("/tmp/ltest", 0666);
	close(f);
	f = open("/tmp/ltest", 2);
	lseek(f, 70000L, 0);
	write(f, "x", 1);
	buf[0] = 0;
	lseek(f, 70000L, 0);
	read(f, buf, 1);
	close(f);
	unlink("/tmp/ltest");
	if (buf[0] != 'x') fail(12);

	mktemp(tname);
	if (tname[11] == 'X') fail(13);

	printf("long ops ok\n");
	exit(0);
}

long
lnext(l)
long l;
{
	return(l+1);
}

fail(n)
{
	printf("long ops FAIL %d\n", n);
	exit(1);
}
