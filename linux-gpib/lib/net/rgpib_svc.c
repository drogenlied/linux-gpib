#include <stdio.h>
#include <rpc/rpc.h>
#include "rgpib.h"

#define  rgpib_null_1          rgpib_svc_null_1
#define  rgpib_gethandle_1     rgpib_svc_gethandle_1           
#define  rgpib_dorequest_1     rgpib_svc_dorequest_1           

extern void                *rgpib_svc_null_1();
extern int                 *rgpib_svc_gethandle_1();
extern struct gpib_request *rgpib_svc_dorequest_1();




void
rgpibprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		gpib_request rgpib_gethandle_1_arg;
		gpib_request rgpib_dorequest_1_arg;
	} argument;
	char *result;
	char client[512];
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();
	argument.rgpib_gethandle_1_arg.client=client;
	strcpy(client,"a");

	switch (rqstp->rq_proc) {
	case RGPIB_NULL:
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) rgpib_null_1;
		break;

	case RGPIB_GETHANDLE:
		xdr_argument = xdr_gpib_request;
		xdr_result = xdr_int;
		local = (char *(*)()) rgpib_gethandle_1;
		break;

	case RGPIB_DOREQUEST:
		xdr_argument = xdr_gpib_request;
		xdr_result = xdr_gpib_request;
		local = (char *(*)()) rgpib_dorequest_1;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero((char *)&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}







