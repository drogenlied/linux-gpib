/***************************************************************************
                              gpib_config.c
                             -------------------

    copyright            : (C) 2001,2002,2003 by Frank Mori Hess
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
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ib_internal.h"

typedef struct
{
	char *config_file;
	unsigned int minor;
	char *board_type;
	int irq;
	int iobase;
	int dma;
	int pci_bus;
	int pci_slot;
	int pad;
	int sad;
} parsed_options_t;


static void help( void )
{
	printf("gpib_config [options] - configures a GPIB interface board\n");
	printf("\t-b, --iobase NUM\n"
		"\t\tSet io base address to NUM for boards without plug-and-play cabability.\n");
	printf("\t-d, --dma NUM\n"
		"\t\tSpecify isa dma channel NUM for boards without plug-and-play cabability.\n");
	printf("\t-i, --irq NUM\n"
		"\t\tSpecify irq line NUM for boards without plug-and-play cabability.\n");
	printf("\t-f, --file STRING\n"
		"\t\tSpecify file path for configuration file.  The values in the configuration\n"
		"\t\tfile will be used as defaults for unspecified options.  The default configuration\n"
		"\t\tfile is /etc/gpib.conf\n");
	printf("\t-l, --pci-slot NUM\n"
		"\t\tSpecify pci slot NUM to select a specific pci board.\n"
		"\t\tIf used, you must also specify the pci bus with --pci-bus.\n");
	printf("\t-m, --minor NUM\n"
		"\t\tConfigure gpib device file with minor number NUM (default 0).\n");
	printf("\t-p, --pad NUM\n"
		"\t\tSpecify primary gpib address.  NUM should be in the range 0 through 30.\n");
	printf("\t-s, --sad NUM\n"
		"\t\tSpecify secondary gpib address.  NUM should be 0 (disabled) or in the range\n"
		"\t\t0x60 through 0x78.\n");
	printf("\t-t, --board-type STRING\n"
		"\t\tSet board type to STRING\n");
	printf("\t-u, --pci-bus NUM\n"
		"\t\tSpecify pci bus NUM to select a specific pci board.\n"
		"\t\tIf used, you must also specify pci slot with --pci-slot.\n");
}

static void parse_options( int argc, char *argv[], parsed_options_t *settings )
{
	int c, index;

	struct option options[] = {
		{ "minor", 1, 0, 'm' },
		{ "board-type", 1, 0, 't' },
		{ "irq", 1, 0, 'i' },
		{ "iobase", 1, 0, 'b' },
		{ "dma", 1, 0, 'd' },
		{ "pci-bus", 1, 0, 'u' },
		{ "pci-slot", 1, 0, 'l' },
		{ "pad", 1, 0, 'p' },
		{ "sad", 1, 0, 's' },
		{ "file", 1, 0, 'f' },
		{ "help", 1, 0, 'h' },
		{ 0 },
	};

	settings->config_file = NULL;
	settings->minor = 0;
	settings->board_type = NULL;
	settings->irq = -1;
	settings->iobase = -1;
	settings->dma = -1;
	settings->pci_bus = -1;
	settings->pci_slot = -1;
	settings->pad = -1;
	settings->sad = -1;

	while( 1 )
	{
		c = getopt_long(argc, argv, "b:d:f:h:i:l:m:p:s:t:u:", options, &index);
		if( c == -1 ) break;
		switch( c )
		{
		case 0:
			break;
		case 'b':
			settings->iobase = strtol( optarg, NULL, 0 );
			break;
		case 'd':
			settings->dma = strtol( optarg, NULL, 0 );
			break;
		case 'f':
			settings->config_file = strdup( optarg );
			break;
		case 'h':
			help();
			exit( 0 );
			break;
		case 'i':
			settings->irq = strtol( optarg, NULL, 0 );
			break;
		case 'l':
			settings->pci_slot = strtol( optarg, NULL, 0 );
			break;
		case 'm':
			settings->minor = strtol( optarg, NULL, 0 );
			break;
		case 'p':
			settings->pad = strtol( optarg, NULL, 0 );
			break;
		case 's':
			settings->sad = strtol( optarg, NULL, 0 );
			break;
		case 't':
			settings->board_type = strdup( optarg );
			break;
		case 'u':
			settings->pci_bus = strtol( optarg, NULL, 0 );
			break;
		default:
			help();
			exit(1);
		}
	}
}

static int configure_board( ibBoard_t *board, unsigned int pad, int sad )
{
	board_type_ioctl_t boardtype;
	select_pci_ioctl_t pci_selection;
	int retval;

	strncpy( boardtype.name, board->board_type, sizeof( boardtype.name ) );
	retval = ioctl( board->fileno, CFCBOARDTYPE, &boardtype );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCBASE, &board->base );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCIRQ, &board->irq );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCDMA, &board->dma );
	if( retval < 0 ) return retval;
 	retval = ioctl( board->fileno, IBPAD, &pad );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, IBSAD, &sad );
	if( retval < 0 ) return retval;

	pci_selection.pci_bus = board->pci_bus;
	pci_selection.pci_slot = board->pci_slot;
	retval = ioctl( board->fileno, IBSELECT_PCI, &pci_selection );
	if( retval < 0 ) return retval;

	return 0;
}

int main( int argc, char *argv[] )
{
	ibConf_t configs[ FIND_CONFIGS_LENGTH ];
	ibBoard_t boards[ MAX_BOARDS ];
	char *filename, *envptr, *devicefile;
	int retval;
	parsed_options_t options;
	ibBoard_t *board;
	ibConf_t *conf;
	unsigned int pad = 0;
	int sad = -1;
	int i;

	parse_options( argc, argv, &options );

	envptr = getenv( "IB_CONFIG" );
	if( options.config_file ) filename = options.config_file;
	else if( envptr ) filename = envptr;
	else filename = DEFAULT_CONFIG_FILE;

	retval = parse_gpib_conf( filename, configs, FIND_CONFIGS_LENGTH,
		boards, MAX_BOARDS );
	if( retval < 0 )
	{
		fprintf( stderr, "failed to parse config file %s\n", filename );
		return retval;
	}

	if( options.minor >= MAX_BOARDS )
	{
		fprintf( stderr, "minor number %i out of range\n", options.minor );
		return -1;
	}

	for( i = 0; i < FIND_CONFIGS_LENGTH; i++ )
	{
		if( configs[ i ].is_interface == 0 ) continue;
		if( configs[ i ].settings.board != options.minor ) continue;
		conf = &configs[ i ];
		pad = conf->settings.pad;
		sad = conf->settings.sad;
		break;
	}
	board = &boards[ options.minor ];

	asprintf( &devicefile, "/dev/gpib%i", options.minor );
	if( devicefile == NULL )
	{
		perror( __FUNCTION__ );
		return -1;
	}
	if( options.board_type )
		strncpy( board->board_type, options.board_type, sizeof( board->board_type ) );
	if( options.irq >= 0 )
		board->irq = options.irq;
	if( options.iobase >= 0 )
		board->base = options.iobase;
	if( options.dma >= 0 )
		board->dma = options.dma;
	if( options.pci_bus >= 0 )
		board->pci_bus = options.pci_bus;
	if( options.pci_slot >= 0 )
		board->pci_slot = options.pci_slot;
	if( options.pad >= 0 )
		pad = options.pad;
	if( options.sad >= 0 )
		sad = options.sad - sad_offset;

	board->fileno = open( devicefile, O_RDWR );
	if( board->fileno < 0 )
	{
		fprintf( stderr, "failed to open device file '%s'\n", devicefile );
		perror( __FUNCTION__ );
		return board->fileno;
	}
	retval = configure_board( board, pad, sad );
	if( retval < 0 )
	{
		fprintf( stderr, "failed to configure board\n" );
		perror( __FUNCTION__ );
		return retval;
	}
	close( board->fileno );
	board->fileno = -1;
	free( devicefile );
	if( options.config_file ) free( options.config_file );
	if( options.board_type ) free( options.board_type );

	return 0;
}
