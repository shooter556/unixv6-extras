/*
 * stest -- check libS.a
 * prints "stdio ok" or the number of the first failing test
 * the output is written by exit, so run it with stdout sent to a file
 * to check that buffers are flushed on exit
 */

#include <stdio.h>

main()
{
	register FILE *f;
	register n;
	int i;
	char buf[100];

	f = fopen("/tmp/stest", "w");
	if (f == NULL) fail(1);
	fprintf(f, "%d %s %o\n", 42, "abc", 8);
	for (i = 0; i < 600; i++)
		putc('x', f);
	putc('\n', f);
	putw(12345, f);
	fclose(f);

	f = fopen("/tmp/stest", "r");
	if (f == NULL) fail(2);
	if (fgets(buf, 100, f) == NULL) fail(3);
	if (strcmp(buf, "42 abc 10\n") != 0) fail(4);
	n = 0;
	while ((i = getc(f)) == 'x')
		n++;
	if (n != 600 || i != '\n') fail(5);
	if (getw(f) != 12345) fail(6);
	if (getc(f) != EOF) fail(7);
	fseek(f, 3L, 0);
	if (getc(f) != 'a') fail(8);
	ungetc('q', f);
	if (getc(f) != 'q') fail(9);
	rewind(f);
	if (getc(f) != '4') fail(10);
	fclose(f);
	unlink("/tmp/stest");

	sprintf(buf, "%s-%d", "ab", -7);
	if (strcmp(buf, "ab--7") != 0) fail(11);
	sscanf("17 xyz", "%d %s", &i, buf+50);
	if (i != 17 || strcmp(buf+50, "xyz") != 0) fail(12);
	strcpy(buf, "ab");
	strcat(buf, "cd");
	if (strcmp(buf, "abcd") != 0 || strlen(buf) != 4) fail(13);
	if (calloc(10, 2) == NULL) fail(14);
	if (i < 0)
		perror("stest");

	printf("stdio ok\n");
	exit(0);
}

fail(n)
{
	printf("stdio FAIL %d\n", n);
	exit(1);
}
