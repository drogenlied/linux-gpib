
#include <ib.h>
#include <ibP.h>

PUBLIC int ibIsMaster(int ud)
{
  if( CONF(ud,flags) & CN_ISCNTL )
    return 1;
  else
    return 0;

}

