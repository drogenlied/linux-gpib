/*
 * rgpib_test.c
 * test module for RPC Remote GPIB features
 * 
 * 
 *
 */

#include <stdio.h>
#include <ib.h>

extern void gpiberr(char*);

int main(int argc,char **argv){


int dev;
char cmd[80];
char res[1024];

  if((dev = ibfind("localhost:voltmeter")) & ERR){
    gpiberr("ibfind err");
    exit(1);
  }


  printf("\nclear device..");
  if(ibclr(dev) & ERR){
    gpiberr("clr err");
    exit(1);
  }


  printf("\nsend nice string..");
  strcpy(cmd,"D1 Init_Device");
  if( ibwrt(dev,cmd,strlen(cmd)) & ERR ){
    gpiberr("wrt err");
    exit(1);
  }


  printf("\nsetup Device");
  strcpy(cmd,"D0 L0 Q1 T1 R2 A1 S0");
  if( ibwrt(dev,cmd,strlen(cmd)) & ERR ){
    gpiberr("wrt err");
    exit(1);
  }

  printf("\nWaiting for RQS...");fflush(stdout);
  if( ibwait(dev,TIMO|RQS) & (ERR |TIMO)){
    gpiberr("ibwait Error");
    /*exit(1);*/
  } else
  printf("OK GOT IT\n");fflush(stdout);


  printf("\nReading Value...");
  if( ibrd(dev,res,1023) & ERR){
    printf("\n Warning, error occured!");
    gpiberr("read error");
  }
  res[ibcnt]=0;
  printf("\nres='%s' cnt=%d\n",res,ibcnt);



  ibonl(dev,0);
	return 0;
}



gpiberr(char *msg) {




printf("%s\n",msg);

printf("ibsta=0x%x  <",ibsta);

if ( ibsta & ERR )  printf(" ERR");
if ( ibsta & TIMO ) printf(" TIMO");
if ( ibsta & END )  printf(" END");
if ( ibsta & SRQI ) printf(" SRQI");
/*if ( ibsta & RQS )  printf(" RQS");*/
if ( ibsta & CMPL ) printf(" CMPL");
/*if ( ibsta & LOK )  printf(" LOK");*/
/*if ( ibsta & REM )  printf(" REM");*/
if ( ibsta & CIC )  printf(" CIC");
if ( ibsta & ATN )  printf(" ATM");
if ( ibsta & TACS ) printf(" TACS");
if ( ibsta & LACS ) printf(" LACS");
/*if ( ibsta & DTAS ) printf(" DATS");*/
/*if ( ibsta & DCAS ) printf(" DCTS");*/

printf(" >\n");

printf("iberr= %d", iberr);
if ( iberr == EDVR) printf(" EDVR <OS Error>\n");
if ( iberr == ECIC) printf(" ECIC <Not CIC>\n");
if ( iberr == ENOL) printf(" ENOL <No Listener>\n");
if ( iberr == EADR) printf(" EADR <Adress Error>\n");
if ( iberr == EARG) printf(" ECIC <Invalid Argument>\n");
if ( iberr == ESAC) printf(" ESAC <No Sys Ctrlr>\n");
if ( iberr == EABO) printf(" EABO <Operation Aborted>\n");
if ( iberr == ENEB) printf(" ENEB <No Gpib Board>\n");
if ( iberr == EOIP) printf(" EOIP <Async I/O in prg>\n");
if ( iberr == ECAP) printf(" ECAP <No Capability>\n");
if ( iberr == EFSO) printf(" EFSO <File sys. error>\n");
if ( iberr == EBUS) printf(" EBUS <Command error>\n");
if ( iberr == ESTB) printf(" ESTB <Status byte lost>\n");
if ( iberr == ESRQ) printf(" ESRQ <SRQ stuck on>\n");
if ( iberr == ETAB) printf(" ETAB <Table Overflow>\n");
if ( iberr == EPAR) printf(" EPAR <Parse Error in Config>\n");
if ( iberr == ECFG) printf(" ECFG <Can't open Config>\n");
if ( iberr == ETAB) printf(" ETAB <Device Table Overflow>\n");
if ( iberr == ENSD) printf(" ENSD <Configuration Error>\n");


printf("ibcnt= %d\n", ibcnt );
printf("\n");


/*ibonl(0,0);*/
}
