/***************************************************************************
                          ib.h  -  header file for gpib library
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

#ifndef _PUBLIC_GPIB_H
#define _PUBLIC_GPIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "gpib_user.h"

typedef uint16_t Addr4882_t;
static const Addr4882_t NOADDR = -1;

/* tells RcvRespMsg() to stop on EOI */
static const int STOPend = 0x100;

enum sad_special_address
{
	NO_SAD = 0,
	ALL_SAD = -1
};

enum send_eotmode
{
	NULLend,
	DABend,
	NLend
};

volatile int ibsta, ibcnt, iberr;
volatile long ibcntl;

void AllSPoll( int board_desc, Addr4882_t addressList[], short resultList[] );
void AllSpoll( int board_desc, Addr4882_t addressList[], short resultList[] );
void DevClear( int board_desc, Addr4882_t address );
void DevClearList( int board_desc, Addr4882_t addressList[] );
void EnableLocal( int board_desc, Addr4882_t addressList[] );
void EnableRemote( int board_desc, Addr4882_t addressList[] );
void FindLstn( int board_desc, Addr4882_t padList[],
	Addr4882_t resultList[], int maxNumResults );
void FindRQS( int board_desc, Addr4882_t addressList[], short *result );
void PassControl( int board_desc, Addr4882_t address );
void PPoll( int board_desc, short *result );
void PPollConfig( int board_desc, Addr4882_t address, int dataLine, int lineSense );
void PPollUnconfig( int board_desc, Addr4882_t addressList[] );
void RcvRespMsg( int board_desc, void *buffer, long count, int termination );
void ReadStatusByte( int board_desc, Addr4882_t address, short *result );
void Receive( int board_desc, Addr4882_t address,
	void *buffer, long count, int termination );
void ReceiveSetup( int board_desc, Addr4882_t address );
void ResetSys( int board_desc, Addr4882_t addressList[] );
void Send( int board_desc, Addr4882_t address, void *buffer,
	long count, int eotmode );
void SendCmds( int board_desc, void *buffer, long count );
void SendDataBytes( int board_desc, void *buffer,
	long count, int eotmode );
void SendIFC( int board_desc );
void SendLLO( int board_desc );
void SendList( int board_desc, Addr4882_t addressList[], void *buffer,
	long count, int eotmode );
void SendSetup( int board_desc, Addr4882_t addressList[] );
void SetRWLS( int board_desc, Addr4882_t addressList[] );
void TestSRQ( int board_desc, short *result );
void TestSys( int board_desc, Addr4882_t addressList[],
	short resultList[] );
int ThreadIbsta( void );
int ThreadIberr( void );
int ThreadIbcnt( void );
long ThreadIbcntl( void );
void Trigger( int board_desc, Addr4882_t address );
void TriggerList( int board_desc, Addr4882_t addressList[] );
void WaitSRQ( int board_desc, short *result );
int ibask( int ud, int option, int *value );
int ibbna( int ud, char *board_name );
int ibcac( int ud, int v );
int ibclr( int ud );
int ibcmd( int ud, void *cmd, long cnt );
int ibcmda( int ud, void *cmd, long cnt );
int ibconfig( int ud, int option, int value );
int ibdev( int board_index, int pad, int sad, int timo, int send_eoi, int eos );
int ibdma( int ud, int v );
int ibeot( int ud, int v );
int ibeos( int ud, int v );
int ibevent( int ud, short *event );
int ibfind( const char *dev );
int ibgts(int ud, int v);
int iblines( int ud, short *line_status );
int ibloc( int ud );
int ibonl( int ud, int onl );
int ibpad( int ud, int v );
int ibpct( int ud );
int ibppc( int ud, int v );
int ibrd( int ud, void *rd, long count );
int ibrdf( int ud, const char *file_path );
int ibrpp( int ud, char *ppr );
int ibrsc( int ud, int v );
int ibrsp( int ud, char *spr );
int ibrsv( int ud, int v );
int ibsad( int ud, int v );
int ibsic( int ud );
int ibsre( int ud, int v );
int ibtmo( int ud,int v );
int ibtrg( int ud );
int ibwait( int ud, int mask );
int ibwrt( int ud, void *rd, long count );
int ibwrtf( int ud, const char *file_path );
const char* gpib_error_string( int iberr );

static __inline__ Addr4882_t MakeAddr( unsigned int pad, unsigned int sad )
{
	Addr4882_t address;

	address = ( pad & 0xff );
	address |= ( sad << 8 ) & 0xff00;
	return address;
}

static __inline__ unsigned int GetPAD( Addr4882_t address )
{
	return address & 0xff;
}

static __inline__ unsigned int GetSAD( Addr4882_t address )
{
	return ( address >> 8 ) & 0xff;
}

#ifdef __cplusplus
}
#endif

#endif	/* _PUBLIC_GPIB_H */
