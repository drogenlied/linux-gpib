
#include "ib_internal.h"
#include <ibP.h>

int ibcmd(int ud, void *cmd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	// check that ud is an interface board
	if(conf->is_interface == 0)
	{
		iberr = EARG;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	__ibtmo(board, conf->tmo);

	count = __ibcmd(board, cmd, cnt);

	if(count != cnt)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	ibcnt = count;
	ibsta &= ~ERR;
	return ibsta;
}

ssize_t __ibcmd(ibBoard_t *board, uint8_t *buffer, size_t count)
{
	read_write_ioctl_t cmd;
	int retval;

	// check that interface board is master
	if(board->ifc == 0)
	{
		return -1;
	}

	cmd.buffer = buffer;
	cmd.count = count;

	retval = ioctl(board->fileno, IBCMD, &cmd);
	if(retval < 0)
	{
		return -1;
	}

	return cmd.count;
}
