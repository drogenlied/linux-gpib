
#include "ib_internal.h"
#include <ibP.h>

int ibeos(int ud, int v)
{
	ibConf_t *conf;

	conf = general_enter_library( ud, 1, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->eos = v & 0xff;
	conf->eos_flags = v & 0xff00;

	return exit_library( ud, 0 );
}

/*
 *  iblcleos()
 *  sets the eos modes of the unit description (local) or
 *  board description (if none)
 *
 */

int iblcleos( const ibConf_t *conf )
{
	int use_eos, compare8;

	use_eos = conf->eos_flags & REOS;
	compare8 = conf->eos_flags & BIN;

	return config_read_eos( interfaceBoard( conf ), use_eos, conf->eos, compare8 ) ;
}

int config_read_eos( ibBoard_t *board, int use_eos_char, int eos_char,
	int compare_8_bits )
{
	eos_ioctl_t eos_cmd;
	int retval;

	eos_cmd.eos_flags = 0;
	if( use_eos_char )
		eos_cmd.eos_flags |= REOS;
	if( compare_8_bits )
		eos_cmd.eos_flags |= BIN;

	eos_cmd.eos = 0;
	if( use_eos_char )
	{
		eos_cmd.eos = eos_char;
		eos_cmd.eos &= 0xff;
		if( eos_cmd.eos != eos_char )
		{
			setIberr( EARG );
			return -1;
		}
	}

	retval = ioctl( board->fileno, IBEOS, &eos_cmd );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
	}

	return retval;
}
