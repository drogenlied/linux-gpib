/***************************************************************************
                          lib/ibconfig.c
                             -------------------

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

#include "ib_internal.h"
#include "ibP.h"

int ibconfig( int ud, int option, int value )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	switch( option )
	{
		case IbcPAD:
			retval = internal_ibpad( conf, value );
			if( retval < 0 ) return exit_library( ud, 1 );
			return exit_library( ud, 0 );
			break;
		case IbcSAD:
			retval = internal_ibsad( conf, value );
			if( retval < 0 ) return exit_library( ud, 1 );
			return exit_library( ud, 0 );
			break;
		case IbcTMO:
			retval = internal_ibtmo( conf, value );
			if( retval < 0 ) return exit_library( ud, 1 );
			return exit_library( ud, 0 );
			break;
		case IbcEOT:
			internal_ibeot( conf, value );
			return exit_library( ud, 0 );
			break;
		case IbcEOSrd:
			if( value )
				conf->eos_flags |= EOS_RD;
			else
				conf->eos_flags &= ~EOS_RD;
			return exit_library( ud, 0 );
			break;
		case IbcEOSwrt:
			if( value )
				conf->eos_flags |= EOS_EOI;
			else
				conf->eos_flags &= ~EOS_EOI;
			return exit_library( ud, 0 );
			break;
		case IbcEOScmp:
			if( value )
				conf->eos_flags |= EOS_BIN;
			else
				conf->eos_flags &= ~EOS_BIN;
			return exit_library( ud, 0 );
			break;
		case IbcEOSchar:
			if( ( value & 0xff ) != value )
			{
				setIberr( EARG );
				return exit_library( ud, 1 );
			}
			conf->eos = value;
			return exit_library( ud, 0 );
			break;
		default:
			break;
	}

	if( conf->is_interface )
	{
		switch( option )
		{
			case IbcPPC:
				retval = internal_ibppc( conf, value );
				if( retval < 0 ) return exit_library( ud, 1 );
				return exit_library( ud, 0 );
				break;
			case IbcAUTOPOLL:
				retval = configure_autopoll( conf, value );
				if( retval < 0 ) return exit_library( ud, 1 );
				return exit_library( ud, 0 );
				break;
			case IbcCICPROT:
				// XXX
				if( value )
				{
					fprintf( stderr, "libgpib: pass control protocol not supported\n");
					setIberr( ECAP );
					return exit_library( ud, 1 );
				}else
					return exit_library( ud, 0 );
				break;
			case IbcIRQ:
				// XXX
				if( value == 0 )
				{
					fprintf( stderr, "libgpib: disabling interrupts not supported\n");
					setIberr( ECAP );
					return exit_library( ud, 1 );
				}else
					return exit_library( ud, 0 );
				break;
			case IbcSC:
				// XXX
				fprintf( stderr, "libgpib: request/release control protocol not supported\n");
				return exit_library( ud, 1 );
				break;
			case IbcSRE:
				retval = internal_ibsre( conf, value );
				if( retval < 0 ) return exit_library( ud, 1 );
				return exit_library( ud, 0 );
				break;
			default:
				break;
		}
	}else
	{
		switch( option )
		{
			case IbcREADDR:
				/* We always re-address.  To support only
				 * readdressing when necessary would require
				 * making the driver keep track of current addressing
				 * state.  Maybe someday, but low priority. */
				if( value )
					conf->readdr = 1;
				else
					conf->readdr = 0;
				return exit_library( ud, 0 );
				break;
			default:
				break;
		}
	}

	setIberr( EARG );
	return exit_library( ud, 1 );
}
