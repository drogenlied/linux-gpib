
#include <ib.h>
#include <ibP.h>
#include <rpc/rpc.h>

#include "rgpib.h"

/*----------------------------------------------------------------------*/
int auth_unix(rqstp,transp)
     struct svc_req *rqstp;
     SVCXPRT *transp;
{

return 0;
}


/***********************************************************************
 *  ibCheckAuth  looks up the host in the given device ud struct
 *
 * 
 *
 ***********************************************************************/
PRIVATE int ibCheckAuth(int ud,char *client)
{
  int i,retval;
  char *hlist;
  char *host;
  char *match;

  if( ibConfigs[ud].networkdb == NULL ){  
    return 0;
  } else
    hlist = ibConfigs[ud].networkdb;

  retval=0;
  while((host = strtok(hlist,":")) != NULL ) {
    hlist = NULL;

    if((match=strchr(host,'*')) == NULL){ 
      /* direct comparison host if*/
      if( strcmp(host,client) == 0 ) return 1;
    } else {
      /* regexp host */
      if( do_match(host,client)  ) return 1;
    }
  }
  return 0;
}
