#include <stdio.h>

#include <ib.h>
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
		ibBoard[i].padsad = 0;
		ibBoard[i].eos = '\n';
		ibBoard[i].eosflags = 0;
		ibBoard[i].base = 0;
		ibBoard[i].irq = 0;
		ibBoard[i].dma = 0;
		ibBoard[i].ifc = 0;
		ibBoard[i].fileno = 0;
		strcpy(ibBoard[i].device, "");
	}
}

/**********************/
int ibBoardOpen(int bd,int flags)
{
	int fd;

	if( ibBoard[bd].fileno == 0 )
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
		ibBoard[bd].fileno = 0;
	}
	return 0;
}

/**********************/

int ibBoardFunc (int bd, int code, ...)
{
	va_list ap;
	static int arg;
	static char *buf;
	static int cnt;

	if(check_descriptor(bd) < 0)
	{
		fprintf(stderr, "bad gpib descriptor\n");
		ibsta |= ERR;
		iberr = ENEB;
		return ibsta;
	}
	switch (code)
	{
		case IBRD:
		case DVRD:
		case IBWRT:
		case DVWRT:
		case IBAPE:
		case IBCMD:
			va_start(ap, code);
			arg=va_arg(ap, int);
			buf=va_arg(ap, char *);
			cnt=va_arg(ap, int);
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

	switch (code)
	{
		case IBRD:
		case DVRD:
			memset(buf, 0, cnt);
			break;
	}

	if( ibBoard[bd].fileno > 0 )
	{
		ibarg.ib_arg = arg;

		ibarg.ib_buf = (char *) buf;

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


// XXX global!
int ibGetNrBoards(void)
{
	extern int bdid;

	return bdid + 1;
}





