
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

ssize_t __ibwrt(ibBoard_t *board, uint8_t *buffer, size_t count, int send_eoi)
{
	read_write_ioctl_t write_cmd;

	write_cmd.buffer = buffer;
	write_cmd.count = count;
	write_cmd.end = send_eoi;

	if( ioctl( board->fileno, IBWRT, &write_cmd) < 0)
	{
		return -1;
	}
	return write_cmd.count;
}

int ibwrt(int ud, void *rd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;
	int status = ibsta & (RQS | CMPL);
	int retval;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	board = &ibBoard[conf->board];

	iblcleos(ud);
	__ibtmo(board, conf->timeout);

	if(conf->is_interface == 0)
	{
		// set up addressing
		if(send_setup( board, conf ) < 0)
		{
			iberr = EDVR;
			status |= ERR;
			ibsta = status;
			return status;
		}
	}

	count = __ibwrt(board, rd, cnt, conf->send_eoi);

	if(count < 0)
	{
		switch(errno)
		{
			case ETIMEDOUT:
				iberr = EABO;
				status |= TIMO;
				break;
			default:
				break;
		}
		status |= ERR;
		iberr = EDVR;
		ibsta = status;
		return status;
	}

	if(count != cnt)
	{
		iberr = EDVR;
		status |= ERR;
	}

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

	status |= CMPL;
	if(conf->send_eoi)
		status |= END;
	ibcnt = count;
	ibsta = status;

	return status;
}




