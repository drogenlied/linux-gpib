/*
 *  This is a short example how the VFS interface is used
 *  
 *  In this example I use fopen/fprintf etc routines
 *  so it is necessary to flush the input/output streams before
 *  strings are received or sent.
 *  
 *  This examples assumes that a device inode
 *  /dev/gpib0/dvm exists, the minor number of the device is the GPIB primary
 *  address of the device.
 *
 */

#include <stdio.h>
#include <unistd.h>

FILE *dr,*dw;

char s[512];

int main(){

  if(( dr = fopen("/dev/gpib0/dvm","r")) == NULL){
    printf("Error opening /dev/gpib0/dvm for reading\n");
    exit(1);
  }

  if(( dw = fopen("/dev/gpib0/dvm","w")) == NULL){
    printf("Error opening /dev/gpib0/dvm for writing\n");
    exit(1);
  }
  printf("sending string...\n");

  fprintf(dw,"D1 VFS Test\n");fflush(dw);
  sleep(1);
  fprintf(dw,"D0 \n");fflush(dw);
  
  fgets(s,13,dr);fflush(dr);
 
  printf("Returned String = '%s'\n",s);


  fclose(dr);
  fclose(dw);

	return 0; 
}

