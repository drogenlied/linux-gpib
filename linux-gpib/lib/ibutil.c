
#include <ib.h>
#include <ibP.h>
#include <stdio.h>



PRIVATE int ibParseConfigFile(char *filename)
{

extern FILE *gpib_yyin;
FILE *infile;
int stat=1;

/*printf("open %s\n",filename);fflush(stdout);*/

if ((infile=fopen(filename,"r"))==NULL){

  iberr = ECFG;
  return(-1);
 }

/*ibPutMsg("Parse Config %s",filename); */
gpib_yyin=infile;
gpib_yyrestart(gpib_yyin); /* Hmm ? */
if(gpib_yyparse()<0) stat=-1 ;

fclose(infile);

return stat;
}

/**********************************************************************/


char ib_ndev=0;

PRIVATE ibInstallConfigItem(ibConf_t *p)
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

        strcpy( ibConfigs[ib_ndev].name, p->name );
	/*ibConfigs[ib_ndev].board = p->board;*/
	ibConfigs[ib_ndev].padsad = p->padsad;
	/*ibConfigs[ib_ndev].dcl = p->dcl;*/
	ibConfigs[ib_ndev].flags = p->flags;
	ibConfigs[ib_ndev].eos = p->eos;
	ibConfigs[ib_ndev].eosflags = p->eosflags;
	ibConfigs[ib_ndev].tmo = p->tmo;


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

register i;

for(i=0;i<IB_MAXDEV;i++){
if(! strcmp(ibConfigs[i].name,name)) return(i);
}

return(-1);
}









