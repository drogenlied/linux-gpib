
#include <ib.h>
#include <ibP.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include "rgpib.h"
#include "rgpibP.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

/* 
 * Network handles are integers with UD_REMOTE bit set
 *
 +
 +
 */

#if DEBUG_SERVER
int debug = 0;
#endif


CLIENT *ibGetClient(int handle);

r_handle_t RgpibHandles[MAXHANDLES];
int Rhandle_cnt = 0;

/*----------------------------------------------------------------------*/
PRIVATE int ibRemoteFunc (int handle, int code, int arg, char *buf, int cnt)
{
  gpib_request request, *result;
  CLIENT *cl;

  init_gpib_request(&request);
  
  request.handle  = ibGetUd(handle);
  request.request = code;
  request.arg     = arg;
  request.ibcnt   = cnt;
  request.count   = cnt;
  if (buf!=NULL)
    request.buf   = buf;

  result = rgpib_dorequest_1 ( &request, ibGetClient(handle) );
  if( result == NULL ) {
    ibsta |= CMPL | ERR;
    iberr = ENWE;
    ibcnt = errno;
    ibPutMsg("Network Error during request %d (%s) ",code,ibVerbCode(code));
  } else {

    ibsta = result->ibsta;
    iberr = result->iberr;
    ibcnt = result->ibcnt;

    if( result->ibsta & ERR ){
      ibPutMsg("Network Request Error: '%s' resulted with an error ",ibVerbCode(code));
    } 
    else switch (code) {
    case IBRD:
    case DVRD:
      memset(buf,0,cnt);
    case IBAPRSP:
    case IBRPP:
    case DVRSP:
      memcpy(buf,result->buf,ibcnt);
      break;
    }

  }
  return ibsta;
}


static char  hostdomain[MAXNETNAMELEN], 
  cmpserver[MAXNETNAMELEN], cmphost[MAXNETNAMELEN];
static int   ibfind_remote_called = 0;

/*----------------------------------------------------------------------*/
PRIVATE int ibFindRemote ( char *server, char *device )
{
int handle,*ud;

static struct gpib_request req;
static CLIENT *cl;
struct hostent *hostent;
struct in_addr hostaddr;
char hostip[255], serverip[255];
int i;

if ( ! ibfind_remote_called ){
/* get host.domain */
#if 1
   gethostname(hostdomain,sizeof(hostdomain));
   i = strlen(hostdomain);
   hostdomain[i++] = '.';
   getdomainname(&hostdomain[i],(sizeof(hostdomain)-i));
#else
   getnetname(hostdomain);
#endif

   strcpy(hostip,"127.0.0.1");
   strcpy(serverip,"");
   if (((hostent=gethostbyname(hostdomain))!=NULL)
       && (hostent->h_length==sizeof(unsigned long))) {
     hostaddr.s_addr=*((unsigned long *)(hostent->h_addr));
     strcpy(hostip,(char *)inet_ntoa(hostaddr));
   }
   if (((hostent=gethostbyname(server))!=NULL)
       && (hostent->h_length==sizeof(unsigned long))) {
     hostaddr.s_addr=*((unsigned long *)(hostent->h_addr));
     strcpy(serverip,(char *)inet_ntoa(hostaddr));
   }

#ifdef IGNORE_LOCAL
   if ((strcmp(hostip,serverip)==0) || (strcmp("127.0.0.1",serverip)==0)) {
     DB(printf("'%s' found at localhost ip '%s'.\n",device,serverip));
     return(ibfind(device));
   }
#endif
   DB(printf("host: '%s'\nserver: '%s'\n",hostdomain,server));
   ibfind_remote_called++;
 }
/* create client stub and create new handle */ 

   if(( handle = ibCreateClientStub(server)) & ERR )  return ERR;

/* remote call gethandle */

   init_gpib_request(&req);

   req.buf = device;


/* set auth style */
   cl =  ibGetClient(handle);
   cl->cl_auth = authunix_create(hostdomain,getuid(),getgid(),0,NULL );

   ud = rgpib_gethandle_1( &req , cl );

   /*auth_destroy(cl->cl_auth);*/

   if( ud == NULL ) {
     ibsta |= CMPL | ERR;
     iberr = ENWE;
     ibcnt = errno;
     return ERR;
   } 

   if( *ud & ERR ){
     ibsta |= CMPL | ERR ;
     iberr = ENWE;
     ibcnt = errno;
     return ERR;
   }

   /* set remote ud */
   RgpibHandles[(handle & ~UD_REMOTE)].r_ud = *ud;


   return (handle | UD_REMOTE);
}

/*----------------------------------------------------------------------*/
CLIENT *ibGetClient(int handle){
   return ( RgpibHandles[(handle & ~UD_REMOTE)].r_client );
}

PRIVATE int ibGetUd(int handle)
{
    return ( RgpibHandles[(handle & ~UD_REMOTE)].r_ud);
}
/*----------------------------------------------------------------------*/
PRIVATE int ibCreateClientStub(char *server)
{

CLIENT *cl;

/* create a client */
cl = clnt_create(server,RGPIBPROG,RGPIBVERS,"tcp");
if ( cl == NULL ){

/* set error code */
iberr = ENWE;
ibPutMsg("Network Error: Cant Request Server '%s'",server);

return ERR;
}

if( Rhandle_cnt < MAXHANDLES ){
  /* add the new client to the client table */
  RgpibHandles[Rhandle_cnt].r_client = cl;

  Rhandle_cnt++;
  return ( (Rhandle_cnt-1) | UD_REMOTE);
} else {
  iberr = ENTF;
  ibPutMsg("Network Error: No more Handles !");
  return ERR;
}
}
/*----------------------------------------------------------------------*/
char *ibGetNetDB(int ud)
{
  if( ud & UD_REMOTE ){ 
    return NULL;
  }
  return ibConfigs[ud].networkdb;

}
/*----------------------------------------------------------------------*/

void ibCleanupRemote(){
  CLIENT *cl;
  int i;
  for( i=0; i< Rhandle_cnt; i++)
    clnt_destroy(ibGetClient(i));
  Rhandle_cnt = 0;
  ibfind_remote_called = 0;
}

