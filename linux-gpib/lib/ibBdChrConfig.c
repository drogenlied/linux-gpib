
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>
#include <string.h>

int ibBdChrConfig(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	board_type_ioctl_t boardtype;

	if(ibCheckDescriptor(ud) < 0)
	{
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	if(board->fileno >= 0) return ibsta;

	if(ibBoardOpen(conf->board, 0) & ERR)
	{
		ibsta = ibarg.ib_ibsta | ERR;
		iberr = EDVR;
		ibcnt = errno;
		ibPutErrlog(ud,"ibBdChrConfig");
	}else 
	{
		strncpy(boardtype.name, board->board_type, sizeof(boardtype.name));
		ioctl(board->fileno, CFCBOARDTYPE, &boardtype); 
		ibBoardFunc(conf->board, CFCBASE, board->base);
		ibBoardFunc(conf->board, CFCIRQ , board->irq);
		ibBoardFunc(conf->board, CFCDMA , board->dma);

		if(!(ibsta & ERR)) 
		{
			iberr = EDVR;
			ibcnt = 0;
			ibPutErrlog(ud,"ibBdChrConfig");
		}
		ibBoardClose(conf->board);
	}
	return ibsta;
}

