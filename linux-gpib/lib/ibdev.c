/***************************************************************************
                                 ibdev.c
                             -------------------
    begin                : Tues Feb 12 2002
    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ib.h>
#include <ibP.h>

extern int ibfind_called;

int ibdev(int minor, int pad, int sad, int timo, int eot, int eos)
{
	int descriptor;
	char *envptr;

	if(!ibfind_called)
	{		/*if called first time load config*/
		if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 )
		{
			if(ibParseConfigFile(DEFAULT_CONFIG_FILE) < 0  )
			{
				ibsta |= ERR;
				ibPutErrlog(-1,"ibParseConfig");
				return ERR;
			}
		}else
		{
			if(ibParseConfigFile(envptr) < 0)
			{
				ibsta = ERR;
				ibPutErrlog(-1,"ibParseConfig");
				return ERR;
			}
		}
		ibPutMsg("Linux-GPIB-Library Initializing..");

		/*setup board characteristics*/
		if(ibBdChrConfig(ind, ibBoard[minor].base,
			ibBoard[minor].irq,
			ibBoard[minor].dma) & ERR)
		{
			return ERR;
		}


		/* If the device is not a Board(controller) do the initializations
			* automagically
			*/
		if(!(CONF(ind,flags) & CN_ISCNTL) )
		{
			if( ibonl(ind,1) & ERR ) return ERR;

			if( ibeos(ind, ibBoard[CONF(ind,board)].eos
				| (ibBoard[CONF(ind,board)].eosflags << 8)) & ERR) return ERR;

			if( ibtmo(ind, ibBoard[CONF(ind,board)].timeout ) & ERR ) return ERR;

			if( CONF(ind,flags) & CN_AUTOPOLL )
				ibape( ind, 1);   /* set autopoll flag */


			if(  ibBoard[CONF(ind,board)].ifc )
			{
				ibPutMsg("IFC ");
				if ( ibsic(ind) & ERR ) return ERR;
				if ( ibsre(ind,1) & ERR ) return ERR;

				if( CONF(ind,flags) & CN_SDCL )
				{
					ibPutMsg("CLR ");
					if(ibclr(ind) & ERR ) return ERR;
				}
				ibPutMsg("INIT: ");
				if(ibConfigs[ind].init_string !='\0')
				{
					if( ibwrt(ind, ibConfigs[ind].init_string,
						strlen( ibConfigs[ind].init_string)) & ERR ) return ERR;
					ibPutMsg(ibConfigs[ind].init_string);
				}
			}
		}
		ibfind_called++;
	}
	return( ind );

}