
#include "ib_internal.h"
#include <ibP.h>
#include <string.h>
#include <stdlib.h>

int ibfind(char *dev)
{
	int index;
	char *envptr;
	int retval;
	int uDesc;
	ibConf_t *conf;

	/* load config */

	if(config_parsed == 0)
	{
		envptr = getenv("IB_CONFIG");
		if(envptr)
			retval = ibParseConfigFile(envptr);
		else
			retval = ibParseConfigFile(DEFAULT_CONFIG_FILE);
		if(retval < 0)
		{
			ibsta |= ERR;
			ibPutErrlog(-1,"ibParseConfig");
			return -1;
		}
		config_parsed = 1;
	}

	if((index = ibFindDevIndex(dev)) < 0)
	{     /* find desired entry */
		iberr = ENSD;
		ibsta = ERR;
		ibPutErrlog(-1,"ibFindDevIndex");
		return -1;
	}

	conf = &ibFindConfigs[index];

	uDesc = my_ibdev( conf->board, conf->pad, conf->sad, conf->usec_timeout,
		conf->send_eoi, conf->eos, conf->eosflags );
	if(uDesc < 0)
	{
		fprintf(stderr, "ibfind failed to get descriptor\n");
		return -1;
	}
	conf = ibConfigs[uDesc];

	if(conf->flags & CN_SDCL)
	{
		ibPutMsg("CLR ");
		if(ibclr(uDesc) & ERR ) return -1;
	}

	ibPutMsg("INIT: ");
	if(strcmp(conf->init_string, ""))
	{
		if(ibwrt(uDesc, conf->init_string, strlen(conf->init_string)) & ERR )
			return -1;
		ibPutMsg(conf->init_string);
	}

	return uDesc;
}











