/***************************************************************************
                              lib/ib_internal.h
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

#ifndef _IB_INTERNAL_H
#define _IB_INTERNAL_H

#include <autoconf.h>

#include "ib.h"
#include "ibP.h"
#include "gpib_types.h"
#include "gpib_registers.h"
#include "gpib_ioctl.h"
#include <unistd.h>
#include <sys/ioctl.h>

#include "ibConf.h"

enum internal_gpib_addr
{
	SAD_DISABLED = -1,
	ADDR_INVALID = -2
};

extern void init_async_op( struct async_operation *async );
extern int ibCheckDescriptor(int ud);
extern  int ibBdChrConfig( ibBoard_t *board );
extern  void ibBoardDefaultValues(void);
extern int ibBoardOpen( ibBoard_t *board );
extern  int ibBoardClose( ibBoard_t *board );
extern  int ibGetNrBoards(void);
extern  void yyerror(char *s);
extern  int iblcleos( const ibConf_t *conf );
extern  char *ibVerbCode(int code);
extern  void ibPutMsg (char *format,...);
extern  void ibPutErrlog(int ud,char *routine);
extern  int ibParseConfigFile(char *filename);
extern  int ibGetDescriptor(ibConf_t conf);
extern  int ibFindDevIndex(char *name);
extern ssize_t my_ibcmd( ibConf_t *conf, uint8_t *buffer, size_t length);
extern int send_setup_string( const ibConf_t *conf, uint8_t *cmdString );
extern unsigned int create_send_setup( const ibBoard_t *board,
	Addr4882_t addressList[], uint8_t *cmdString );
extern int send_setup( ibConf_t *conf );
extern void init_ibconf( ibConf_t *conf );
extern int my_ibdev( int minor, int pad, int sad, unsigned int usec_timeout,
	int send_eoi, int eos, int eosflags);
extern unsigned int timeout_to_usec( enum gpib_timeout timeout );
extern unsigned int ppoll_timeout_to_usec( unsigned int timeout );
extern unsigned int usec_to_ppoll_timeout( unsigned int usec );
extern int set_timeout( const ibBoard_t *board, unsigned int usec_timeout );
extern int ib_lock_mutex( ibBoard_t *board );
extern int ib_unlock_mutex( ibBoard_t *board );
extern int close_gpib_device( ibConf_t *conf );
extern int open_gpib_device( ibConf_t *conf );
extern int gpibi_change_address( ibConf_t *conf,
	unsigned int pad, int sad );
extern int lock_board_mutex( ibBoard_t *board );
extern int unlock_board_mutex( ibBoard_t *board );
extern int exit_library( int ud, int error );
extern int general_exit_library( int ud, int error, int keep_lock );
extern ibConf_t * enter_library( int ud );
extern ibConf_t * general_enter_library( int ud, int no_lock_board, int ignore_eoip );
extern void setIbsta( int status );
extern void setIberr( int error );
extern void setIbcnt( long count );
extern unsigned int usec_to_timeout( unsigned int usec );
extern int query_ppc( const ibBoard_t *board );
extern int conf_online( ibConf_t *conf, int online );
extern int configure_autopoll( ibConf_t *conf, int enable );
extern int extractPAD( Addr4882_t address );
extern int extractSAD( Addr4882_t address );
extern Addr4882_t packAddress( unsigned int pad, int sad );
extern int addressIsValid( Addr4882_t address );
extern int addressListIsValid( Addr4882_t addressList[] );
extern unsigned int numAddresses( Addr4882_t addressList[] );
extern int remote_enable( const ibBoard_t *board, int enable );
extern int config_read_eos( ibBoard_t *board, int use_eos_char,
	int eos_char, int compare_8_bits );

extern int internal_ibpad( ibConf_t *conf, unsigned int address );
extern int internal_ibsad( ibConf_t *conf, int address );
extern int internal_ibtmo( ibConf_t *conf, int timeout );
extern void internal_ibeot( ibConf_t *conf, int send_eoi );
extern int internal_ibppc( ibConf_t *conf, int v );
extern int internal_ibsre( ibConf_t *conf, int v );
extern int internal_ibrsv( ibConf_t *conf, int v );
extern int internal_iblines( ibConf_t *conf, short *line_status );
extern int internal_ibgts( ibConf_t *conf, int shadow_handshake );
extern int internal_ibsic( ibConf_t *conf );
extern int InternalDevClearList( ibConf_t *conf, Addr4882_t addressList[] );

static __inline__ ibBoard_t* interfaceBoard( const ibConf_t *conf )
{
	return &ibBoard[ conf->board ];
}

#include <stdio.h>
int gpib_yyparse(void);
void gpib_yyrestart(FILE*);
int gpib_yylex(void);

#endif	/* _IB_INTERNAL_H */
