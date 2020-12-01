/***************************************************************************
                             findlisteners.c
                            ------------------

   An example program to list the devices connected to a board.

    copyright : (C) 2020 by Dave Penkler
    email     : dpenkler@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef unsigned char uint8_t;
#include <gpib/gpib_user.h>
#include <gpib/ib.h>
#include <getopt.h>

static int ud;
static int timeout; /* old timeoout */
static char* myProg;

static void myError(int erc, char * mess) {
  int sys_errno = ThreadIbcnt();
  fprintf(stderr,"%s: error: %s", myProg, mess);
  fprintf(stderr," - %s\n", gpib_error_string(erc));
  if (!erc) fprintf(stderr," system error: %s\n",strerror(sys_errno));
  exit(1);
}

int findListeners(char * board, int from, int to) {
  int ibsta, erc, pad, n = 0;
  short stat;
  int bpad;  /* board primary address */
  ud = ibfind(board);
  if (ud < 0) myError(ThreadIberr(), "Can't find board");
  ibask(ud, IbaPAD, &bpad);
  ibask(ud, IbaTMO, &timeout); /* Remember old timeout */
  ibtmo(ud, T30ms); /* Set a shortish timeout for now */
  for (pad=from; pad<=to; pad++) {
    if ( ibln(ud, pad, NO_SAD, &stat) & ERR  ) {
      ibsta = ThreadIbsta();
      erc = ThreadIberr();
      ibtmo(ud,timeout); /* Restore old timeout */
      if ((erc == ENOL) || (ibsta & TIMO)) { /* No listeners on the bus */
	return(0);
      } else {
	myError(erc, "unexpected error");
      }
    } else if (stat) {
      printf("%s: Listener at pad %2d", myProg, pad);
      if (pad == bpad) printf(" (board) ");
      else n++;
      printf("\n");
    }
  }
  ibtmo(ud,timeout); /* Restore old timeout */
  return n;
}

void usage(int ecode) {
  fprintf(stderr,"Usage: %s [-h] [-s <start pad>] [-e <end pad>] <board name>\n", myProg);
  if (ecode >= 0) exit( ecode ) ;
}

int main(int argc, char ** argv) {
  int n;
  char *board;
  int from = 0;
  int to = 30;
  char c;
  myProg = argv[0];
  
  while ((c = getopt (argc, argv, "s:e:h")) != -1) {
    switch (c)  {
    case 's': from = atoi(optarg); break;
    case 'e': to   = atoi(optarg); break;
    case 'h': usage(-1);
      fprintf(stderr,"  where <start pad>..<end pad> is the range of pads to scan, default 0..30\n");
      fprintf(stderr,"  and <board name> is the name of the board from gpib.conf\n");
      exit(0);
      break;
    default: usage(1);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "%s: Expected board name\n", myProg);
    usage(1);
  }

  if ((from < 0) || (from > to)) {
    fprintf(stderr,"%s: Invalid start address. Expected 0 <= start <= end.\n", myProg);
    exit(1);
  }

  if ((to < from) || (to > 30)) {
    fprintf(stderr,"%s: Invalid end address. Expected start <= end <= 30.\n", myProg);
    exit(1);
  }
  
  board = argv[optind];

  printf("%s: Scanning pads %d to %d on board \"%s\"\n", myProg, from, to, board);
  
  n = findListeners(board, from, to);
  printf("%s: %d device%s found.\n",myProg, n, (n==1) ? "" : "s");
  exit(0);
}
