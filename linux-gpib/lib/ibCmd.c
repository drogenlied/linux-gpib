
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

int ibcmd(int ud, void *cmd, long cnt)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	// check that ud is an interface board
	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	board = interfaceBoard( conf );

	count = my_ibcmd( conf, cmd, cnt);

	if(count < 0)
	{
		switch(errno)
		{
			case ETIMEDOUT:
				setIberr( EABO );
				board->timed_out = 1;
				break;
			default:
				break;
		}
		return exit_library( ud, 1);
	}

	if(count != cnt)
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

ssize_t my_ibcmd( const ibConf_t *conf, uint8_t *buffer, size_t count)
{
	read_write_ioctl_t cmd;
	int retval;
	const ibBoard_t *board = &ibBoard[ conf->board ];

	// check that interface board is master
	if( board->is_system_controller == 0 )
	{
		return -1;
	}

	cmd.buffer = buffer;
	cmd.count = count;

	set_timeout( board, conf->usec_timeout);

	retval = ioctl( board->fileno, IBCMD, &cmd );
	if( retval < 0 )
	{
		return -1;
	}

	return cmd.count;
}

int send_setup_string( const ibConf_t *conf,
	uint8_t *cmdString )
{
	int pad = conf->pad;
	int sad = conf->sad;
	unsigned int i = 0;
	const ibBoard_t *board = &ibBoard[ conf->board ];

	if( pad < 0 || pad > gpib_addr_max || sad > gpib_addr_max)
	{
		fprintf(stderr, "libgpib: bug! bad conf gpib address\n");
		return -1;
	}

	if( board->sad > gpib_addr_max )
	{
		fprintf(stderr, "libgpib: bug! bad board gpib secondary address\n");
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

	return i;
}

int send_setup( const ibBoard_t *board, const ibConf_t *conf )
{
	uint8_t cmdString[8];
	int retval;

	retval = send_setup_string( conf, cmdString );
	if( retval < 0 ) return retval;

	if( my_ibcmd( conf, cmdString, retval ) < 0 )
		return -1;

	return 0;
}
