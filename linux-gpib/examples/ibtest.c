/*
 *  This Example demonstrates how normal device functions (with implicit 
 *  addressing) are used.
 *
 *  This Example makes use of the SRQ waiting feature
 *  if USE_SRQ is enabled.
 *
 */


/* here set your values to enable special features */


#define USE_SRQ 1
#define READS 5
#define WAIT 1


#include <stdio.h>
#include <ib.h>

extern int ibsta,ibcnt,iberr;

char spr;
char cmd[130];
Char res[1024];

void gpiberr(char *msga);

int main(int argc,char **argv){

  register int i=0;
  char spr;
  int dev;
  char eosbyte = 0x0a;


  /* 
   * Lookup device in the device configuration
   * and initialize board functions
   * 
   */


  if((dev = ibfind("voltmeter")) & ERR){
    gpiberr("ibfind err");
    exit(1);
  }

  /* this could be used to set your debugging level 
   * If you enable this you will see verbose messages in /usr/adm/messages
   * or with the 'dmesg' command.
   */

  /*ibSdbg(dev,0x06);*/  

  /*
   * Send IFC and REN
   * this should not be necessary if configured to be done by ibfind
   *
   */


  printf("\nsend IFC");
  if( ibsic(dev) & ERR ){
    gpiberr("ibsic Err");
    exit(1);

  }

  printf("\nset REM");
  if( ibsre(dev,1) & ERR ){
    gpiberr("ibsre err");
    exit(1);
  }


#if WAIT
  getchar();
#endif

  /* 
   * Now send device reset
   *
   */
 
  printf("\nclear device..");
  if(ibclr(dev) & ERR){
    gpiberr("clr err");
    exit(1);
  }

#if WAIT
  getchar();
#endif

  /*
   * Issue a string to the device
   * for my DVM that means that the string is displayed on the
   * front panel display.
   */

  printf("\nsend nice string..");
  strcpy(cmd,"D1 Init_Device");
  if( ibwrt(dev,cmd,strlen(cmd)) & ERR ){
    gpiberr("wrt err");
    exit(1);
  }

#if WAIT
  getchar();
#endif

  /*
   * Send some configuration strings
   * if SRQ waiting is used my device has to be configured
   * to SRQ generation with 'Q1'
   *
   */


  printf("\nsetup Device");
#if USE_SRQ
  strcpy(cmd,"D0 L0 Q1 T1 R2 A1 S0");
#else
  strcpy(cmd,"D0 L0 Q0 T1 R2 A1 S0");
#endif
  if( ibwrt(dev,cmd,strlen(cmd)) & ERR ){
    gpiberr("wrt err");
    exit(1);
  }


  for(i=0;i<READS;i++){


#if USE_SRQ

  /*
   * If SRQ waiting is enabled your device will assert the SRQ line 
   * whenever a value is reading for reading or another requested
   * action has been done.
   *
   * if you call ibwait() with the RQS flag the device will be serial
   * polled if the SRQ interrupt occurs, if the SRQ is not from the
   * requested the process goes to sleep again.
   *
   */
#if 0
  printf("\nWaiting for SRQI...");fflush(stdout);
  if( ibwait(dev,TIMO|SRQI) & (ERR |TIMO)){
    gpiberr("ibwait Error");
    /*exit(1);*/
  } else
  printf("OK GOT IT\n");fflush(stdout);
#else
  printf("\nWaiting for RQS...");fflush(stdout);
  if( ibwait(dev,TIMO|RQS) & (ERR |TIMO)){
    gpiberr("ibwait Error");
    /*exit(1);*/
  } else
  printf("OK GOT IT\n");fflush(stdout);
#endif


#endif

  /*
   * OK, now read the value
   *
   *
   */


  printf("\nReading Value...");

  if( ibrd(dev,res,1223) & ERR){
    printf("\n Warning, error occured!");
    gpiberr("read error");
  }
  res[ibcnt]=0;
  printf("\nres='%s' cnt=%d\n",res,ibcnt);

  }

 /*
  * This shows how ibcmd could be used
  * to clean up the bus sending UNL UNT messages.
  *
  */


  printf("\nSend Unlisten,Untalk\n");
  cmd[0] = UNL;
  cmd[1] = UNT;
  if( ibcmd(dev,cmd,2L) & ERR ){
    gpiberr("error sending UNL,UNT\n");
    exit(1);
  }


  if( ibsre(dev,0) & ERR ){      /* unset rem */
    gpiberr("ibsre err");
    exit(1);
  }

  ibonl(dev,0);
	return 0;
}



/*
 * This is a simple error handling function
 *
 */


void gpiberr(char *msg) {

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


/*ibonl(0);*/
}

