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
#include <gpib_user.h>

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

extern volatile int ibsta, ibcnt, iberr;
extern volatile long ibcntl;

extern void AllSPoll( int boardID, Addr4882_t addressList[], short resultList[] );
extern void AllSpoll( int boardID, Addr4882_t addressList[], short resultList[] );
extern void DevClear( int boardID, Addr4882_t address );
extern void DevClearList( int boardID, Addr4882_t addressList[] );
extern void EnableLocal( int boardID, Addr4882_t addressList[] );
extern void EnableRemote( int boardID, Addr4882_t addressList[] );
extern void FindLstn( int boardID, Addr4882_t padList[],
	Addr4882_t resultList[], int maxNumResults );
extern void FindRQS( int boardID, Addr4882_t addressList[], short *result );
extern void PPoll( int boardID, short *result );
extern void PPollConfig( int boardID, Addr4882_t address, int dataLine, int lineSense );
extern void PPollUnconfig( int boardID, Addr4882_t addressList[] );
extern void RcvRespMsg( int boardID, void *buffer, long count, int termination );
extern void ReadStatusByte( int boardID, Addr4882_t address, short *result );
extern void Receive( int boardID, Addr4882_t address,
	void *buffer, long count, int termination );
extern void ReceiveSetup( int boardID, Addr4882_t address );
extern void ResetSys( int boardID, Addr4882_t addressList[] );
extern void Send( int boardID, Addr4882_t address, void *buffer,
	long count, int eotmode );
extern void SendCmds( int boardID, void *buffer, long count );
extern void SendDataBytes( int boardID, void *buffer,
	long count, int eotmode );
extern void SendIFC( int boardID );
extern void SendLLO( int boardID );
extern void SendList( int boardID, Addr4882_t addressList[], void *buffer,
	long count, int eotmode );
extern void SendSetup( int boardID, Addr4882_t addressList[] );
extern void SetRWLS( int boardID, Addr4882_t addressList[] );
extern void TestSRQ( int boardID, short *result );
extern void TestSys( int boardID, Addr4882_t addressList[],
	short resultList[] );
extern int ThreadIbsta( void );
extern int ThreadIberr( void );
extern int ThreadIbcnt( void );
extern long ThreadIbcntl( void );
extern void Trigger( int boardID, Addr4882_t address );
extern void TriggerList( int boardID, Addr4882_t addressList[] );
extern void WaitSRQ( int boardID, short *result );
extern int ibask(int ud, int option, int *value );
extern int ibbna( int ud, char *board_name );
extern int ibcac(int ud, int v);
extern int ibclr(int ud);
extern int ibcmd( int ud, void *cmd, long cnt );
extern int ibcmda( int ud, void *cmd, long cnt );
extern int ibconfig( int ud, int option, int value );
extern int ibdev(int minor, int pad, int sad, int timo, int eot, int eos);
extern int ibdma( int ud, int v );
extern int ibeot(int ud, int v);
extern int ibeos(int ud, int v);
extern int ibevent(int ud, short *event);
extern int ibfind(char *dev);
extern int ibgts(int ud, int v);
extern int iblines( int ud, short *line_status );
extern int ibloc(int ud);
extern int ibonl(int ud, int onl);
extern int ibpad(int ud, int v);
extern int ibppc( int ud, int v );
extern int ibrd(int ud, void *rd, long count);
extern int ibrpp(int ud, char *ppr);
extern int ibrsp(int ud, char *spr);
extern int ibrsv(int ud, int v);
extern int ibsad(int ud, int v);
extern int ibsic(int ud);
extern int ibsre(int ud, int v);
extern int ibtmo(int ud,int v);
extern int ibtrg(int ud);
extern int ibwait(int ud, int mask);
extern int ibwrt(int ud, void *rd, long count);

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

#endif	// _PUBLIC_GPIB_H
