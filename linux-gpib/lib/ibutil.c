
#include <ib.h>
#include <ibP.h>
#include <stdio.h>
#include "parse.h"


PRIVATE int ibParseConfigFile(char *filename)
{

extern FILE *gpib_yyin;
FILE *infile;
int stat=1;

if ((infile=fopen(filename,"r"))==NULL){

  iberr = ECFG;
  return(-1);
 }

/*ibPutMsg("Parse Config %s",filename); */
gpib_yyin=infile;
gpib_yyrestart(gpib_yyin); /* Hmm ? */
// initialize line counter to 1 before calling yyparse
yylloc.first_line = 1;
if(gpib_yyparse()<0) stat=-1 ;

fclose(infile);

return stat;
}

/**********************************************************************/


int ib_ndev=0;

PRIVATE int ibInstallConfigItem(ibConf_t *p)
{

/* check validity of values */

	if( (p->padsad & 0xff) > IB_MAXDEV ){
		iberr = ETAB;
		return(ERR);
	}

        if( ib_ndev >= IB_MAXDEV ){
		iberr = ETAB;
		return(ERR);
	}
	
/* put entry to the table */

printf("dev %i\n", ib_ndev);
        strcpy( ibConfigs[ib_ndev].name, p->name );
printf("name %s\n", p->name);
	ibConfigs[ib_ndev].board = p->board;
printf("minor %i\n", p->board);
	ibConfigs[ib_ndev].padsad = p->padsad;
printf("padsad %i\n", p->padsad);
	ibConfigs[ib_ndev].flags = p->flags;
printf("flags 0x%x\n", p->flags);
	ibConfigs[ib_ndev].eos = p->eos;
printf("eos %i\n", p->eos);
	ibConfigs[ib_ndev].eosflags = p->eosflags;
printf("eos flags 0x%x\n", p->eosflags);
	ibConfigs[ib_ndev].tmo = p->tmo;
printf("%i\n", p->tmo);


	ibConfigs[ib_ndev].networkdb = p->networkdb;

	
        strcpy( ibConfigs[ib_ndev].init_string, p->init_string );

	ib_ndev++;
	return (ib_ndev);
}


PRIVATE int ibGetNrDev(VOID)
{
return ib_ndev;
}

/**********************************************************************/
PRIVATE int ibFindDevIndex(char *name)
{

register int i;

for(i=0;i<IB_MAXDEV;i++){
if(! strcmp(ibConfigs[i].name,name)) return(i);
}

return(-1);
}









