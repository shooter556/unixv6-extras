/*
 * access -- V6 has no access system call, so do it with stat.
 * mode is 4 read, 2 write, 1 execute, 0 exists.
 * Like the real one, it checks the real user and group ids.
 * Returns 0, or -1 with errno set.
 */
access(name, mode)
char *name;
{
	extern errno;
	int st[18];
	register m, uid;

	if (stat(name, st) < 0)
		return(-1);
	uid = getuid() & 0377;
	if (uid == 0)
		return(0);
	m = st[2];
	if (uid == ((st[3] >> 8) & 0377))
		m =>> 6;
	else if ((getgid() & 0377) == (st[4] & 0377))
		m =>> 3;
	if ((m & mode & 07) == (mode & 07))
		return(0);
	errno = 13;
	return(-1);
}
