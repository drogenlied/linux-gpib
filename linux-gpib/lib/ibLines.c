
#include <ib.h>
#include <ibP.h>

int iblines(int ud, unsigned short *buf)
{

  ibBoardFunc(CONF(ud,board), IBLINES, 0 );
  if( ibsta & ERR )
    *buf = 0;
  else
    *buf = ibarg.ib_ret;

  return ibsta;

}
