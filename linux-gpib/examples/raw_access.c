/*
 *  This example demonstrates how to use raw bus access, in contrast to
 *  normal bus access the user must take care that the bus is in correct
 *  state before any action can be done.
 *
 *  Raw access should not be used if more than one process is using the bus
 *  because device functions could mess up the bus addressing and in worst case
 *  hang the bus so bdonl() always tries to open the driver O_EXCL and fails
 *  if another process is using the driver. If one process is using the driver
 *  in raw access mode no other process can open the driver.
 */

#include <stdio.h>
#include <ib.h>

char cmd[130];
char res[1024];

int main(int argc,char **argv){

  int dev;
  unsigned long  i=0;

  /*
   *  Open device 'gpib0' 
   *
   */
  
  if((dev = ibfind("gpib0")) & ERR){
    printf("ibfind err: can't find 'gpib0' \n");
    exit(1);
  }

  /*
   *  Check if this device has the master flag set
   *
   */

  if( ibIsMaster(dev) )
    printf(" Opened 'gpib0' as master \n");
  else {
    printf("device 'gpib0' is not set as master \n");
    exit(1);
  }

  /*
   *  Initialize GPIB board
   *
   */

  if( ibonl(dev,1) & ERR ){
    printf("can't initialize board device busy or other error\n");
    exit(1);
  }

getchar();


  /*
   * Send IFC and REN
   *
   * 
   */

  if( ibsic(dev) & ERR ){
    printf("can't send SIC \n");
    exit(1);
  }

  if( ibsre(dev,1) & ERR ){
    printf("can't send REN \n");
    exit(1);
  }


  /*
   *  Now initialize addressing for issueing commands
   *
   */

  i=0;
  cmd[i]    = UNL;           /* send Unlisten */
  cmd[i++]  = 7 | LAD;       /* send listener address */
  cmd[i++]  = 0 | TAD;       /* send talker adress 
                                (assuming that 0 is the controller */

  if( ibcmd( dev, cmd, i ) & ERR ){   /* send out addressing */
    printf("ibcmd failed\n");
    exit(1);
  }

#define STRING "D1 Hallo"

  /* now send string to the device adressed as listener */

  if( ibwrt( dev, STRING, strlen(STRING)) & ERR ){
    printf("ibwrt failed\n");
    exit(1);
  }
  
  

  /* 
   * Now lets see how read works:
   *
   * first swap talker and listener adressing
   */

  i=0;
  cmd[i]    = UNL;           /* send Unlisten */
  cmd[i++]  = 0 | LAD;       /* send listener address */
  cmd[i++]  = 7 | TAD;       /* send talker adress 
                                (assuming that 0 is the controller) */

  if( ibcmd( dev, cmd, i ) & ERR ){   /* send out addressing */
    printf("ibcmd failed\n");
    exit(1);
  }


  /* now try to read a string */

  if( ibrd( dev, res, 1024) & ERR ){
    printf("read failed \n");
    exit(1);
  }

  printf("The String is '%s' \n",res);



  /* clean up the bus */


  i=0;
  cmd[i]    = UNL;           /* send Unlisten */
  cmd[i++]  = UNT;           /* send Untalk */

  if( ibcmd( dev, cmd, i ) & ERR ){   /* send out addressing */
    printf("ibcmd failed\n");
    exit(1);
  }

  ibsre(dev,0);

  ibonl(dev,0);
	return 0;
}

 
