#include <ib.h>
#include <ibP.h>

#include <stdio.h>
#include <rpc/rpc.h>
#include <getopt.h>

#include "rgpib.h"

#define SUBVERSION 0

#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "/etc/gpib.conf"
#endif

extern void rgpibprog_1();
void gpib_server();

static int gpib_bus = NULL ;  /* busmaster descriptor */


/***********************************************************************
 * 
 *
 * This is a ugly trick to get the auth handle: wrapping the remote calls
 **********************************************************************/
void
rgpibprog_wrap_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
struct authunix_parms *unix_cred;

if( rqstp->rq_proc == RGPIB_GETHANDLE ){
  switch (rqstp->rq_cred.oa_flavor) {
  case AUTH_UNIX:
    unix_cred = (struct authunix_parms *) rqstp->rq_clntcred;
    DB(fprintf(stderr,"machname = '%s'\n",unix_cred->aup_machname));
    ibPutMsg("Remote Handle Request From <%s> (uid=%d,gid=%d)",unix_cred->aup_machname,
                                                               unix_cred->aup_uid,
	                                                       unix_cred->aup_gid);
    /* lookup machname in the local configuration */

    if(! ibCheckAuth(gpib_bus,unix_cred->aup_machname ) ){
      DB(fprintf(stderr,"Host '%s' is not Permitted to Requesting Server\n",
                      unix_cred->aup_machname));
      ibPutMsg("Host <%s> is not Permitted to Requesting Server\n",
                      unix_cred->aup_machname);
      svcerr_weakauth(transp);
      return;
    } 
    break;
  default:
    DB(fprintf(stderr,"No authentication Packets sent, Request rejected\n"));
    ibPutMsg("Illegal Request from <%s>, Rejected",unix_cred->aup_machname);
    svcerr_weakauth(transp);
    return;
  }
}  
/* call the handler function */  
rgpibprog_1(rqstp, transp);

}


usage() {
  printf("Remote GPIB server (c)1994,1995,1996,1997 C.Schroeter \n\
  Usage:\n\
  \t --debug, -d  Switch Debugging on \n\
  \t --help , -?  Display this Message \n\n");
  exit(1);

}


/***********************************************************************
 * 
 *  the main routine
 * 
 *
 ***********************************************************************/
main(int argc, char **argv)
{

        char *envptr;
        int no=0;
        char tmp[80];
        char hostname[64];


         int c;
         int digit_optind = 0;

/* parse options */

         while (1)
           {
             int this_option_optind = optind ? optind : 1;
             int option_index = 0;
             static struct option long_options[] =
             {
            {"debug", 0, 0, 0},
            {"help",  0, 0, 0},
            {0, 0, 0, 0}
             };

             c = getopt_long (argc, argv, "d",
                        long_options, &option_index);
             if (c == -1)
            break;

             switch (c)
            {
            case 0:
	    case 'd':
	      debug = 1;
	      break;
	    case 1:
	    case '?':
	      usage();
	      break;
            default:
              printf ("?? getopt returned character code 0%o \n", c);
            }
           }




        /* first parse configuration */

        if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 ){
	  if(ibParseConfigFile(DEFAULT_CONFIG_FILE) < 0  ) {
	    fprintf(stderr,"Can't open Config File %s\n",DEFAULT_CONFIG_FILE);
	    exit (1);
	  }
	}
        else{
	  if(ibParseConfigFile(envptr) < 0) {
	    fprintf(stderr,"Can't open Config File %s\n",envptr);
	    exit (1);
	  }
	}
      
        if((gpib_bus=ibFindDevIndex("gpib0")) < 0 ){     /* find desired entry */
	    fprintf(stderr,"Can't find busmaster entry \n");
	    exit (1);
        }

        ibOpenErrlog( ibBoard[CONF(gpib_bus,board)].errlog );

	ibPutMsg("Remote-GPIB Server, Protocol Version %d.%d (c)1995 C.Schroeter\n",
		RGPIBVERS , SUBVERSION );

        /* now start server and wait for requests */

	gpib_server();  /* start server */

	fprintf(stderr, "Fatal Error: gpib_server returned\n");
	exit(1);
}

/***********************************************************************
 *  the server routine
 *  never returns
 * 
 *
 ***********************************************************************/
void gpib_server (void) {

	SVCXPRT *transp;

        /*
         * Initiate RPC Protocol stuff
         *
         */

	(void)pmap_unset(RGPIBPROG, RGPIBVERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, RGPIBPROG, RGPIBVERS, rgpibprog_wrap_1, IPPROTO_UDP)) {
		(void)fprintf(stderr, "unable to register (RGPIBPROG, RGPIBVERS, udp).\n");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create tcp service.\n");
		exit(1);
	}
	if (!svc_register(transp, RGPIBPROG, RGPIBVERS, rgpibprog_wrap_1, IPPROTO_TCP)) {
		(void)fprintf(stderr, "unable to register (RGPIBPROG, RGPIBVERS, tcp).\n");
		exit(1);
	}

	
	/*
         *  run server
         *  svc_run() never returns !
         *
         */

	svc_run();
}

