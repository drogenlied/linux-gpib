
#include <ib.h>
#include <ibP.h>

PUBLIC int ibrsp(int ud, char *spr)
{

return  ibBoardFunc( CONF(ud,board),
                     ((CONF(ud,flags)&CN_AUTOPOLL) ? IBAPRSP : DVRSP),
                     CONF(ud,padsad), spr);

}
