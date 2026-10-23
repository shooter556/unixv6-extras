#
/*
 *	Converts the null terminated string pointed to by "s" into an
 *	integer value modulo PWHASHSIZE.
 */

#include	<passwd.h>

pwhash(s)
register char	*s;
{

	long sum;
	struct{
		unsigned hiword;
		unsigned loword;
	};

	sum = 0;
	while( *s ){
		if('0' <= *s && *s <= '9'){
			sum =* 10;
			sum =+ *s - '0';
		}else if('a' <= *s && *s <= 'z'){
			sum =* 26;
			sum =+ *s - 'a';
		}else{
			sum =<< 7;
			sum =+ *s;
		}
		sum.loword =+ sum.hiword;
		sum.hiword = 0;
		s++;
	}
	return((sum.loword&077777) % PWHASHSIZE);
}
