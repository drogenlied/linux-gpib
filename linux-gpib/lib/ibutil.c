
#include <ib.h>
#include <ibP.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

static int num_devices = 0;

int ibParseConfigFile(char *filename)
{
	extern FILE *gpib_yyin;
	FILE *infile;
	int stat=1;

	if ((infile=fopen(filename,"r"))==NULL)
	{
		iberr = ECFG;
		return -1;
	}

	/*ibPutMsg("Parse Config %s",filename); */
	gpib_yyin = infile;
	gpib_yyrestart(gpib_yyin); /* Hmm ? */
	// initialize line counter to 1 before calling yyparse
	yylloc.first_line = 1;
	if(gpib_yyparse()<0) stat=-1 ;

	fclose(infile);

	return stat;
}

/**********************************************************************/

int ibInstallConfigItem(ibConf_t *p)
{
	ibConf_t *conf;
	int ib_ndev;

	/* check validity of values */
	if( (p->padsad & 0xff) >= IB_MAXDEV )
	{
		iberr = ETAB;
		return -1;
	}
	// search for an unused descriptor
	for(ib_ndev = 0; ib_ndev < NUM_CONFIGS; ib_ndev++)
	{
		if(check_descriptor(ib_ndev) < 0)
		{
			ibConfigs[ib_ndev] = malloc(sizeof(ibConf_t));
			break;
		}
	}
	if( ib_ndev == NUM_CONFIGS)
	{
		iberr = ETAB;
		return -1;
	}
	conf = ibConfigs[ib_ndev];
	/* put entry to the table */
	strncpy(conf->name, p->name, sizeof(conf->name) );
	conf->board = p->board;
	conf->padsad = p->padsad;
	conf->flags = p->flags;
	conf->eos = p->eos;
	conf->eosflags = p->eosflags;
	conf->tmo = p->tmo;

	strncpy(conf->init_string, p->init_string, sizeof(conf->init_string));
	num_devices++;
	return ib_ndev;
}

int ibGetNrDev(void)
{
return num_devices;
}

/**********************************************************************/
int ibFindDevIndex(char *name)
{
	int i;

	for(i = 0; i < NUM_CONFIGS; i++)
	{
		if(check_descriptor(i) < 0) continue;
		if(!strcmp(ibConfigs[i]->name, name)) return i;
	}

	return -1;
}

int check_descriptor(int ud)
{
	int fd;
	ibConf_t *conf = ibConfigs[ud];

	if(ud >= NUM_CONFIGS) return -1;

	if(conf == NULL) return -1;
	fd = ibBoard[conf->board].fileno;
	if(fcntl(fd, F_GETFD) < 0)
	{
		free(conf);
		ibConfigs[ud] = NULL;
		num_devices--;
		return -1;
	}
	return 0;
}







