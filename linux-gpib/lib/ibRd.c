
#include <stdio.h>
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

// sets up bus to receive data from device with address pad/sad
int receive_setup(ibBoard_t *board, int pad, int sad)
{
	uint8_t cmdString[8];
	unsigned int i = 0;

	if( pad < 0 || pad > gpib_addr_max || sad > gpib_addr_max)
	{
		fprintf(stderr, "receive_setup: bad gpib address\n");
		return -1;
	}

	cmdString[i++] = UNL;

	cmdString[i++] = MLA(board->pad);	/* controller's listen address */
	if ( board->sad >= 0 )
		cmdString[i++] = MSA(board->sad);
	cmdString[i++] = MTA(pad);
	if(sad >= 0)
		cmdString[i++] = MSA(sad);

	if ( __ibcmd(board, cmdString, i) < 0)
		return -1;

	return 0;
}

int ibrd(int ud, void *rd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	read_write_ioctl_t read_cmd;
	int retval;
	int status = ibsta & (RQS | CMPL);

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	board = &ibBoard[conf->board];

	// set eos mode
	iblcleos(ud);
	// set timeout XXX need to init conf with board default when not set
	__ibtmo(board, conf->tmo);

	if(conf->is_interface == 0)
	{
		// set up addressing
		if(receive_setup(board, conf->pad, conf->sad) < 0)
		{
			iberr = EDVR;
			status |= ERR;
			ibsta = status;
			return status;
		}
	}

	read_cmd.buffer = rd;
	read_cmd.count = cnt;

	retval = ioctl(board->fileno, IBRD, &read_cmd);
	if(retval < 0)
	{
		switch(errno)
		{
			case ETIMEDOUT:
				status |= TIMO;
				iberr = EABO;
				break;
			default:
				iberr = EDVR;
				break;
		}
		status |= ERR;
		ibsta = status;
		return status;
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

	ibcnt = read_cmd.count;
	status |= CMPL;
	if(read_cmd.end)
		status |= END;
	ibsta = status;

	return status;
}




