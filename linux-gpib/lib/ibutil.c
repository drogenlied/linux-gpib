
#include <ib.h>
#include <ibP.h>
#include <stdio.h>
#include "parse.h"


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


int ib_ndev=0;

int ibInstallConfigItem(ibConf_t *p)
{
	/* check validity of values */
	if( (p->padsad & 0xff) > IB_MAXDEV )
	{
		iberr = ETAB;
		return(ERR);
	}

	if( ib_ndev >= IB_MAXDEV )
	{
		iberr = ETAB;
		return(ERR);
	}

	/* put entry to the table */

	strcpy( ibConfigs[ib_ndev].name, p->name );
	ibConfigs[ib_ndev].board = p->board;
	ibConfigs[ib_ndev].padsad = p->padsad;
	ibConfigs[ib_ndev].flags = p->flags;
	ibConfigs[ib_ndev].eos = p->eos;
	ibConfigs[ib_ndev].eosflags = p->eosflags;
	ibConfigs[ib_ndev].tmo = p->tmo;

	strcpy(ibConfigs[ib_ndev].init_string, p->init_string );

	ib_ndev++;
	return (ib_ndev);
}

int ibGetNrDev(VOID)
{
return ib_ndev;
}

/**********************************************************************/
int ibFindDevIndex(char *name)
{
	int i;

	for(i = 0; i < IB_MAXDEV; i++)
	{
		if(!strcmp(ibConfigs[i].name, name)) return i;
	}

	return -1;
}









