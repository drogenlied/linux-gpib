/***************************************************************************
                          lib/ibWrt.c
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
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int find_eos( uint8_t *buffer, size_t length, int eos, int eos_flags )
{
	unsigned int i;
	unsigned int compare_mask;

	if( eos_flags & BIN ) compare_mask = 0xff;
	else compare_mask = 0x7f;

	for( i = 0; i < length; i++ )
	{
		if( ( buffer[i] & compare_mask ) == ( eos & compare_mask ) )
		return i;
	}

	return -1;
}

long send_data( ibConf_t *conf, void *buffer, unsigned long count, int send_eoi )
{
	ibBoard_t *board;
	read_write_ioctl_t write_cmd;

	board = interfaceBoard( conf );

	set_timeout( board, conf->usec_timeout );

	write_cmd.buffer = buffer;
	write_cmd.count = count;
	write_cmd.end = send_eoi;

	if( ioctl( board->fileno, IBWRT, &write_cmd) < 0)
	{
		switch( errno )
		{
			case ETIMEDOUT:
				conf->timed_out = 1;
				setIberr( EABO );
				break;
			default:
				setIberr( EDVR );
				setIbcnt( errno );
				break;
		}
		return -1;
	}

	conf->end = send_eoi;

	return count;
}

long send_data_smart_eoi( ibConf_t *conf, void *buffer, unsigned long count, int force_eoi )
{
	int eoi_on_eos;
	int eos_found = 0;
	int send_eoi;
	unsigned long block_size;
	int retval;

	eoi_on_eos = conf->eos_flags & XEOS;

	block_size = count;

	if( eoi_on_eos )
	{
		retval = find_eos( buffer, count, conf->eos, conf->eos_flags );
		if( retval < 0 ) eos_found = 0;
		else
		{
			block_size = retval;
			eos_found = 1;
		}
	}

	send_eoi = force_eoi || ( eoi_on_eos && eos_found );
	if( send_data( conf, buffer, block_size, send_eoi ) < 0 )
	{
		return -1;
	}

	return block_size;
}

ssize_t my_ibwrt( ibConf_t *conf,
	uint8_t *buffer, size_t count )
{
	ibBoard_t *board;
	long block_size;
	size_t bytes_sent = 0;

	board = interfaceBoard( conf );

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( send_setup( conf ) < 0 )
		{
			return -1;
		}
	}

	set_timeout( board, conf->usec_timeout );

	while( count )
	{
		block_size = send_data_smart_eoi( conf, buffer, count, conf->send_eoi );
		if( block_size < 0 )
		{
			return -1;
		}
		count -= block_size;
		bytes_sent += block_size;
		buffer += block_size;
	}

	return bytes_sent;
}

int ibwrt( int ud, void *rd, long cnt )
{
	ibConf_t *conf;
	ssize_t count;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->end = 0;

	count = my_ibwrt( conf, rd, cnt );
	if( count < 0 )
	{
		return exit_library( ud, 1 );
	}

	setIbcnt( count );

	if(count != cnt)
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

ssize_t my_ibwrtf( ibConf_t *conf, const char *file_path )
{
	ibBoard_t *board;
	long block_size, count;
	size_t bytes_sent = 0;
	int retval;
	FILE *data_file;
	struct stat file_stats;
	uint8_t buffer[ 0x1000 ];

	board = interfaceBoard( conf );

	data_file = fopen( file_path, "r" );
	if( data_file == NULL )
	{
		setIberr( EFSO );
		setIbcnt( errno );
		return -1;
	}

	retval = fstat( fileno( data_file ), &file_stats );
	if( retval < 0 )
	{
		setIberr( EFSO );
		setIbcnt( errno );
		return -1;
	}

	count = file_stats.st_size;

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( send_setup( conf ) < 0 )
		{
			return -1;
		}
	}

	set_timeout( board, conf->usec_timeout );

	while( count )
	{
		long fread_count;
		int send_eoi;

		fread_count = fread( buffer, 1, sizeof( buffer ), data_file );
		if( fread_count == 0 )
		{
			setIberr( EFSO );
			setIbcnt( errno );
			return -1;
		}

		send_eoi = conf->send_eoi && ( count == fread_count );
		block_size = send_data_smart_eoi( conf, buffer, fread_count, send_eoi );
		if( block_size < fread_count )
		{
			return -1;
		}
		count -= block_size;
		bytes_sent += block_size;
	}

	return bytes_sent;
}

int ibwrtf( int ud, const char *file_path )
{
	ibConf_t *conf;
	ssize_t count;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->end = 0;

	count = my_ibwrtf( conf, file_path );
	if( count < 0 )
	{
		return exit_library( ud, 1 );
	}

	setIbcnt( count );

	return exit_library( ud, 0 );
}

int InternalSendDataBytes( ibConf_t *conf, void *buffer,
	long count, int eotmode )
{
	int retval;
	unsigned long bytes_sent;

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	switch( eotmode )
	{
		case DABend:
		case NLend:
		case NULLend:
			break;
		default:
			setIberr( EARG );
			return -1;
			break;
	}

	bytes_sent = 0;

	retval = send_data( conf, buffer, count, eotmode == DABend );
	if( retval < 0 ) return retval;
	bytes_sent += retval;

	if( eotmode == NLend )
	{
		retval = send_data( conf, "\n", 1, 1 );
		if( retval < 0 ) return retval;
		bytes_sent += retval;
	}

	setIbcnt( bytes_sent );
	return 0;
}

void SendDataBytes( int boardID, void *buffer,
	long count, int eotmode )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}

	retval = InternalSendDataBytes( conf, buffer, count, eotmode );
	if( retval < 0 )
	{
		exit_library( boardID, 1 );
		return;
	}

	exit_library( boardID, 0 );
}

int InternalSendList( ibConf_t *conf, Addr4882_t addressList[],
	void *buffer, long count, int eotmode )
{
	ibBoard_t *board;
	int retval;

	if( addressListIsValid( addressList ) == 0 ||
		numAddresses( addressList ) == 0 )
	{
		setIberr( EARG );
		return -1;
	}

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	board = interfaceBoard( conf );

	if( is_cic( board ) == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	retval = InternalSendSetup( conf, addressList );
	if( retval < 0 ) return retval;

	retval = InternalSendDataBytes( conf, buffer, count, eotmode );
	if( retval < 0 ) return retval;

	return 0;
}

void SendList( int boardID, Addr4882_t addressList[], void *buffer, long count,
	int eotmode )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}

	retval = InternalSendList( conf, addressList, buffer, count, eotmode );
	if( retval < 0 )
	{
		exit_library( boardID, 1 );
		return;
	}

	exit_library( boardID, 0 );
}

void Send( int boardID, Addr4882_t address, void *buffer, long count,
	int eotmode )
{
	Addr4882_t addressList[ 2 ];

	addressList[ 0 ] = address;
	addressList[ 1 ] = NOADDR;

	SendList( boardID, addressList, buffer, count, eotmode );
}


