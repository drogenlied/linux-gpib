
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

ssize_t __ibwrt(ibBoard_t *board, uint8_t *buffer, size_t count)
{
	read_write_ioctl_t write_cmd;

	write_cmd.buffer = buffer;
	write_cmd.count = count;

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

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	iblcleos(ud);
	__ibtmo(board, conf->tmo);

	if(conf->is_interface == 0)
	{
		// set up addressing
		send_setup(board, conf->pad, conf->sad);
	}

	count = __ibwrt(board, rd, cnt);

	if(count != cnt)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	ibcnt = count;

	return ibsta;
}




