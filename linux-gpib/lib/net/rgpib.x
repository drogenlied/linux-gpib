/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *
 *
 *     RPC Protocol definition for remote GPIB Protocol
 * (c) 1995 by C.Schroeter
 *
 *
 *
 ***********************************************************************/

/**********************
 * Using the GPIB library inside the server requires
 * other naming conventions for the server stubs
 * see rgpib_proc.c for functions (Yes i know it's a dirty trick :-) )
 *
 */

#ifdef RPC_SVC
%
%#define  rgpib_null_1          rgpib_svc_null_1
%#define  rgpib_gethandle_1     rgpib_svc_gethandle_1           
%#define  rgpib_dorequest_1     rgpib_svc_dorequest_1           
%
%extern void                *rgpib_svc_null_1();
%extern int                 *rgpib_svc_gethandle_1();
%extern struct gpib_request *rgpib_svc_dorequest_1();
%
%
%
#endif

#ifdef RPC_HDR
%
%#if DEBUG_SERVER
%#define DB(a) a
%#else
%#define DB(a)
%#endif
#endif

/*----------------------------------------------------------------------*/
/* some structures */

struct gpib_request {
	int request;
        int handle;

	int count;
        int arg;
        int ret;
	int ibsta;
        int iberr;
	int ibcnt;
	
	string client<512>;

	string buf<>;
};

/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/

/* the program services description */

program RGPIBPROG {
   version RGPIBVERS {
        /*
         * empty request
         *
         */	

  	 void 
         RGPIB_NULL(void) = 0;

	/*
         * get gpib remote handle (ud of remote device)
         *
         *
         */

         int
         RGPIB_GETHANDLE(gpib_request) = 1;

	/*
         * do gpib_request
         *
         *
         */

	 gpib_request
         RGPIB_DOREQUEST(gpib_request) = 2;

   } = 1;
} = 0x20001234;    /* hope this has not been used by others */



