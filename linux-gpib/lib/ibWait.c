
#include "ib_internal.h"
#include <ibP.h>

int ibwait(int ud, int mask)
{
	char spr;
	int pollflag = 0;
	int amask;
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	if( conf->flags & CN_AUTOPOLL ) pollflag = 1;

	// XXX this if should depend on whether ud is a board or device descriptor
	if( mask & RQS )
	{
		amask = ( mask | SRQI ) & ~RQS ;

		while(1)
		{
			/* wait for SRQ */
			if(ibBoardFunc(conf->board, pollflag ? IBAPWAIT : IBWAIT,
				pollflag ? padsad(conf->pad, conf->sad) : amask) & ( ERR | TIMO ) )
				return ERR;
			/* Serial Poll Device */
			if( ibBoardFunc( conf->board, pollflag ? IBAPRSP : DVRSP,
				padsad(conf->pad, conf->sad), &spr ) & ( ERR | TIMO ))
        		return ERR;
	/* if RQS set return */
			if ( spr & 0x40 ) return spr;
		}
	}
	/*pollflag will not necessary be*/
	/*taken from remote*/
	else
		return ibBoardFunc(conf->board, IBWAIT, mask);
}
