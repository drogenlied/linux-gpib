
#include "ib_internal.h"
#include <ibP.h>

int ibrpp( int ud, char *ppr )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval;
	int status = ibsta & CMPL;
	uint8_t poll_byte;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EDVR;
		return status;
	}

	board = &ibBoard[ conf->board ];

	set_timeout( board, conf->usec_timeout );

	retval = ioctl( board->fileno, IBRPP, &poll_byte );
	if( retval < 0 )
	{
		status |= ERR;
		ibsta = status;
		return status;
	}

	*ppr = poll_byte;

	ibsta = status;
	return status;
}
