/***************************************************************************
                          lib/ibutil.c
                             -------------------

	copyright            : (C) 2001,2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ib_internal.h"
#include "ibP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "parse.h"

ibConf_t *ibConfigs[NUM_CONFIGS];

int insert_descriptor( int ud, ibConf_t p )
{
	ibConf_t *conf;

	if( ibConfigs[ ud ] != NULL )	return -1;

	ibConfigs[ ud ] = malloc( sizeof( ibConf_t ) );
	if( ibConfigs[ ud ] == NULL ) return -1;

	conf = ibConfigs[ ud ];

	/* put entry to the table */
	*conf = p;
	init_async_op( &conf->async );

	return 0;
}

void setup_global_board_descriptors( void )
{
	int i;

	for( i = 0; i < FIND_CONFIGS_LENGTH; i++ )
	{
		if( ibFindConfigs[ i ].is_interface )
		{
			insert_descriptor( ibFindConfigs[ i ].board, ibFindConfigs[ i ] );
		}
	}
}

// initialize variables used in yyparse() - see ibConfYacc.y
void init_yyparse_vars( void )
{
	int i ;

	findIndex = 0; bdid = 0;
	for( i = 0; i < FIND_CONFIGS_LENGTH; i++ )
	{
		init_ibconf( &ibFindConfigs[ i ] );
	}

	// initialize line counter to 1 before calling yyparse
	yylloc.first_line = 1;

	ibBoardDefaultValues();
}

int ibParseConfigFile( void )
{
	extern FILE *gpib_yyin;
	FILE *infile;
	int stat = 0;
	static int config_parsed = 0;
	char *filename, *envptr;

	if( config_parsed ) return 0;

	envptr = getenv("IB_CONFIG");
	if( envptr ) filename = envptr;
	else filename = DEFAULT_CONFIG_FILE;

	if ((infile = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "failed to open configuration file\n");
		setIberr( EDVR );
		return -1;
	}

	/*ibPutMsg("Parse Config %s",filename); */
	gpib_yyin = infile;
	gpib_yyrestart(gpib_yyin); /* Hmm ? */

	init_yyparse_vars();
	if(gpib_yyparse() < 0)
	{
		fprintf(stderr, "failed to parse configuration file\n");
		stat = -1 ;
	}
	fclose(infile);

	if( stat == 0 )
	{
		setup_global_board_descriptors();
		config_parsed = 1;
	}

	return stat;
}

/**********************************************************************/

int ibGetDescriptor(ibConf_t p)
{
	int ib_ndev;
	int retval;

	/* check validity of values */
	if( p.pad >= gpib_addr_max || p.sad >= gpib_addr_max )
	{
		setIberr( ETAB );
		return -1;
	}
	// search for an unused descriptor
	for( ib_ndev = MAX_BOARDS; ib_ndev < NUM_CONFIGS; ib_ndev++ )
	{
		if( ibConfigs[ ib_ndev ] == NULL )
		{
			retval = insert_descriptor( ib_ndev, p );
			if( retval < 0 )
			{
				setIberr( ETAB );
				return retval;
			}
			break;
		}
	}
	if( ib_ndev == NUM_CONFIGS)
	{
		setIberr( ETAB );
		return -1;
	}

	return ib_ndev;
}

/**********************************************************************/
int ibFindDevIndex(char *name)
{
	int i;

	if( strcmp( "", name ) == 0 ) return -1;

	for(i = 0; i < FIND_CONFIGS_LENGTH; i++)
	{
		if(!strcmp(ibFindConfigs[i].name, name)) return i;
	}

	return -1;
}

int ibCheckDescriptor( int ud )
{
	int retval;

	if( ud < 0 || ud > NUM_CONFIGS || ibConfigs[ud] == NULL )
		return -1;

	retval = conf_online( ibConfigs[ ud ], 1 );
	if( retval < 0 ) return retval;

	return 0;
}

void init_ibconf( ibConf_t *conf )
{
	conf->name[0] = 0;
	conf->pad = -1;
	conf->sad = -1;
	conf->init_string[0] = 0;
	conf->board = -1;
	conf->eos = 0;
	conf->eos_flags = 0;
	conf->flags = 0;
	conf->usec_timeout = 3000000;
	conf->spoll_usec_timeout = 1000000;
	conf->ppoll_usec_timeout = 2;
	conf->send_eoi = 1;
	conf->is_interface = 0;
	conf->dev_is_open = 0;
	conf->board_is_open = 0;
	conf->has_lock = 0;
	conf->ppoll_config = 0;
	conf->local_lockout = 0;
	conf->timed_out = 0;
}

int ib_lock_mutex( ibBoard_t *board )
{
	int lock = 1;
	return ioctl( board->fileno, IBMUTEX, &lock );
}

int ib_unlock_mutex( ibBoard_t *board )
{
	int unlock = 0;
	return ioctl( board->fileno, IBMUTEX, &unlock );
}

int open_gpib_device( ibConf_t *conf )
{
	open_close_dev_ioctl_t open_cmd;
	int retval;
	ibBoard_t *board;

	if( conf->dev_is_open ||
		conf->is_interface ) return 0;

	board = interfaceBoard( conf );

	open_cmd.pad = conf->pad;
	open_cmd.sad = conf->sad;
	retval = ioctl( board->fileno, IBOPENDEV, &open_cmd );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	conf->dev_is_open = 1;

	return 0;
}

int close_gpib_device( ibConf_t *conf )
{
	open_close_dev_ioctl_t close_cmd;
	int retval;
	ibBoard_t *board;

	if( conf->dev_is_open == 0 ||
		conf->is_interface ) return 0;

	board = interfaceBoard( conf );

	close_cmd.pad = conf->pad;
	close_cmd.sad = conf->sad;
	retval = ioctl( board->fileno, IBCLOSEDEV, &close_cmd );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	conf->dev_is_open = 0;

	return 0;
}

int gpibi_change_address( ibConf_t *conf, unsigned int pad, int sad )
{
	int retval;
	ibBoard_t *board;

	board = interfaceBoard( conf );

	if ( conf->is_interface )
	{
		if( pad != conf->pad )
		{
			retval = ioctl( board->fileno, IBPAD, &pad );
			if( retval < 0 )
			{
				setIberr( EDVR );
				setIbcnt( errno );
				return retval;
			}
		}

		if( sad != conf->sad )
		{
			retval = ioctl( board->fileno, IBSAD, &sad );
			if( retval < 0 )
			{
				setIberr( EDVR );
				setIbcnt( errno );
				return retval;
			}
		}
	}

	retval = close_gpib_device( conf );
	if( retval < 0 ) return retval;

	conf->pad = pad;
	conf->sad = sad;

	retval = open_gpib_device( conf );
	if( retval < 0 ) return retval;

	return 0;
}

int lock_board_mutex( ibBoard_t *board )
{
	static const int lock = 1;
	int retval;

	retval = ioctl( board->fileno, IBMUTEX, &lock );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: error locking board mutex!\n");
		setIberr( EDVR );
		setIbcnt( errno );
	}

	return retval;
}

int unlock_board_mutex( ibBoard_t *board )
{
	static const int unlock = 0;
	int retval;

	retval = ioctl( board->fileno, IBMUTEX, &unlock );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: error unlocking board mutex!\n");
		setIberr( EDVR );
		setIbcnt( errno );
	}
	return retval;
}

ibConf_t * enter_library( int ud )
{
	return general_enter_library( ud, 0, 0 );
}

ibConf_t * general_enter_library( int ud, int no_lock_board, int ignore_eoip )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		setIberr( EDVR );
		return NULL;
	}
	conf->timed_out = 0;

	board = interfaceBoard( conf );

	if( no_lock_board == 0 )
	{
		if( ignore_eoip == 0 )
		{
			if( conf->async.in_progress )
			{
				setIberr( EOIP );
				return NULL;
			}
		}

		retval = lock_board_mutex( board );
		if( retval < 0 )
		{
			setIberr( EDVR );
			return NULL;
		}
		conf->has_lock = 1;
	}

	return conf;
}

int ibstatus( ibConf_t *conf, int error )
{
	int status = 0;
	int retval;

	if( conf->is_interface )
	{
		int board_status;
		retval = ioctl( interfaceBoard( conf )->fileno, IBSTATUS, &board_status );
		if( retval < 0 )
		{
			error++;
			setIberr( EDVR );
		}else
			status |= board_status & board_status_mask;
	}else
	{
		spoll_bytes_ioctl_t cmd;
		cmd.pad = conf->pad;
		cmd.sad = conf->sad;
		retval = ioctl( interfaceBoard( conf )->fileno, IBSPOLL_BYTES, &cmd );
		if( retval < 0 )
		{
			error++;
			setIberr( EDVR );
		}else
		{
			if( cmd.num_bytes > 0 )
				status |= RQS;
		}
	}

	// XXX
	status |= CMPL;
	if( error ) status |= ERR;
	if( conf->timed_out )
		status |= TIMO;
	if( conf->end )
		status |= END;

	setIbsta( status );

	return status;
}

int exit_library( int ud, int error )
{
	return general_exit_library( ud, error, 0 );
}

int general_exit_library( int ud, int error, int keep_lock )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval;
	int status;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		setIberr( EDVR );
		setIbsta( ERR );
		sync_globals();
		return ERR;
	}

	board = interfaceBoard( conf );

	status = ibstatus( conf, error );

	if( conf->has_lock && !keep_lock )
	{
		conf->has_lock = 0;
		retval = unlock_board_mutex( board );
		if( retval < 0 )
		{
			setIberr( EDVR );
			conf->has_lock = 1;
			status = ibstatus( conf, error );
		}
	}

	sync_globals();

	return status;
}

int extractPAD( Addr4882_t address )
{
	int pad = address & 0xff;

	if( address == NOADDR ) return ADDR_INVALID;

	if( pad < 0 || pad > gpib_addr_max ) return ADDR_INVALID;

	return pad;
}

int extractSAD( Addr4882_t address )
{
	int sad = ( address >> 8 ) & 0xff;

	if( address == NOADDR ) return ADDR_INVALID;

	if( sad == NO_SAD ) return SAD_DISABLED;

	if( ( sad & 0x60 ) == 0 ) return ADDR_INVALID;

	sad &= ~0x60;

	if( sad < 0 || sad > gpib_addr_max ) return ADDR_INVALID;

	return sad;
}

Addr4882_t packAddress( unsigned int pad, int sad )
{
	Addr4882_t address;

	address = 0;
	address |= pad & 0xff;
	if( sad >= 0 )
		address |= ( ( sad | 0x60 ) << 8 ) & 0xff00;

	return address;
}

int addressIsValid( Addr4882_t address )
{
	if( address == NOADDR ) return 1;

	if( extractPAD( address ) == ADDR_INVALID ||
		extractSAD( address ) == ADDR_INVALID ) return 0;

	return 1;
}

int addressListIsValid( Addr4882_t addressList[] )
{
	int i;

	if( addressList == NULL ) return 1;

	for( i = 0; addressList[ i ] != NOADDR; i++ )
	{
		if( addressIsValid( addressList[ i ] ) == 0 )
		{
			setIberr( EARG );
			setIbcnt( i );
			return 0;
		}
	}

	return 1;
}

unsigned int numAddresses( Addr4882_t addressList[] )
{
	unsigned int count;

	if( addressList == NULL )
		return 0;

	count = 0;
	while( addressList[ count ] != NOADDR )
	{
		count++;
	}

	return count;
}
