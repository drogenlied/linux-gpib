
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

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

int send_setup(ibBoard_t *board, int pad, int sad)
{
	uint8_t cmdString[8];
	unsigned i = 0;

	if( pad < 0 || pad > gpib_addr_max || sad > gpib_addr_max)
	{
		fprintf(stderr, "gpib: bad addr\n");
		return -1;
	}

	cmdString[ i++ ] = UNL;
	cmdString[ i++ ] = MLA( pad );
	if( sad >= 0)
		cmdString[ i++ ] = MSA( sad );
	/* controller's talk address */
	cmdString[ i++ ] = MTA( board->pad );
	if( board->sad >= 0 )
		cmdString[ i++ ] = MSA( board->sad );

	if( __ibcmd( board, cmdString, i) < 0 )
		return -1;

	return 0;
}
