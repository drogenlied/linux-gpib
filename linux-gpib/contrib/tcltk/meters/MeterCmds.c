#include <tcl.h>
#include <tk.h>

#include "tkTurndial.h"

extern int Tk_DialCmd _ANSI_ARGS_(( ClientData clientData,
			      Tcl_Interp *interp,
			       int argc,
			       char *argv[]
			       ));

extern int Tk_PieCmd _ANSI_ARGS_(( ClientData clientData,
			      Tcl_Interp *interp,
			       int argc,
			       char *argv[]
			       ));

extern int Tk_StripchartCmd _ANSI_ARGS_(( ClientData clientData,
			      Tcl_Interp *interp,
			       int argc,
			       char *argv[]
			       ));

extern int Tk_BargraphCmd _ANSI_ARGS_(( ClientData clientData,
			      Tcl_Interp *interp,
			       int argc,
			       char *argv[]
			       ));

extern int Tk_TurndialCmd _ANSI_ARGS_((ClientData clientData,
        Tcl_Interp *interp, int argc, char **argv));




int Meters_Init ( Tcl_Interp *interp ){

  Tk_Window main;

  main = Tk_MainWindow(interp);

    Tcl_CreateCommand(interp,"dial",Tk_DialCmd,
		         (ClientData) main,
			 (Tcl_CmdDeleteProc *) NULL );


    Tcl_CreateCommand(interp,"pie",Tk_PieCmd,
		         (ClientData) main,
			 (Tcl_CmdDeleteProc *) NULL );


    Tcl_CreateCommand(interp,"stripchart",Tk_StripchartCmd,
		         (ClientData) main,
			 (Tcl_CmdDeleteProc *) NULL );

    Tcl_CreateCommand(interp,"bargraph",Tk_BargraphCmd,
		         (ClientData) main,
			 (Tcl_CmdDeleteProc *) NULL );

    Tcl_CreateCommand(interp, "turndial", Tk_TurndialCmd, (ClientData) main,
                         (void (*)()) NULL);


    return TCL_OK;
}

