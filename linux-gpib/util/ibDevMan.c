#include <ib.h>
#include <ibP.h>
#include <stdio.h>

static char *ibTimeouts[] = {
	"none",
	"10us",
	"30us",
	"100us",
	"300us",
	"1ms",
	"3ms",
	"10ms",
	"30ms",
	"100ms",
	"300ms",
	"1s",
	"3s",
	"10s",
	"30s",
	"100s",
	"300s",
	"1000s"
};

static char *yesno[] =
{
	"no",
	"yes"
};

int ibDumpConfiguration(int format,char *filename)
{
	FILE *outfile;
	int bd=0,ud;
	int i=0;

	if( filename == NULL )
		outfile = stdout;
	else if((outfile=fopen(filename,"w")) == NULL )
	{
		ibPutMsg("Can't Open file %s to dump configuration",filename );
		return ERR;
	}
	switch (format)
	{
		/*======================================================================*/
		case IB_DUMP_MKDEV:
			fprintf(outfile,"#!/bin/sh \n");
			fprintf(outfile,"# Automatic device inode generation (Dumped from Configuration) \n");
			fprintf(outfile,"device_dir=/dev \n");
			fprintf(outfile,"device_major=%d \n",IBMAJOR);
			fprintf(outfile,"if [ -d $device_dir/gpib%d ]; then\n",bd);
			fprintf(outfile,"  mkdir $device_dir/gpib%d \n",bd);
			fprintf(outfile,"fi \n");
			for(bd = 0; bd < ibGetNrBoards(); bd++)
			{
				fprintf(outfile,"mknod $device_dir/gpib%d c $device_major 0 \n", bd);
			}
			break;

		case IB_DUMP_TCL:
			fprintf(outfile,"# TCL script dumped with ibdump \n\n");
			fprintf(outfile,"set ibBoard(boards) %d \n\n",ibGetNrBoards());

			for(bd = 0; bd < ibGetNrBoards(); bd++)
			{
				/* dump board information */
				fprintf(outfile,"set ibBoard(%d,devices) { ",bd);
				for(ud = 0; ud < ibGetNrDev(); ud++)
				{
					if( CONF(ud,board) == bd )
					{ /* only current board */
						fprintf(outfile,"%s ",ibConfigs[ud]->name);
					}
				}
				fprintf(outfile,"}\n");
				fprintf(outfile,"set ibBoard(%d,pad) %d\n"     ,bd,ibBoard[bd].padsad & 0xff );
				fprintf(outfile,"set ibBoard(%d,sad) %d\n"     ,bd,(ibBoard[bd].padsad >> 8) & 0xff );
				fprintf(outfile,"set ibBoard(%d,timeout) %s\n" ,bd,ibTimeouts[ibBoard[bd].timeout] );

				fprintf(outfile,"set ibBoard(%d,base) 0x%x\n" ,bd,ibBoard[bd].base );
				fprintf(outfile,"set ibBoard(%d,irq)  %d\n" ,bd,ibBoard[bd].irq );
				fprintf(outfile,"set ibBoard(%d,dma)  %d\n" ,bd,ibBoard[bd].dma );

				fprintf(outfile,"set ibBoard(%d,eos) 0x%x\n" ,bd,ibBoard[bd].eos );
				fprintf(outfile,"set ibBoard(%d,reos) %d\n" ,bd,(ibBoard[bd].eosflags & REOS ? 1 : 0 ) );
				fprintf(outfile,"set ibBoard(%d,bin)  %d\n" ,bd,(ibBoard[bd].eosflags & BIN ? 1 : 0 ) );

				fprintf(outfile,"set ibBoard(%d,ifc)  %d\n" ,bd,ibBoard[bd].ifc  );
				/* dump devices information */
				for(ud = 0; ud < ibGetNrDev(); ud++)
				{
					if( CONF(ud,board) == bd )
					{ /* only current board */
						fprintf(outfile,"set %s(pad) %d\n", ibConfigs[ud]->name, CONF(ud,padsad) & 0xff );
						fprintf(outfile,"set %s(sad) %d\n", ibConfigs[ud]->name, (CONF(ud,padsad)>>8) & 0xff );
						if(strlen(ibConfigs[ud]->init_string) > 0 )
							fprintf(outfile,"set %s(init) %s \n",ibConfigs[ud]->name, ibConfigs[ud]->init_string );
						else
							fprintf(outfile,"set %s(init) \"\" \n",ibConfigs[ud]->name);

						fprintf(outfile,"set %s(eos) 0x%x\n",ibConfigs[ud]->name,CONF(ud,eos) );

						fprintf(outfile,"set %s(reos) %d\n",ibConfigs[ud]->name,( (CONF(ud,eosflags) & REOS) ? 1:0 ) );
						fprintf(outfile,"set %s(bin) %d \n",ibConfigs[ud]->name,( (CONF(ud,eosflags) & BIN)  ? 1:0) );


						fprintf(outfile,"set %s(init_flags) { ",ibConfigs[ud]->name);
						if( CONF(ud,flags) & CN_SDCL)
							fprintf(outfile,"DCL ");
						if ( CONF(ud,flags) & CN_SLLO)
							fprintf(outfile,"LLO ");
						if ( CONF(ud,flags) & CN_EXCLUSIVE)
							fprintf(outfile,"EXCL ");
						fprintf(outfile,"} \n");

						if( CONF(ud,flags) & CN_ISCNTL )
						{
							fprintf(outfile,"set %s(master) 1 \n",ibConfigs[ud]->name);
						}else
						{
							fprintf(outfile,"set %s(master) 0 \n",ibConfigs[ud]->name);
						}
						if( CONF(ud,flags) & CN_AUTOPOLL )
						{
							fprintf(outfile,"set %s(autopoll) 1 \n",ibConfigs[ud]->name);
						}else
						{
							fprintf(outfile,"set %s(autopoll) 0 \n",ibConfigs[ud]->name);
						}
					}
				}
			}
			break;
		default:
			case IB_DUMP_CONFIG:
			for(bd=0;bd<ibGetNrBoards();bd++)
			{
				fprintf(outfile,"/* Linux GPIB Configuration File (Autodumped from Local Configuration */\n");
				fprintf(outfile,"config { \n");  /* begin header */
				fprintf(outfile,"         pad     = %d \n",ibBoard[bd].padsad & 0xff );
				fprintf(outfile,"         sad     = %d \n",(ibBoard[bd].padsad >> 8) & 0xff );
				fprintf(outfile,"         timeout = %s \n",ibTimeouts[ibBoard[bd].timeout] );
				fprintf(outfile,"         base    = 0x%x \n",ibBoard[bd].base );
				fprintf(outfile,"         irq     = 0x%x \n",ibBoard[bd].irq  );
				fprintf(outfile,"         dma     = 0x%x \n",ibBoard[bd].dma  );

				fprintf(outfile,"         eos     = 0x%x \n",ibBoard[bd].eos );
				fprintf(outfile,"         set-reos= %s \n",((ibBoard[bd].eosflags & REOS) ? yesno[1] : yesno[0]) );
				fprintf(outfile,"         set-bin = %s \n",((ibBoard[bd].eosflags & BIN) ? yesno[1] : yesno[0]) );

				fprintf(outfile,"         set-ifc = %s \n",yesno[ibBoard[bd].ifc] );

				fprintf(outfile,"}\n");
				for(ud=0;ud<ibGetNrDev();ud++)
				{
					if( CONF(ud,board) == bd )
					{	/* only current board */
						fprintf(outfile,"device { name        = %s\n",ibConfigs[ud]->name ); /* begin header */
						fprintf(outfile,"         pad         = %d \n",CONF(ud,padsad) & 0xff );
						fprintf(outfile,"         sad         = %d \n",(CONF(ud,padsad)>>8) & 0xff );
						if(strlen(ibConfigs[ud]->init_string) > 0 )
							fprintf(outfile,"         init-string = %s \n", ibConfigs[ud]->init_string );

						if( CONF(ud,eos) )
							fprintf(outfile,"         eos         = 0x%x \n",CONF(ud,eos) );
						if( CONF(ud,eosflags) > 0 )
						{
							fprintf(outfile,"         set-reos    = %s \n",( (CONF(ud,eosflags) & REOS) ? yesno[1] : yesno[0]) );
							fprintf(outfile,"         set-bin     = %s \n",( (CONF(ud,eosflags) & BIN)  ? yesno[1] : yesno[0]) );
						}

#define COMMA if(i) fprintf(outfile,",")

						if ( CONF(ud,flags) & (CN_SDCL) ||  CONF(ud,flags) & (CN_SLLO)
							|| CONF(ud,flags) & (CN_EXCLUSIVE) )
						{
							fprintf(outfile,"         init-flags  = ");
							if( CONF(ud,flags) & (CN_SDCL))
							{
								COMMA;
								fprintf(outfile,"DCL ");
								i++;
							}
							if ( CONF(ud,flags) & (CN_SLLO))
							{
								COMMA;
								fprintf(outfile,"LLO ");
								i++;
							}
							if ( CONF(ud,flags) & (CN_EXCLUSIVE))
							{
								COMMA;
								fprintf(outfile,"EXCL ");
								i++;
							}
							fprintf(outfile,"\n");
						}
						if( CONF(ud,flags) & CN_ISCNTL )
						{
							fprintf(outfile,"         master \n");
						}
						if( CONF(ud,flags) & CN_AUTOPOLL )
						{
							fprintf(outfile,"         autopoll \n");
						}
						fprintf(outfile,"}\n");
					}
				}
			}
			break;
	}
	fclose(outfile);
	return 0;
}





