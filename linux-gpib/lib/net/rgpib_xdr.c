#include <rpc/rpc.h>
#include <gpib_ioctl.h>
#include "rgpib.h"

#define BUFSIZE 0xFFFF
#define CLIENTSIZE 512

void
init_gpib_request(req)
     gpib_request *req;
{
  static char tmpbuf[BUFSIZE], tmpclient[CLIENTSIZE];

  tmpbuf[0]=0;
  tmpclient[0]=0;
  
  req->buf=tmpbuf;
  req->client=tmpclient;
  req->request=CFCIO;
}

bool_t
xdr_gpib_request(xdrs, objp)
	XDR *xdrs;
	gpib_request *objp;
{
        unsigned int sizep;

	if (!xdr_int(xdrs, &objp->request)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->handle)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->count)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->arg)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->ret)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->ibsta)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->iberr)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->ibcnt)) {
		return (FALSE);
	}
	if (!xdr_string(xdrs, &objp->client, CLIENTSIZE)) {
	  return (FALSE);
	}
        switch(objp->request) {
	case IBRD:
	case DVRD:
	case IBAPRSP:
	case IBRPP:
	case DVRSP:
          sizep=objp->ibcnt;
	  xdr_bytes(xdrs, &objp->buf, &sizep, BUFSIZE);
	  break;
	default:
	  if (!xdr_string(xdrs, &objp->buf, BUFSIZE)) {
	    return (FALSE);
	  }
	  break;
	}
	return (TRUE);
}
