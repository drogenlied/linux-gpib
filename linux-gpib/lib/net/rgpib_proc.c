/*
 *  
 *
 *
 *
 *
 */
#include <ib.h>

#include <rpc/rpc.h>
#include "rgpib.h"

/*----------------------------------------------------------------------*/
void *
rgpib_svc_null_1 (void){

DB(printf("nullproc called\n"));


}

/*----------------------------------------------------------------------*/
int *
rgpib_svc_gethandle_1 (req)
struct gpib_request *req;
{

static int result;
int r;


DB(printf("gethandle '%s' called\n",req->buf ));



if( (result = ibfind(req->buf) ) & ERR) {
  result = ERR;
} else {
  ibPutMsg("Remote Request for Device '%s' handle 0x%x\n",req->buf,result);


}


return &result;
}

/*----------------------------------------------------------------------*/

struct gpib_request *
rgpib_svc_dorequest_1 (req)
struct gpib_request *req;
{
static struct gpib_request result;
static char *tmpbuf = NULL;
static char *tmpclient = NULL;

static char errmsg[80];
static char buf[512], client[512];

int r;

DB(printf("request was %d(%s) -> UD: %d\n",req->request,ibVerbCode(req->request),req->handle));

init_gpib_request(&result);
result.request=req->request;

switch(req->request) {

 case IBRD:
 case DVRD:
    DB(printf("ibrd: %d \n",req->count));
    /*
     * Free previous result
     *
     */
    DB(printf("Freeing old buffer @ 0x%x \n",tmpbuf));
    if (tmpbuf) free(tmpbuf);
     /*xdr_free(xdr_gpib_request, &result );*/
    /*
     * get the memory for the XDR result string
     */
    
    if(( tmpbuf = calloc( 1, req->count * sizeof(char) + 1 )) == NULL){
      ibsta |= ERR;
      ibPutMsg("Network Request Error: No more Memory !");
    } else {
      DB(printf("Got %d bytes @ 0x%x for Read Buffer\n",req->count+1,result.buf));
      DB(printf("ibrd(req->handle=%i,tmpbuf=0x%x,req->count=%i)\n",
		req->handle,tmpbuf,req->count));
      if (( r = ibrd(req->handle,tmpbuf,req->count)) & ERR )
	ibsta |= ERR;
      else
	result.buf = tmpbuf;
      DB(printf("Read all %i bytes\n", ibcnt));
      if (ibsta&ERR)
	DB(printf("with errors\n"));
    }
    break;
 case IBWRT:
 case DVWRT:
    DB(printf("ibwrt: %s \n",req->buf));
    if (( r = ibwrt(req->handle,req->buf,req->count)) & ERR )
	ibsta |= ERR;
    break;
 case IBCMD:
    DB(printf("ibcmd: %s \n",req->buf));
    if (( r = ibcmd( req->handle ,req->buf, req->count)) & ERR )
	ibsta |= ERR;
    break;
    break;
 case IBWAIT:
    DB(printf("ibwait: 0x%x \n",req->arg));
    if (( r = ibwait(req->handle,req->arg)) & ERR )
	ibsta |= ERR;
    break;

 case IBONL:
    DB(printf("ibonl: 0x%x \n",req->arg));
#if 0
    if (( r = ibonl(req->handle,req->arg)) & ERR )
	ibsta |= ERR;
#endif
    break;

 case IBSIC:
    DB(printf("ibsic: \n"));
    if (( r = ibsic(req->handle)) & ERR )
	ibsta |= ERR;
    break;

 case IBSRE:
    DB(printf("ibsre: %d \n",req->arg ));
    if (( r = ibsre(req->handle,req->arg)) & ERR )
	ibsta |= ERR;
    break;

 case IBGTS:
    DB(printf("ibgts: %d\n",req->arg));
    if (( r = ibgts(req->handle,req->arg)) & ERR )
	ibsta |= ERR;
    break;

 case IBCAC:
    DB(printf("ibcac: %d\n",req->arg));
    if (( r = ibcac(req->handle,req->arg)) & ERR )
	ibsta |= ERR;
    break;

 
 case DVTRG:
    DB(printf("ibtrg \n"));
    if(( r = ibtrg( req->handle )) & ERR )
      ibsta |= ERR;
    break;

 case DVCLR:
    DB(printf("ibclr %d\n",req->handle));
    if(( r = ibclr( req->handle )) & ERR )
      ibsta |= ERR;
    break;

 case DVRSP:
    DB(printf("ibrsp \n"));
    if(( r = ibrsp( req->handle, result.buf )) & ERR )
      ibsta |= ERR;
    break;


 case IBRPP:
 case IBTMO:
 case IBEOS:
  default:
    result.ibsta = CMPL | ERR;
    result.iberr = ENWE;
    strcpy(errmsg,"Request Not Implemented");
    result.buf = errmsg;
    break;

      }

result.ibsta = ibsta;
result.iberr = iberr;
result.ibcnt = ibcnt;



return &result;
}
