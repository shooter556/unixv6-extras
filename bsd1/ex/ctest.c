/*
 * ctest -- ex_tty.c makes the terminal type as buf[0] | buf[1] << 8
 * and ex_ca.c compares it with constants like 'vt', so the compiler
 * must put the first character of a constant in the low byte.
 */
main()
{
	if ('vt' != ('v' | 't' << 8))
		write(1, "two character constants are the wrong way round\n", 49);
}
