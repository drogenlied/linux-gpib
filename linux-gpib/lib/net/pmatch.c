#include <ib.h>
#include <ibP.h>
#include <regex.h>


/***********************************************************************
 * A simple Pattern matcher
 *  first converts wildcard characters into regular expression
 *  then executes a regular expression match
 *
 ***********************************************************************/
char *ast=".*";                /* replacement for '*' */
char *dot="\\.";               /* replacement for '.' */

int do_match(char *pattern,char *match)
{
  char regl[100];
  int i,r;
  regex_t regex_buffer;


  if( strlen(pattern) == 1 ) return 1;   /* pattern was '*' */

  regl[0]='\0';

  i=0;
  while( i< strlen(pattern) && i<100){
    if( pattern[i] == '*' )
      strcat(regl,ast);
    else if ( pattern[i] == '.' )
      strcat(regl,dot);
    else 
      strncat(regl,&pattern[i],1);
    i++;
  }

  re_comp(regl);


  if( re_exec(match) == 1 ){
    return 1;
  }
  return 0;
}
