
#include <stdio.h>

#include "ib_internal.h"
#include <ibP.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <string.h>

int iberr = 0;
int ibsta = 0;
int ibcnt = 0;

ibBoard_t ibBoard[MAX_BOARDS];

void ibBoardDefaultValues(void)
{
	int i;
	for(i = 0; i < MAX_BOARDS; i++)
	{
		ibBoard[i].pad = 0;
		ibBoard[i].sad = -1;
		ibBoard[i].base = 0;
		ibBoard[i].irq = 0;
		ibBoard[i].dma = 0;
		ibBoard[i].is_system_controller = 0;
		ibBoard[i].fileno = -1;
		strcpy(ibBoard[i].device, "");
		strcpy(ibBoard[i].board_type, "");
	}
}

/**********************/
int ibBoardOpen( int bd, int flags )
{
	int fd;

	if( ibBoard[bd].fileno < 0 )
	{
		if((fd = open(ibBoard[bd].device, O_RDWR | flags)) < 0 )
		{
			ibsta =  ERR;
			iberr = EDVR;
			ibcnt = errno;
			ibPutErrlog(-1,"ibBoardOpen");
			return ERR;
		}
		ibBoard[bd].fileno = fd;
	}
	return 0;
}

/**********************/
int ibBoardClose(int bd)
{
	if(ibBoard[bd].fileno > 0)
	{
		close(ibBoard[bd].fileno);
		ibBoard[bd].fileno = -1;
	}
	return 0;
}

/**********************/







