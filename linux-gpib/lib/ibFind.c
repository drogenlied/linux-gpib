
#include <ib.h>
#include <ibP.h>
#include <string.h>
#include <stdlib.h>

int ibfind_called = 0;       /* first call = setup board */

int ibfind(char *dev)
{

	int ind;
	char *envptr;

	if(!ibfind_called)
	{		/*if called first time load config*/
		if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 )
		{
			if(ibParseConfigFile(DEFAULT_CONFIG_FILE) < 0  )
			{
				ibsta |= ERR;
				ibPutErrlog(-1,"ibParseConfig");
				return -1;
			}
		}else
		{
			if(ibParseConfigFile(envptr) < 0)
			{
				ibsta = ERR;
				ibPutErrlog(-1,"ibParseConfig");
				return -1;
			}
		}
	}

	if((ind=ibFindDevIndex(dev)) < 0 )
	{     /* find desired entry */
		iberr = ENSD;
		ibsta = ERR;
		ibPutErrlog(-1,"ibFindDevIndex");
		return -1;
	}

	if(!ibfind_called)
	{
		ibPutMsg("Linux-GPIB-Library Initializing..");

		/*setup board characteristics*/
		if(ibBdChrConfig( ind, ibBoard[CONF(ind,board)].base,
			ibBoard[CONF(ind,board)].irq,
			ibBoard[CONF(ind,board)].dma) & ERR )
		{
			return -1;
		}


		/* If the device is not a Board(controller) do the initializations
			* automagically
			*/
		if(!(CONF(ind,flags) & CN_ISCNTL) )
		{
			if( ibonl(ind,1) & ERR ) return -1;

			if( ibeos(ind, ibBoard[CONF(ind,board)].eos
				| (ibBoard[CONF(ind,board)].eosflags << 8)) & ERR) return -1;

			if( ibtmo(ind, ibBoard[CONF(ind,board)].timeout ) & ERR ) return -1;

			if( CONF(ind,flags) & CN_AUTOPOLL )
				ibape( ind, 1);   /* set autopoll flag */


			if(  ibBoard[CONF(ind,board)].ifc )
			{
				ibPutMsg("IFC ");
				if ( ibsic(ind) & ERR ) return -1;
				if ( ibsre(ind,1) & ERR ) return -1;

				if( CONF(ind,flags) & CN_SDCL )
				{
					ibPutMsg("CLR ");
					if(ibclr(ind) & ERR ) return -1;
				}
				ibPutMsg("INIT: ");
				if(ibConfigs[ind].init_string !='\0')
				{
					if( ibwrt(ind, ibConfigs[ind].init_string,
						strlen( ibConfigs[ind].init_string)) & ERR ) return -1;
					ibPutMsg(ibConfigs[ind].init_string);
				}
			}
		}
		ibfind_called = 1;
	}
	return( ind );
}











