
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

ssize_t my_ibwrt( const ibBoard_t *board, const ibConf_t *conf,
	uint8_t *buffer, size_t count )
{
	read_write_ioctl_t write_cmd;

	write_cmd.buffer = buffer;
	write_cmd.count = count;
	write_cmd.end = conf->send_eoi;

	set_timeout( board, conf->usec_timeout );

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

	board = &ibBoard[ conf->board ];

	iblcleos( conf );

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

	count = my_ibwrt( board, conf, rd, cnt );

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




