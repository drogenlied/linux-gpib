
#include <ib.h>
#include <ibP.h>

#define DB(a) a

static ibEventHandler event_handle;

PUBLIC int ibevent(int ud, ibEventHandler handler )
{
int dev;

  DB(printf("handler = 0x%x",handler));
  event_handle = handler;
  if( !fork() ){
  DB(printf("waiting..."));
    if( ibwait(ud, RQS) & ERR ){
      ibPutMsg("Error Waiting for Event on Device %d",CONF(ud,padsad));
      _exit(1);
    } else
    DB(printf("OK calling 0x%x\n", event_handle));
    if( event_handle != NULL )(*event_handle)(ud);
    _exit(0);
  }
  return 1;
}
