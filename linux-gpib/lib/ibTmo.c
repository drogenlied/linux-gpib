
#include <ib.h>
#include <ibP.h>

int ibtmo(int ud,int v)
{
	ibConf_t *conf = ibConfigs[ud];

	if ((v < TNONE) || (v > T1000s) || ibCheckDescriptor(ud) < 0)
	{
		ibsta = CMPL | ERR;
		iberr = EARG;
		return ibsta;
	}

	conf->tmo = v;

	ibsta = CMPL;
	return ibsta;
}


