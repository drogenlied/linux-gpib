
#include <stdio.h>

#include "ib_internal.h"
#include <ibP.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int iberr = 0;
int ibsta = 0;
int ibcnt = 0;

ibarg_t ibarg = {0 };

ibBoard_t ibBoard[MAX_BOARDS];

void ibBoardDefaultValues(void)
{
	int i;
	for(i = 0; i < MAX_BOARDS; i++)
	{
		ibBoard[i].pad = 0;
		ibBoard[i].sad = -1;
		ibBoard[i].eos = '\n';
		ibBoard[i].eosflags = 0;
		ibBoard[i].base = 0;
		ibBoard[i].irq = 0;
		ibBoard[i].dma = 0;
		ibBoard[i].is_system_controller = 0;
		ibBoard[i].fileno = -1;
		strcpy(ibBoard[i].device, "");
	}
}

/**********************/
int ibBoardOpen(int bd,int flags)
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

int ibBoardFunc (int bd, int code, ...)
{
	va_list ap;
	static int arg;
	static uint8_t *buf;
	static int cnt;

	switch (code)
	{
		case IBAPE:
			va_start(ap, code);
			arg=va_arg(ap, int);
			buf=va_arg(ap, void*);
			cnt=va_arg(ap, unsigned long);
			va_end(ap);
			break;
		case IBAPRSP:
		case IBRPP:
		case DVRSP:
			va_start(ap, code);
			arg=va_arg(ap, int);
			buf=va_arg(ap, char *);
			buf[0]=0;
			va_end(ap);
			cnt=1;
			break;
		default:
			va_start(ap, code);
			arg=va_arg(ap, int);
			va_end(ap);
			buf=NULL;
			cnt=0;
			break;
	}

	if( ibBoard[bd].fileno > 0 )
	{
		ibarg.ib_arg = arg;

		ibarg.ib_buf =  buf;

		ibarg.ib_cnt = cnt;
		if( ioctl( ibBoard[bd].fileno, code, (ibarg_t *) &ibarg ) < 0)
		{
			ibsta = ERR;
			iberr = EDVR;
			ibcnt = errno;
		}else
		{
			ibsta = ibarg.ib_ibsta;
			iberr = ibarg.ib_iberr;
			ibcnt = ibarg.ib_ibcnt;
			ibPutErrlog(-1,ibVerbCode(code));
		}
	}else
	{
		ibsta = CMPL | ERR;
		iberr = ENEB;
		ibcnt = 0;
	}
	ibPutErrlog(-1,"ibBoardFunc");
	return ibsta;
}






