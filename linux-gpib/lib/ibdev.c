/***************************************************************************
                                 ibdev.c
                             -------------------
    begin                : Tues Feb 12 2002
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

#include "ib_internal.h"
#include <ibP.h>

static int config_parsed = 0;

int ibdev(int minor, int pad, int sad, int timo, int eot, int eos)
{
	int descriptor;
	int index;
	char *envptr;
	int retval;
	int uDesc;
	ibConf_t conf;

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

	conf.
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
