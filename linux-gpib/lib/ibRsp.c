
#include "ib_internal.h"
#include <ibP.h>

static int serial_poll( ibBoard_t *board, unsigned int pad, int sad,
	unsigned int usec_timeout, char *result )
{
	serial_poll_ioctl_t poll_cmd;
	int retval;

	poll_cmd.pad = pad;
	poll_cmd.sad = sad;

	set_timeout( board, usec_timeout );

	retval = ioctl( board->fileno, IBRSP, &poll_cmd );
	if(retval < 0)
	{
		switch( errno )
		{
			case ETIMEDOUT:
				setIberr( EABO );
				break;
			default:
				setIberr( EDVR );
				break;
		}
		return -1;
	}

	*result = poll_cmd.status_byte;

	return 0;
}

int ibrsp(int ud, char *spr)
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = serial_poll( board, conf->pad, conf->sad,
		conf->usec_timeout, spr );
	if(retval < 0)
	{
		if( errno == ETIMEDOUT )
			conf->timed_out = 1;
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

void AllSPoll( int boardID, Addr4882_t addressList[], short resultList[] )
{
	int i;
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}
	if( addressListIsValid( addressList ) == 0 )
	{
		setIberr( EARG );
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

	// XXX could use slightly more efficient ALLSPOLL protocol
	retval = 0;
	for( i = 0; i < numAddresses( addressList ); i++ )
	{
		char result;
		retval = serial_poll( board, extractPAD( addressList[ i ] ),
			extractSAD( addressList[ i ] ), conf->usec_timeout, &result );
		if( retval < 0 )
		{
			if( errno == ETIMEDOUT )
				conf->timed_out = 1;
			break;
		}
		resultList[ i ] = result & 0xff;
	}
	setIbcnt( i );

	if( retval < 0 ) exit_library( boardID, 1 );
	else exit_library( boardID, 0 );
}

void AllSpoll( int boardID, Addr4882_t addressList[], short resultList[] )
{
	AllSPoll( boardID, addressList, resultList );
}
