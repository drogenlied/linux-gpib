#include <ib.h>
#include <ibP.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>

/***********************************************************************
 * A simple Pattern matcher
 *  first converts wildcard characters into regular expression
 *  then executes a regular expression match
 *
 ***********************************************************************/
char *ast=".*";                /* replacement for '*' */
char *dot="\\.";               /* replacement for '.' */

PRIVATE int do_match(char *pattern,char *match)
{
	regex_t regex_buffer;

	regcomp(&regex_buffer, pattern, 0);
	return regexec(&regex_buffer, match, 0, NULL, 0);
}

