#include <rpc/rpc.h>
#include "rgpib.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

void *
rgpib_null_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, RGPIB_NULL, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


int *
rgpib_gethandle_1(argp, clnt)
	gpib_request *argp;
	CLIENT *clnt;
{
	static int res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, RGPIB_GETHANDLE, xdr_gpib_request, argp, xdr_int, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


gpib_request *
rgpib_dorequest_1(argp, clnt)
	gpib_request *argp;
	CLIENT *clnt;
{
	static gpib_request res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, RGPIB_DOREQUEST, xdr_gpib_request, argp, xdr_gpib_request, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

