/*
 *
 * SoundCmds.c -
 *     
 *     Sount It TCL Interface
 *
 *     (c) 1995 C. Schroeter (clausi@chemie.fu-berlin.de)
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tcl.h>
#include "soundIt.h"


int Sound_Init ( Tcl_Interp *interp ){

extern int SoundCmd _ANSI_ARGS_(( ClientData clientData,
			      Tcl_Interp *interp,
			       int argc,
			       char *argv[]
			       ));

    Tcl_CreateCommand(interp,"sound",SoundCmd,
		         (ClientData) NULL,
			 (Tcl_CmdDeleteProc *) NULL );

    return TCL_OK;
}


/**********************************************************************/

#define MAX_SAMPLES 8
#define SAMPLE_RATE 11000

Sample smp[MAX_SAMPLES];
int    smp_count=0;
int initialized=0;

int SoundCmd _ANSI_ARGS_(( ClientData clientData,
			Tcl_Interp *interp,
			int argc,
			char *argv[]
			)) 
{
 register i,j;

 if ( *argv[1] == 'l' && !strcmp(argv[1],"load")) {


   if( argc < 3 ){
     Tcl_AppendResult(interp,"Usage: sound load <list> ",(char *) NULL);
     return TCL_ERROR;
   }

  if( smp_count<MAX_SAMPLES ){
    /*printf("loading %s as %d\n",argv[2],smp_count);*/
    Snd_loadRawSample( argv[2], &smp[smp_count++] );
  }

/*printf("loaded %d sounds",smp_count);*/
  return TCL_OK;
 }

 if ( *argv[1] == 'i' && !strcmp(argv[1],"init")) {

  if( !initialized ){
  if( Snd_init(smp_count,smp,SAMPLE_RATE,smp_count,"/dev/dsp") == EXIT_FAILURE ){
     Tcl_AppendResult(interp,"Can't Init SountIt Library ",(char *) NULL);
     return TCL_ERROR;
  }
  initialized++;
  }

  return TCL_OK;
 }

 if ( *argv[1] == 'p' && !strcmp(argv[1],"play")) {
   if( argc < 4 ){
     Tcl_AppendResult(interp,"Usage: sound play <index> <channel> ",(char *) NULL);
     return TCL_ERROR;
   }
/*printf("mixing %d in channel %d",atoi(argv[2]),atoi(argv[3]) );*/
   Snd_effect( atoi(argv[2]),atoi(argv[3]) );
   return TCL_OK;
 }

 if ( *argv[1] == 'r' && !strcmp(argv[1],"restore")) {
   Snd_restore();
   return TCL_OK;
 }


 Tcl_AppendResult(interp,"Usage: sound command <parameter> ",(char *) NULL);
 return TCL_ERROR;
 

}















