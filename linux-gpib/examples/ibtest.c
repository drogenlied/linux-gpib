/***************************************************************************
                                 ibtest.c
                             -------------------

Example program which uses gpib c library, good for initial test of library.

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
#include <ib.h>
#include <stdint.h>
#include <string.h>

uint8_t buffer[1024];

void gpiberr(char *msga);

enum Action
{
	GPIB_QUIT,
	GPIB_READ,
	GPIB_TIMEOUT,
	GPIB_WRITE,
};

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
		printf("enter primary address for device (not interface board) [0-30]: ");
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
		printf("failed to get descriptor\n");
	}

	return ud;
}

// asks user what they want to do next
int prompt_for_action(void)
{
	char input[10];
	while(1)
	{
		printf("You can:\n"
 			"\t(q)uit\n"
			"\t(r)ead string from device\n"
			"\tchange (t)imeout on io operations\n"
			"\t(w)rite string to device\n"
			);
		fgets( input, sizeof( input ), stdin );
		switch( input[0] )
		{
			case 'q':
			case 'Q':
				return GPIB_QUIT;
				break;
			case 'r':
			case 'R':
				return GPIB_READ;
				break;
			case 't':
			case 'T':
				return GPIB_TIMEOUT;
				break;
			case 'w':
			case 'W':
				return GPIB_WRITE;
				break;
			default:
				break;
		}
	}

	return -1;
}

int perform_read(int ud)
{
	char buffer[1024];

	printf("trying to read from device...\n");

	if( ibrd(ud, buffer, sizeof(buffer) - 1) & ERR)
	{
		gpiberr("read error");
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
	char buffer[1024];

	printf("enter a string to send to your device: ");
	fgets( buffer, sizeof(buffer), stdin );

	printf("sending string: %s\n", buffer);
	if( ibwrt(ud, buffer, strlen(buffer)) & ERR )
	{
		gpiberr("write error");
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

int main(int argc,char **argv)
{
	int dev;
	enum Action act;

	dev = prompt_for_device();
	if(dev < 0) exit(1);

	/*
	* send device reset
	*
	*/

	printf("clearing device..\n");
	if( (ibclr(dev) & ERR ) && iberr != EARG )
	{
		gpiberr("gpib clear error");
		ibonl(dev, 0);
		exit(1);
	}

	do	
	{
		act = prompt_for_action();

		switch( act )
		{
			case GPIB_READ:
				if(perform_read(dev) < 0)
				{
					ibonl(dev, 0);
					exit(1);
				}
				break;
			case GPIB_TIMEOUT:
				prompt_for_timeout( dev ); 
				break;
			case GPIB_WRITE:
				if(prompt_for_write(dev) < 0)
				{
					ibonl(dev, 0);
					exit(1);
				}
				break;
			default:
				break;
		}
	}while( act != GPIB_QUIT );


#if 0
	fprintf(stderr, "\nserial poll\n");
	if( ibrsp(dev, &result) & ERR )
	{
		gpiberr("serial poll error");
		ibonl(dev, 0);
		exit(1);
	}
	fprintf(stderr, "result 0x%x\n", result);
#endif

	ibonl(dev, 0);
	return 0;
}

/*
* This is a simple error handling function
*
*/


void gpiberr(char *msg)
{

	fprintf(stderr, "%s\n", msg);

	fprintf(stderr, "ibsta = 0x%x  <", ibsta);

	if ( ibsta & ERR )  fprintf( stderr," ERR");
	if ( ibsta & TIMO ) fprintf( stderr," TIMO");
	if ( ibsta & END )  fprintf( stderr," END");
	if ( ibsta & SRQI ) fprintf( stderr," SRQI");
	if ( ibsta & CMPL ) fprintf( stderr," CMPL");
	if ( ibsta & CIC )  fprintf( stderr," CIC");
	if ( ibsta & ATN )  fprintf( stderr," ATM");
	if ( ibsta & TACS ) fprintf( stderr," TACS");
	if ( ibsta & LACS ) fprintf( stderr," LACS");

	fprintf( stderr," >\n");

	fprintf( stderr,"iberr= %d", iberr);
	if ( iberr == EDVR) fprintf( stderr," EDVR <OS Error>\n");
	if ( iberr == ECIC) fprintf( stderr," ECIC <Not CIC>\n");
	if ( iberr == ENOL) fprintf( stderr," ENOL <No Listener>\n");
	if ( iberr == EADR) fprintf( stderr," EADR <Adress Error>\n");
	if ( iberr == EARG) fprintf( stderr," ECIC <Invalid Argument>\n");
	if ( iberr == ESAC) fprintf( stderr," ESAC <No Sys Ctrlr>\n");
	if ( iberr == EABO) fprintf( stderr," EABO <Operation Aborted>\n");
	if ( iberr == ENEB) fprintf( stderr," ENEB <No Gpib Board>\n");
	if ( iberr == EOIP) fprintf( stderr," EOIP <Async I/O in prg>\n");
	if ( iberr == ECAP) fprintf( stderr," ECAP <No Capability>\n");
	if ( iberr == EFSO) fprintf( stderr," EFSO <File sys. error>\n");
	if ( iberr == EBUS) fprintf( stderr," EBUS <Command error>\n");
	if ( iberr == ESTB) fprintf( stderr," ESTB <Status byte lost>\n");
	if ( iberr == ESRQ) fprintf( stderr," ESRQ <SRQ stuck on>\n");
	if ( iberr == ETAB) fprintf( stderr," ETAB <Table Overflow>\n");
	if ( iberr == EPAR) fprintf( stderr," EPAR <Parse Error in Config>\n");
	if ( iberr == ECFG) fprintf( stderr," ECFG <Can't open Config>\n");
	if ( iberr == ETAB) fprintf( stderr," ETAB <Device Table Overflow>\n");
	if ( iberr == ENSD) fprintf( stderr," ENSD <Configuration Error>\n");

	fprintf(stderr, "ibcnt = %d\n", ibcnt );
}

