#include <gpib/ib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include <stdlib.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

MODULE = LinuxGpib		PACKAGE = LinuxGpib		


double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL


int
ibcac(ud, v)
	int	ud
	int	v

int
ibclr(ud)
	int	ud

int
ibcmd(ud, cmd, cnt)
	int	ud
	char *	cmd
	unsigned long	cnt

int
ibconfig(ud, option, value)
	int	ud
	int	option
	int	value

int
ibdev(minor, pad, sad, timo, eot, eos)
	int	minor
	int	pad
	int	sad
	int	timo
	int	eot
	int	eos

int
ibdma(ud, v)
	int	ud
	int	v

int
ibeot(ud, v)
	int	ud
	int	v

int
ibevent(ud, event)
	int	ud
	short *	event

int
ibfind(dev)
	char *	dev

int
ibgts(ud, v)
	int	ud
	int	v

int
iblines(ud, buf)
	int	ud
	unsigned short *	buf

int
ibloc(ud)
	int	ud

int
ibonl(ud, onl)
	int	ud
	int	onl

int
ibpad(ud, v)
	int	ud
	int	v

int
ibrd(ud, rd, cnt)
	int	ud
	SV  *rd
	unsigned long	cnt
CODE:
	int i;
	STRLEN len;
	char *buf;

	buf = malloc( cnt + 1 );
	if( buf )
	{
		for(i = 0; i <= cnt; i++) { buf[i] = 0; }
		RETVAL = ibrd(ud, buf, cnt);
		sv_setpvn(rd, buf, cnt + 1);
		free( buf );
	}else
		RETVAL = ERR;

int
ibrpp(ud, ppr)
	int	ud
	char *	ppr

int
ibrsp(ud, spr)
	int	ud
	char *	spr

int
ibrsv(ud, v)
	int	ud
	int	v

int
ibsad(ud, v)
	int	ud
	int	v

int
ibsic(ud)
	int	ud

int
ibsre(ud, v)
	int	ud
	int	v

int
ibtmo(ud, v)
	int	ud
	int	v

int
ibtrg(ud)
	int	ud

int
ibwait(ud, mask)
	int	ud
	int	mask

int
ibwrt(ud, rd, cnt)
	int	ud
	char *	rd
	unsigned long	cnt
