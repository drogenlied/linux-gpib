
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

int ibtmo(int ud,int v)
{
	ibConf_t *conf = ibConfigs[ud];

	if ((v < TNONE) || (v > T1000s) || ibCheckDescriptor(ud) < 0)
	{
		ibsta = CMPL | ERR;
		iberr = EARG;
		return ibsta;
	}

	conf->timeout = v;

	ibsta = CMPL;
	return ibsta;
}

int __ibtmo(ibBoard_t *board, int timeout)
{
	ioctl(board->fileno, IBTMO, timeout);

	return 0;
}


