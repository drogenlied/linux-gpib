
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>
#include <string.h>

int set_iobase( ibBoard_t *board, ibConf_t *conf )
{
	return ioctl( board->fileno, CFCBASE, &board->base );
}

int ibBdChrConfig(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	board_type_ioctl_t boardtype;
	int status = ibsta & CMPL;
	int retval;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		status |= ERR;
		ibsta = status;
		return status;
	}

	board = &ibBoard[ conf->board ];

	if( board->fileno >= 0 )
	{
		ibsta = status;
		return status;
	}

	if( ibBoardOpen( conf->board, 0 ) & ERR)
	{
		status |= ERR;
		iberr = EDVR;
		ibcnt = errno;
	}else
	{
		strncpy( boardtype.name, board->board_type, sizeof( boardtype.name ) );
		retval = ioctl( board->fileno, CFCBOARDTYPE, &boardtype );
		if( retval < 0 )
			status |= ERR;
		retval = ioctl( board->fileno, CFCBASE, &board->base );
		if( retval < 0 )
			status |= ERR;
		retval = ioctl( board->fileno, CFCIRQ, &board->irq );
		if( retval < 0 )
			status |= ERR;
		retval = ioctl( board->fileno, CFCDMA, &board->dma );
		if( retval < 0 )
			status |= ERR;

		if( !( status & ERR ) )
		{
			iberr = EDVR;
			ibcnt = 0;
		}
		ibBoardClose(conf->board);
	}
	ibsta = status;
	return status;
}

