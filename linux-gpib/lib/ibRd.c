
#include <stdio.h>
#include "ib_internal.h"
#include <ibP.h>

// sets up bus to receive data from device with address pad/sad
int receive_setup( ibBoard_t *board, ibConf_t *conf )
{
	uint8_t cmdString[8];
	unsigned int i = 0;
	int pad = conf->pad;
	int sad = conf->sad;

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

	if ( my_ibcmd( conf, cmdString, i) < 0)
	{
		fprintf(stderr, "receive_setup: command failed\n");
		return -1;
	}

	return 0;
}

ssize_t my_ibrd( ibBoard_t *board, ibConf_t *conf, uint8_t *buffer, size_t count,
	int *end )
{
	read_write_ioctl_t read_cmd;
	int retval;

	// set eos mode
	iblcleos( conf );

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( receive_setup( board, conf ) < 0 )
		{
			return -1;
		}
	}

	read_cmd.buffer = buffer;
	read_cmd.count = count;

	set_timeout( board, conf->usec_timeout );

	retval = ioctl( board->fileno, IBRD, &read_cmd );
	if( retval < 0 )
	{
		return retval;
	}

	*end = read_cmd.end;

	return read_cmd.count;
}

int ibrd(int ud, void *rd, long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;
	int end;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->end = 0;

	board = interfaceBoard( conf );

	count = my_ibrd( board, conf, rd, cnt, &end );
	if( count < 0 )
	{
		return exit_library( ud, 1 );
	}

	setIbcnt( count );

	if( end ) conf->end = 1;

	return exit_library( ud, 0 );
}




