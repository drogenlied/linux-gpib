
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

int ibeot(int ud, int send_eoi)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	int status = ibsta & (CMPL | RQS);
	int retval;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	if(send_eoi)
		conf->send_eoi = 1;
	else
		conf->send_eoi = 0;

	board = &ibBoard[conf->board];

	// get more status bits from interface board if appropriate
	if(conf->is_interface)
	{
		int board_status;
		retval = ioctl(board->fileno, IBSTATUS, &board_status);
		if(retval < 0)
		{
			status |= ERR;
			iberr = EDVR;
			ibsta = status;
			return status;
		}
		status |= board_status & DRIVERBITS;
	}

	return status;
}
