
#include <ib.h>
#include <ibP.h>
#include <string.h>
#include <stdlib.h>

char ibfind_called = 0;       /* first call = setup board */


PUBLIC int ibfind(char *dev)
{

int ind;
char *envptr;
char *s;

char server[60];
char *device;

#if HAS_RGPIB

   if(( s=(char *)strchr(dev,':')) != (char *)NULL ){
        ibPutErrlog(-1,"connect server");

        
        strncpy(server,dev,s-dev);
	server[s-dev]=0;
        device = &s[1];

	return (ibFindRemote( server , device ));
   }

#endif

   if(!ibfind_called){		/*if called first time load config*/
     if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 ){
       if(ibParseConfigFile(DEFAULT_CONFIG_FILE) < 0  ) {
         ibsta |= ERR;
         ibPutErrlog(-1,"ibParseConfig");
	 return ERR;
       }
     }
     else{
       if(ibParseConfigFile(envptr) < 0) {
         ibsta = ERR;
         ibPutErrlog(-1,"ibParseConfig");
	 return ERR;
       }
     }
   }

   if((ind=ibFindDevIndex(dev)) < 0 ){     /* find desired entry */
     iberr = ENSD;
     ibsta = ERR;
     ibPutErrlog(-1,"ibFindDevIndex");
     return ERR;
   }

   if(!ibfind_called){

       ibOpenErrlog( ibBoard[CONF(ind,board)].errlog );
       ibPutMsg("Linux-GPIB-Library Initializing..");

       /*setup board characteristics*/

       if( ibBdChrConfig( ind, ibBoard[CONF(ind,board)].base, 
		          ibBoard[CONF(ind,board)].irq, 
                          ibBoard[CONF(ind,board)].dma,ibBoard[CONF(ind,board)].dmabuf  ) & ERR ) {
	 return ERR;
       }


       /* If the device is not a Board(controller) do the initializations 
        * automagically
        */

       if( ! (CONF(ind,flags) & CN_ISCNTL) ){

	 if( ibonl(ind,1) & ERR ) return ERR;


	 if( ibeos(ind, ibBoard[CONF(ind,board)].eos 
		   | (ibBoard[CONF(ind,board)].eosflags << 8)) & ERR) return ERR;

	 if( ibtmo(ind, ibBoard[CONF(ind,board)].timeout ) & ERR ) return ERR;




         if( CONF(ind,flags) & CN_AUTOPOLL )
	   ibape( ind, 1);   /* set autopoll flag */


	 if(  ibBoard[CONF(ind,board)].ifc ){
	   ibPutMsg("IFC ");
	   if ( ibsic(ind) & ERR ) return ERR;
	   if ( ibsre(ind,1) & ERR ) return ERR;

	   if( CONF(ind,flags) & CN_SDCL ) {

	     ibPutMsg("CLR ");
	     if(ibclr(ind) & ERR ) return ERR;
	   }

	   ibPutMsg("INIT: ");
	   if( ibConfigs[ind].init_string !='\0' ){
	     if( ibwrt(ind, ibConfigs[ind].init_string,
		       strlen( ibConfigs[ind].init_string)) & ERR ) return ERR;
	     ibPutMsg(ibConfigs[ind].init_string);

	   }

	 }


       }

       if(ibBoard[CONF(ind,board)].debug) 
	 ibSdbg(ind,ibBoard[CONF(ind,board)].debug);

       ibfind_called++;
     
     }
   
   return( ind );

}











