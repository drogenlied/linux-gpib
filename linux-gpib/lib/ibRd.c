
#include <stdio.h>
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

// sets up bus to receive data from device with address pad/sad
int receive_setup(ibBoard_t *board, int pad, int sad)
{
	uint8_t cmdString[8];
	unsigned int i = 0;

	if( pad > gpib_addr_max || sad > gpib_addr_max)
	{
		fprintf(stderr, "bad gpib address\n");
		return -1;
	}

	cmdString[i++] = UNL;

	cmdString[i++] = MLA(board->pad);	/* controller's listen address */
	if ( board->sad >= 0 )
		cmdString[i++] = MSA(board->sad);
	cmdString[i++] = MTA(pad);
	if (sad)
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

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	// set eos mode
	iblcleos(ud);
	// set timeout XXX need to init conf with board default when not set
	ibBoardFunc(conf->board, IBTMO, conf->tmo);

	if(conf->is_interface == 0)
	{
		// set up addressing
		receive_setup(board, conf->pad, conf->sad);
	}

	read_cmd.buffer = rd;
	read_cmd.count = cnt;

	retval = ioctl(ibBoard[conf->board].fileno, IBRD, &read_cmd);
	if(retval < 0)
	{
		switch(errno)
		{
			case ETIMEDOUT:
				iberr = EABO;
				break;
			default:
				iberr = ENEB;
				break;
		}
		return ibsta | ERR;
	}

	return ibsta;
}




