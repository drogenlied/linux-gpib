
#include <stdio.h>
#include "ib_internal.h"
#include <ibP.h>

// sets up bus to receive data from device with address pad/sad
int receive_setup( ibConf_t *conf )
{
	ibBoard_t *board;
	uint8_t cmdString[8];
	unsigned int i = 0;
	int pad = conf->pad;
	int sad = conf->sad;

	board = interfaceBoard( conf );

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

ssize_t read_data( ibConf_t *conf, uint8_t *buffer, size_t count )
{
	ibBoard_t *board;
	read_write_ioctl_t read_cmd;
	int retval;

	board = interfaceBoard( conf );

	read_cmd.buffer = buffer;
	read_cmd.count = count;

	set_timeout( board, conf->usec_timeout );

	conf->end = 0;

	retval = ioctl( board->fileno, IBRD, &read_cmd );
	if( retval < 0 )
	{
		switch( errno )
		{
			case ETIMEDOUT:
				conf->timed_out = 1;
				break;
			default:
				setIberr( EDVR );
				break;
		}
		return retval;
	}

	if( read_cmd.end ) conf->end = 1;

	return read_cmd.count;
}

ssize_t my_ibrd( ibConf_t *conf, uint8_t *buffer, size_t count )
{

	// set eos mode
	iblcleos( conf );

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( receive_setup( conf ) < 0 )
		{
			return -1;
		}
	}

	return read_data( conf, buffer, count );
}

int ibrd(int ud, void *rd, long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	count = my_ibrd( conf, rd, cnt );
	if( count < 0 )
	{
		return exit_library( ud, 1 );
	}

	setIbcnt( count );

	return exit_library( ud, 0 );
}

void RcvRespMsg( int boardID, void *buffer, long count, int termination )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	int use_eos;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		exit_library( boardID, 1 );
		return;
	}

	board = interfaceBoard( conf );

	if( board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		exit_library( boardID, 1 );
		return;
	}

	if( termination != ( termination & 0xff ) &&
		termination != STOPend )
	{
		setIberr( EARG );
		exit_library( boardID, 1 );
		return;
	}
	// XXX check for listener active state

	//XXX detect no listeners (EBUS) error
	use_eos = ( termination != STOPend );
	retval = config_read_eos( board, use_eos, termination, 1 );
	if( retval < 0 )
	{
		exit_library( boardID, 1 );
		return;
	}

	retval = read_data( conf, buffer, count );
	if( retval != count )
	{
		exit_library( boardID, 1 );
		return;
	}

	exit_library( boardID, 0 );

}


