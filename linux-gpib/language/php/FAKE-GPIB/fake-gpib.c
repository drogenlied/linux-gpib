/* -*- linux-c -*- 

   A fake GPIB library.

   Was Needed to develop the GPIB PHP-entension on a computer
   without any GPIB hardware.

written by 
	Michel Billaud, billaud@labri.fr, 
	Laboratoire Bordelais de Recherche en Informatique
	Université Bordeaux 1 - France

for 
	the Minerva "Emerge" project, in February 2003
	(C) Michel Billaud, 2003

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

STATUS: alpha code.

LIMITS: Only ib* command are considered for the moment.

CREDITS: 
The list of variable and function names was borrowed from
the linux-gpib-3.1.92 files. http://linux-gpib.sourceforge.net

*/


#include "ib.h"


volatile int ibsta, ibcnt, iberr;
volatile long ibcntl;

#ifdef WITHALLFUNCTIONS
void AllSPoll( int boardID, Addr4882_t addressList[], short resultList[] )
{
	printf("fake-gpib: calling ALLSPoll\n");
}

void AllSpoll( int boardID, Addr4882_t addressList[], short resultList[] )
{
	printf("fake-gpib: calling AllSpoll\n");
}

void DevClear( int boardID, Addr4882_t address )
{
	printf("fake-gpib: calling DevClear\n");
}

void DevClearList( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling DevClearList\n");
}

void EnableLocal( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling EnableLocal\n");
}

void EnableRemote( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling EnableRemote\n");
}

void FindLstn( int boardID, Addr4882_t padList[],
	       Addr4882_t resultList[], int maxNumResults )
{
	printf("fake-gpib: calling FindLstn\n");
}

void FindRQS( int boardID, Addr4882_t addressList[], short *result )
{
	printf("fake-gpib: calling FindRQS\n");
}

void PassControl( int boardID, Addr4882_t address )
{
	printf("fake-gpib: calling PassControl\n");
}

void PPoll( int boardID, short *result )
{
	printf("fake-gpib: calling PPoll\n");
}

void PPollConfig( int boardID, Addr4882_t address, int dataLine, int lineSense )
{
	printf("fake-gpib: calling PPollConfig\n");
}

void PPollUnconfig( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling PPollUnconfig\n");
}

void RcvRespMsg( int boardID, void *buffer, long count, int termination )
{
	printf("fake-gpib: calling RcvRespMsg\n");
}

void ReadStatusByte( int boardID, Addr4882_t address, short *result )
{
	printf("fake-gpib: calling ReadStatusByte\n");
}

void Receive( int boardID, Addr4882_t address,
	      void *buffer, long count, int termination )
{
	printf("fake-gpib: calling Receive\n");
}

void ReceiveSetup( int boardID, Addr4882_t address )
{
	printf("fake-gpib: calling ReceiveSetup\n");
}

void ResetSys( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling ResetSys\n");
}

void Send( int boardID, Addr4882_t address, void *buffer,
	   long count, int eotmode )
{
	printf("fake-gpib: calling Send\n");
}

void SendCmds( int boardID, void *buffer, long count )
{
	printf("fake-gpib: calling SendCmds\n");
}

void SendDataBytes( int boardID, void *buffer,
		    long count, int eotmode )
{
	printf("fake-gpib: calling SendDataBytes\n");
}

void SendIFC( int boardID )
{
	printf("fake-gpib: calling SendIFC\n");
}

void SendLLO( int boardID )
{
	printf("fake-gpib: calling SendLLO\n");
}

void SendList( int boardID, Addr4882_t addressList[], void *buffer,
	       long count, int eotmode )
{
	printf("fake-gpib: calling SendList\n");
}

void SendSetup( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling SendSetup\n");
}

void SetRWLS( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling SetRWLS\n");
}

void TestSRQ( int boardID, short *result )
{
	printf("fake-gpib: calling TestSRQ\n");
}

void TestSys( int boardID, Addr4882_t addressList[],
	      short resultList[] )
{
	printf("fake-gpib: calling TestSys\n");
}

int ThreadIbsta( void )
{
	printf("fake-gpib: calling ThreadIbsta\n");
	return 0;
}

int ThreadIberr( void )
{
	printf("fake-gpib: calling ThreadIberr\n");
	return 0;
}

int ThreadIbcnt( void )
{
	printf("fake-gpib: calling ThreadIbcnt\n");
	return 0;
}

long ThreadIbcntl( void )
{
	printf("fake-gpib: calling ThreadIbcntl\n");
	return 0;
}

void Trigger( int boardID, Addr4882_t address )
{
	printf("fake-gpib: calling Trigger\n");
}

void TriggerList( int boardID, Addr4882_t addressList[] )
{
	printf("fake-gpib: calling TriggerList\n");
}

void WaitSRQ( int boardID, short *result )
{
	printf("fake-gpib: calling WaitSRQ\n");
}

#endif

int ibask( int ud, int option, int *value )
{
	printf("fake-gpib: calling ibask\n");
	return 0;
}

int ibbna( int ud, char *board_name )
{
	printf("fake-gpib: calling ibbna\n");
	return 0;
}

int ibcac( int ud, int v )
{
	printf("fake-gpib: calling ibcac\n");
	return 0;
}

int ibclr( int ud )
{
	printf("fake-gpib: calling ibclr\n");
	return 0;
}

int ibcmd( int ud, void *cmd, long cnt )
{
	printf("fake-gpib: calling ibcmd\n");
	return 0;
}

int ibcmda( int ud, void *cmd, long cnt )
{
	printf("fake-gpib: calling ibcmda\n");
	return 0;
}

int ibconfig( int ud, int option, int value )
{
	printf("fake-gpib: calling ibconfig\n");
	return 0;
}

int ibdev( int boardID, int pad, int sad, int timo, int eot, int eos )
{
	printf("fake-gpib: calling ibdev\n");
	return 0;
}

int ibdma( int ud, int v )
{
	printf("fake-gpib: calling ibdma\n");
	return 0;
}

int ibeot( int ud, int v )
{
	printf("fake-gpib: calling ibeot\n");
	return 0;
}

int ibeos( int ud, int v )
{
	printf("fake-gpib: calling ibeos\n");
	return 0;
}

int ibevent( int ud, short *event )
{
	printf("fake-gpib: calling ibevent\n");
	return 0;
}

int ibfind( const char *dev )
{
	printf("fake-gpib: calling ibfind\n");
	return 42;
}

int ibgts(int ud, int v)
{
	printf("fake-gpib: calling ibgts\n");
	return 0;
}

int iblines( int ud, short *line_status )
{
	printf("fake-gpib: calling iblines\n");
	return 0;
}

int ibloc( int ud )
{
	printf("fake-gpib: calling ibloc\n");
	return 0;
}

int ibonl( int ud, int onl )
{
	printf("fake-gpib: calling ibonl\n");
	return 0;
}

int ibpad( int ud, int v )
{
	printf("fake-gpib: calling ibpad\n");
	return 0;
}

int ibpct( int ud )
{
	printf("fake-gpib: calling ibpct\n");
	return 0;
}

int ibppc( int ud, int v )
{
	printf("fake-gpib: calling ibppc\n");
	return 0;
}

int ibrd( int ud, void *rd, long count )
{
	char *t = rd;
	int i;
	printf("fake-gpib: calling ibrd(%d,...,%d)\n",ud,count);
	for (i=0; i<count-1; i++)
		t[i] = 'A' +i;
	t[count-1]='\0';
	return 0;
}

int ibrdf( int ud, const char *file_path )
{
	printf("fake-gpib: calling ibrdf\n");
	return 0;
}

int ibrpp( int ud, char *ppr )
{
	printf("fake-gpib: calling ibrpp\n");
	*ppr = 42;
	return 0;
}

int ibrsc( int ud, int v )
{
	printf("fake-gpib: calling ibrsc\n");
	return 0;
}

int ibrsp( int ud, char *spr )
{
	printf("fake-gpib: calling ibrsp\n");
	return 0;
}

int ibrsv( int ud, int v )
{
	printf("fake-gpib: calling ibrsv\n");
	return 0;
}

int ibsad( int ud, int v )
{
	printf("fake-gpib: calling ibsad\n");
	return 0;
}

int ibsic( int ud )
{
	printf("fake-gpib: calling ibsic\n");
	return 0;
}

int ibsre( int ud, int v )
{
	printf("fake-gpib: calling ibsre\n");
	return 0;
}

int ibtmo( int ud,int v )
{
	printf("fake-gpib: calling ibtmo\n");
	return 0;
}

int ibtrg( int ud )
{
	printf("fake-gpib: calling ibtrg\n");
	return 0;
}

int ibwait( int ud, int mask )
{
	printf("fake-gpib: calling ibwait\n");
	return 0;
}

int ibwrt( int ud, void *rd, long count )
{
	printf("\nfake-gpib: calling ibwrt(%d,'%s',%ld)\n",ud,rd,count);
	return 0;
}

int ibwrtf( int ud, const char *file_path )
{
	printf("fake-gpib: calling ibwrtf\n");
	return 0;
}

const char* gpib_error_string( int iberr )
{
	return "a fake error string";
}
