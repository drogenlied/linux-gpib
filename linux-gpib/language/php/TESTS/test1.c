/* -*- linux-c -*- */
#include <ugpib.h>

/* ---- macros for declarations -------------------------- */

main (int argc, char *argv[])
{
   int n,status;
   int taille;
   n = ibfind("dev1");
   printf("unite=%d\n",n);
   ibpad(n,25);
   ibclr(n);
   //ibwrt(n,argv[1],strlen(argv[1]));
//   taille=20;
   status=ibwrt(n,"*RST",4);
   printf("rst status= %d\n", status);
   sleep(3);
   status=ibwrt(n,"DISPLAY:TEXT:CLEAR",18);
   printf("clear status= %d\n", status);
   sleep(3);
   status=ibwrt(n,"SOURCE:NSELECT:1",16);
   printf("nselect status= %d\n", status);
   sleep(3);
   status=ibwrt(n,"SOURCE:VOLTAGE 2",16);
   printf("voltage status= %d\n", status);
}
