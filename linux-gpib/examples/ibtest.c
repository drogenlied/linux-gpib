/***************************************************************************
                                 ibtest.c
                             -------------------

Example program which uses gpib c library.  More complex than an example
program needs to be really, but useful for testing library functions.

    copyright            : (C) 2002 by Frank Mori Hess
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <gpib/ib.h>

void fprint_status( FILE* filep, char *msg  );

enum Action
{
	GPIB_IFC,
	GPIB_QUIT,
	GPIB_READ,
	GPIB_REQUEST_SERVICE,
	GPIB_SERIAL_POLL,
	GPIB_TIMEOUT,
	GPIB_WAIT,
	GPIB_WRITE,
	GPIB_LINE_STATUS,
};

void descriptor_type( int ud, int *is_board, int *is_master )
{
	int master;
	int status;

	status = ibask( ud, IbaSC, &master );

	if( status & ERR )
	{
		if( ThreadIberr() != EARG )
		{
			fprint_status( stderr, "ibask error" );
			abort();
		}
		*is_board = 0;
		*is_master = 0;
	}else
	{
		*is_board = 1;
		*is_master = master;
	}
}

int descriptor_is_board( int ud )
{
	int is_board;
	int is_master;
	descriptor_type( ud, &is_board, &is_master );
	return is_board;
}

int descriptor_is_master( int ud )
{
	int is_board;
	int is_master;
	descriptor_type( ud, &is_board, &is_master );
	return is_board && is_master;
}

// returns a device descriptor after prompting user for primary address
int prompt_for_device(void)
{
	int ud, pad;
	const int sad = 0;
	const int minor = 0;
	const int send_eoi = 1;
	const int eos_mode = 0;
	const int timeout = T1s;
	char input[100];

	while(1)
	{
		printf("enter primary gpib address for device you wish to open [0-30]: ");
		fgets(input, sizeof(input), stdin);
		pad = strtol(input, NULL, 0);
		if(pad < 0 || pad > 30)
			printf("primary address must be between 0 and 30\n");
		else break;
	}

	printf("trying to open pad = %i on /dev/gpib%i ...\n", pad, minor);
	ud = ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
	if(ud < 0)
	{
		fprint_status( stderr, "ibdev error\n");
		abort();
	}

	return ud;
}

// returns a device descriptor after prompting user for primary address
int prompt_for_board( void )
{
	int ud;
	char board_name[100];

	do
	{
		int length;
		printf("enter name of interface board (or device) you wish to open: ");
		fgets( board_name, sizeof( board_name ), stdin);
		length = strlen( board_name );
		if( board_name[ length - 1 ] == '\n' )
			board_name[ length - 1 ] = 0;
		printf( "trying to open board named '%s'\n", board_name );
		ud = ibfind( board_name );
		if(ud < 0)
		{
			fprintf( stderr, "failed to open board\n" );
		}
	}while( ud < 0 );

	return ud;
}

int prompt_for_descriptor( void )
{
	char input[100];

	do
	{
		printf( "Do you wish to open a (d)evice or an interface (b)oard?\n"
			"\t(you probably want to open a device): ");
		fgets( input, sizeof( input ), stdin );
		switch( input[0] )
		{
			case 'd':
			case 'D':
				return prompt_for_device();
				break;
			case 'b':
			case 'B':
				return prompt_for_board();
				break;
			default:
				break;
		}
	} while( 1 );

	return -1;
}

// asks user what they want to do next
int prompt_for_action(void)
{
	char input[100];
	while(1)
	{
		printf("You can:\n"
			"\tw(a)it for an event\n"
			"\tsend (i)nterface clear (system controller only)\n"
			"\tget bus (l)ine status (board only)\n"
 			"\t(q)uit\n"
			"\t(r)ead string\n"
			"\tperform (s)erial poll (device only)\n"
			"\tchange (t)imeout on io operations\n"
			"\trequest ser(v)ice (board only)\n"
			"\t(w)rite string\n"
			": " );
		do fgets( input, sizeof( input ), stdin );
		while( input[0] == '\n' );

		switch( input[0] )
		{
			case 'A':
			case 'a':
				return GPIB_WAIT;
				break;
			case 'L':
			case 'l':
				return GPIB_LINE_STATUS;
				break;
			case 'I':
			case 'i':
				return GPIB_IFC;
				break;
			case 'q':
			case 'Q':
				return GPIB_QUIT;
				break;
			case 'r':
			case 'R':
				return GPIB_READ;
				break;
			case 's':
			case 'S':
				return GPIB_SERIAL_POLL;
				break;
			case 't':
			case 'T':
				return GPIB_TIMEOUT;
				break;
			case 'v':
			case 'V':
				return GPIB_REQUEST_SERVICE;
				break;
			case 'w':
			case 'W':
				return GPIB_WRITE;
				break;
			default:
				fprintf( stderr, "invalid selection\n");
				break;
		}
	}

	return -1;
}

int perform_read(int ud)
{
	char buffer[ 1024 ];

	printf("trying to read from device...\n");

	if( ibrd(ud, buffer, sizeof(buffer) - 1) & ERR)
	{
		return -1;
	}
	// make sure string is null-terminated
	buffer[ibcnt] = 0;
	printf("received string: '%s'\n"
		"number of bytes read: %i\n", buffer, ibcnt);
	return 0;
}

int prompt_for_write(int ud)
{
	char buffer[ 1024 ];

	printf("enter a string to send to your device: ");
	fgets( buffer, sizeof(buffer), stdin );

	printf("sending string: %s\n", buffer);
	if( ibwrt(ud, buffer, strlen(buffer)) & ERR )
	{
		return -1;
	}
	return 0;
}

int do_serial_poll( int ud )
{
	char result;
	int status;

	if( descriptor_is_board( ud ) != 0 )
	{
		fprintf( stderr, "You have a board open (as opposed to a device).\n"
			"\tYou cannot perform a serial poll.\n" );
		return -1;
	}

	status = ibrsp( ud, &result );
	if( status & ERR )
	{
		return -1;
	}

	printf( "serial poll result: 0x%x\n", ( (unsigned int) result ) & 0xff );
	return 0;
}

int request_service( int ud )
{
	int status_byte;
	int status;

	if( descriptor_is_board( ud ) == 0 )
	{
		fprintf( stderr, "You have a device open (as opposed to a board).\n"
			"\tYou cannot request service.\n" );
		return -1;
	}

	printf( "enter new status byte (bit 0x40 requests service): " );
	scanf( "%i", &status_byte );

	status = ibrsv( ud, status_byte );

	if( status & ERR )
	{
		return -1;
	}

	return 0;
}

int prompt_for_timeout( int ud )
{
	int timeout;

	printf( "enter the desired timeout:\n"
		"\t(0) none\n"
		"\t(1) 10 microsec\n"
		"\t(2) 30 microsec\n"
		"\t(3) 100 microsec\n"
		"\t(4) 300 microsec\n"
		"\t(5) 1 millisec\n"
		"\t(6) 3 millisec\n"
		"\t(7) 10 millisec\n"
		"\t(8) 30 millisec\n"
		"\t(9) 100 millisec\n"
		"\t(10) 300 millisec\n"
		"\t(11) 1 sec\n"
		"\t(12) 3 sec\n"
		"\t(13) 10 sec\n"
		"\t(14) 30 sec\n"
		"\t(15) 100 sec\n"
		"\t(16) 300 sec\n"
		"\t(17) 1000 sec\n"
		);
	scanf( "%i", &timeout );

	if( ibtmo( ud, timeout ) & ERR )
	{
		fprintf( stderr, "failed to set timeout to %i\n", timeout );
		return -1;
	}
	return 0;
}

int prompt_for_wait( int ud )
{
	int wait_mask;
	int status;

	printf( "Possible wait bits:\n"
		"\t0x%x timeout\n"
		"\t0x%x device requesting service\n",
		TIMO, RQS );
	printf("Enter wait mask: ");
	scanf( "%i", &wait_mask );

	status = ibwait( ud, wait_mask );

	return 0;
}

int get_lines( int ud )
{
	short line_status;

	if( iblines( ud, &line_status ) & ERR )
		return -1;

	printf( "Line status word is 0x%x\n", line_status & 0xffff );

	return 0;
}

int interface_clear( int ud )
{

	if( ibsic( ud ) & ERR )
		return -1;

	printf( "Inferface clear sent\n" );

	return 0;
}

int main(int argc,char **argv)
{
	int dev;
	enum Action act;

	dev = prompt_for_descriptor();

	do
	{
		act = prompt_for_action();

		if( act == GPIB_QUIT ) break;

		switch( act )
		{
			case GPIB_IFC:
				interface_clear( dev );
				break;
			case GPIB_LINE_STATUS:
				get_lines( dev );
				break;
			case GPIB_READ:
				perform_read( dev );
				break;
			case GPIB_REQUEST_SERVICE:
				request_service( dev );
				break;
			case GPIB_SERIAL_POLL:
				do_serial_poll( dev );
				break;
			case GPIB_TIMEOUT:
				prompt_for_timeout( dev );
				break;
			case GPIB_WAIT:
				prompt_for_wait( dev );
				break;
			case GPIB_WRITE:
				prompt_for_write( dev );
				break;
			default:
				fprintf( stderr, "bug, unknown selection\n");
				break;
		}
		fprint_status( stdout, "gpib status is: " );

	}while( act != GPIB_QUIT );

	ibonl(dev, 0);
	return 0;
}

/*
* This is a simple error handling function
*
*/

void fprint_status( FILE* filep, char *msg )
{
	fprintf( filep, "%s\n", msg);

	fprintf( filep, "ibsta = 0x%x  < ", ThreadIbsta() );
	if ( ThreadIbsta() & ERR )  fprintf( filep, "ERR " );
	if ( ThreadIbsta() & TIMO ) fprintf( filep, "TIMO " );
	if ( ThreadIbsta() & END )  fprintf( filep, "END " );
	if ( ThreadIbsta() & SRQI ) fprintf( filep, "SRQI " );
	if ( ThreadIbsta() & RQS ) fprintf( filep, "RQS " );
	if ( ThreadIbsta() & CMPL ) fprintf( filep, "CMPL " );
	if ( ThreadIbsta() & CIC )  fprintf( filep, "CIC " );
	if ( ThreadIbsta() & ATN )  fprintf( filep, "ATN " );
	if ( ThreadIbsta() & TACS ) fprintf( filep, "TACS " );
	if ( ThreadIbsta() & LACS ) fprintf( filep, "LACS " );
	fprintf( filep, ">\n" );

	fprintf( filep,"iberr= %d\n", iberr);
	if( ( ThreadIbsta() & ERR ) )
	{
		fprintf( filep, "%s\n", gpib_error_string( ThreadIberr() ) );
	}

	fprintf( filep, "\n" );

	fprintf( filep, "ibcnt = %d\n", ibcnt );
}

