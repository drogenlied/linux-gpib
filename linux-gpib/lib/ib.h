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

extern int ibsta, ibcnt, iberr;
extern long ibcntl;
extern int ThreadIbsta( void );
extern int ThreadIberr( void );
extern int ThreadIbcnt( void );
extern long ThreadIbcntl( void );

/***** Public Functions ******/
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

#ifdef __cplusplus
}
#endif

#endif	// _PUBLIC_GPIB_H
