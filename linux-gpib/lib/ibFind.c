
#include <ib.h>
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

	if(dev == NULL) index = 0;	// XXX temporary hack to get ibchk working
	else if((index = ibFindDevIndex(dev)) < 0)
	{     /* find desired entry */
		iberr = ENSD;
		ibsta = ERR;
		ibPutErrlog(-1,"ibFindDevIndex");
		return -1;
	}

	uDesc = ibGetDescriptor(&ibFindConfigs[index]);
	if(uDesc < 0)
	{
		fprintf(stderr, "ibfind failed to get descriptor\n");
		return -1;
	}
	conf = ibConfigs[uDesc];

	if(ibBdChrConfig(uDesc) & ERR)
		return -1;

	if(ibonl(uDesc, 1) & ERR)
	{
		fprintf(stderr, "failed to bring device online\n");
		return -1;
	}

	if(ibsre(uDesc,1) & ERR ) return -1;

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











