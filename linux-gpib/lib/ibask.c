/***************************************************************************
                          lib/ibask.c
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
#include <ibP.h>

int query_ppc( const ibBoard_t *board )
{
	int retval;
	int ppc;

	retval = ioctl( board->fileno, IBQUERY_PPC, &ppc );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	return ppc;
}

int query_autopoll( const ibBoard_t *board )
{
	int retval;
	int autopolling;

	retval = ioctl( board->fileno, IBQUERY_AUTOPOLL, &autopolling );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	return autopolling;
}

int query_board_rsv( const ibBoard_t *board )
{
	int retval;
	int status;

	retval = ioctl( board->fileno, IBQUERY_BOARD_RSV, &status );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	return status;
}

int ibask( int ud, int option, int *value )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	switch( option )
	{
		case IbaPAD:
			*value = conf->pad;
			return exit_library( ud, 0 );
			break;
		case IbaSAD:
			if( conf->sad < 0 ) *value = 0;
			else *value = MSA( conf->sad );
			return exit_library( ud, 0 );
			break;
		case IbaTMO:
			*value = usec_to_timeout( conf->usec_timeout );
			return exit_library( ud, 0 );
			break;
		case IbaEOT:
			*value = conf->send_eoi;
			return exit_library( ud, 0 );
			break;
		case IbaEOSrd:
			*value = conf->eos_flags & EOS_RD;
			return exit_library( ud, 0 );
			break;
		case IbaEOSwrt:
			*value = conf->eos_flags & EOS_EOI;
			return exit_library( ud, 0 );
			break;
		case IbaEOScmp:
			*value = conf->eos_flags & EOS_BIN;
			return exit_library( ud, 0 );
			break;
		case IbaEOSchar:
			*value = conf->eos;
			return exit_library( ud, 0 );
			break;
		case IbaReadAdjust:
			/* XXX I guess I could implement byte swapping stuff,
			 * it's pretty stupid though */
			*value = 0;
			return exit_library( ud, 0 );
			break;
		case IbaWriteAdjust:
			/* XXX I guess I could implement byte swapping stuff,
			 * it's pretty stupid though */
			*value = 0;
			return exit_library( ud, 0 );
			break;
		case IbaEndBitIsNormal:
			/* XXX no support for setting END status on EOI only yet */
			*value = 1;
			return exit_library( ud, 0 );
		default:
			break;
	}

	if( conf->is_interface )
	{
		switch( option )
		{
			case IbaPPC:
				retval = query_ppc( board );
				if( retval < 0 ) return exit_library( ud, 1 );
				*value = retval;
				return exit_library( ud, 0 );
			case IbaAUTOPOLL:
				retval = query_autopoll( board );
				if( retval < 0 ) return exit_library( ud, 1 );
				*value = retval;
				return exit_library( ud, 0 );
			case IbaCICPROT:
				// XXX we don't support pass control protocol yet
				*value = 0;
				return exit_library( ud, 0 );
				break;
			case IbaIRQ:
				// XXX we don't support interrupt-less operation yet
				*value = 0;
				return exit_library( ud, 0 );
				break;
			case IbaSC:
				*value = board->is_system_controller;
				return exit_library( ud, 0 );
				break;
			case IbaSRE:
				/* XXX pretty worthless, until changing
				 * system controllers is supported */
				*value = 1;
				return exit_library( ud, 0 );
				break;
			case IbaPP2:
				*value = conf->local_ppc;
				return exit_library( ud, 0 );
				break;
			case IbaTIMING:
				// XXX we don't support changing bus timings yet
				*value = 1;
				return exit_library( ud, 0 );
				break;
			case IbaDMA:
				// XXX bogus, but pretty unimportant
				*value = board->dma;
				return exit_library( ud, 0 );
				break;
			case IbaEventQueue:
				// XXX no event queue yet
				*value = 0;
				return exit_library( ud, 0 );
			case IbaSPollBit:
				// XXX no support for SPOLL status yet
				*value = 0;
				return exit_library( ud, 0 );
			case IbaSendLLO:
				*value = conf->local_lockout;
				return exit_library( ud, 0 );
			case IbaPPollTime:
				*value = usec_to_timeout( conf->ppoll_usec_timeout );
				return exit_library( ud, 0 );
			case IbaHSCableLength:
				/* HS transfer not supported and may never
				 * be as it is not part of GPIB standard */
				*value = 0;
				return exit_library( ud, 0 );
			case IbaIst:
				// XXX ist support not implemented yet
				*value = 0;
				return exit_library( ud, 0 );
			case IbaRsv:
				retval = query_board_rsv( board );
				if( retval < 0 ) return exit_library( ud, 1 );
				*value = retval;
				return exit_library( ud, 0 );
			default:
				break;
		}
	}else
	{
		switch( option )
		{
			case IbaREADDR:
				/* We always re-address.  To support only
				 * readdressing when necessary would require
				 * making the driver keep track of current addressing
				 * state.  Maybe someday, but low priority. */
				*value = 1;
				return exit_library( ud, 0 );
				break;
			case IbaSPollTime:
				*value = usec_to_timeout( conf->spoll_usec_timeout );
				return exit_library( ud, 0 );
				break;
			case IbaUnAddr:
				/* XXX sending UNT and UNL after device level read/write
				 * not supported yet, I suppose it could be since it
				 * is harmless. */
				*value = 0;
				return exit_library( ud, 0 );
				break;
			case IbaBNA:
				*value = conf->board;
				return exit_library( ud, 0 );
				break;
			default:
				break;
		}
	}

	setIberr( EARG );

	return exit_library( ud, 1 );
}


