/*
 * tkBargraph.c --
 *
 *	This module implements a bargraph widget for the Tk toolkit.
 *
 * Copyright 1992 Regents of the University of Victoria, Wellington, NZ.
 * This code is derived from the tkScale widget.
 * Copyright 1990 Regents of the University of California.
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tcl.h"
#include "tk.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * BUGS:
 *   Vertical bargraphs:
 *     There is no left PADDING (don't know why)
 *     If maxv == minv, will crash.
 *     When bar drawn down, the bar is 2 pixels too short
 *   Horizontal bargraphs:
 *     Bar offset 2 pixels to the left
 *     Missing a tick
 *
 * TO DO:
 */

/*
 *  Default bargraph configuration values
 */

#define DEF_BARGRAPH_BG_COLOR            "gray77"
#define DEF_BARGRAPH_BAR_COLOR           "red"
#define DEF_BARGRAPH_TEXT_COLOR          "black"
#define DEF_BARGRAPH_TICK_COLOR          "blue2"
#define DEF_BARGRAPH_CURSOR              ((char *) NULL)

#define ALT_BARGRAPH_BG_COLOR           "red2"
#define ALT_BARGRAPH_TICK_COLOR         "gray70"
#define ALT_BARGRAPH_BAR_COLOR          "green"
#define ALT_BARGRAPH_TEXT_COLOR         "white"

        /* for monochrome displays */

#define DEF_BARGRAPH_BG_MONO             "white"
#define DEF_BARGRAPH_BAR_MONO            "black"
#define DEF_BARGRAPH_TEXT_MONO           "black"
#define DEF_BARGRAPH_TICK_MONO           "black"

#define ALT_BARGRAPH_BG_MONO             "black"
#define ALT_BARGRAPH_BAR_MONO            "white"
#define ALT_BARGRAPH_TEXT_MONO           "white"
#define ALT_BARGRAPH_TICK_MONO           "white"

#define DEF_BARGRAPH_BORDER_WIDTH        "3"
#define DEF_BARGRAPH_FONT                "*-Helvetica-Bold-R-Normal-*-120-*"
#define DEF_BARGRAPH_MINVALUE            "0"
#define DEF_BARGRAPH_MAXVALUE            "100"
#define DEF_BARGRAPH_BASEVALUE           "0"
#define DEF_BARGRAPH_CALLBACK_INTERVAL   "500"
#define DEF_BARGRAPH_TITLE               (char *)NULL
#define DEF_BARGRAPH_LENGTH              "200"
#define DEF_BARGRAPH_ORIENT              "vertical"
#define DEF_BARGRAPH_RELIEF              "raised"
#define DEF_BARGRAPH_SHOW_VALUE          "true"
#define DEF_BARGRAPH_TICK_INTERVAL       "20"
#define DEF_BARGRAPH_WIDTH               "20"
#define DEF_BARGRAPH_BARBORDERWIDTH      "0"
#define DEF_BARGRAPH_BARRELIEF           "raised"
#define DEF_USERBITS                     "0"
#define DEF_USERDATA                     (char *) NULL

#define TICK_LENGTH              4
#define PADDING                  2
#define  min(a, b)               ((a) < (b) ? (a) : (b))
#define  max(a, b)               ((a) > (b) ? (a) : (b))
#define  hasatitle(d)            (((d)->title != NULL) && \
                                  ((d)->title[0] != '\0'))
#define  abs(x)                  ( ((x) < 0) ? -(x) : (x) )


/*
 * A data structure of the following type is kept for each barchart
 * widget managed by this file:
 */

typedef struct bargraph_struct Bargraph;
struct bargraph_struct {
    Tk_Window tkwin;		/* Window that embodies the Bargraph.  NULL
				 * means that the window has been destroyed
				 * but the data structures haven't yet been
				 * cleaned up.*/
    Display *display;
    Tcl_Interp *interp;		/* Interpreter associated with Bargraph. */
    Tk_Uid orientUid;		/* Orientation for window ("vertical" or
				 * "horizontal"). */
    int vertical;		/* Non-zero means vertical orientation,
				 * zero means horizontal. */
    double value;		/* Current value of Bargraph. */
    double base_value;          /* Value that is the root of the bar */
    int basePixels;             /* Offset in pixels base value is from top
				 * of window */
    int grow_up;                /* TRUE if the bar grows "up" */
    double max_value;		/* Value corresponding to right or top of
				 * Bargraph. */
    double min_value;		/* Value corresponding to left or bottom
				 * of Bargraph. */
    int tickInterval;		/* Distance between tick marks;  0 means
				 * don't display any tick marks. */
    char *command;		/* Command used for callback. Malloc'ed. */
    int continue_callback;    /* boolean flag used to terminate the callback */
    unsigned long interval;   /* interval (mS) between callbacks             */
    char *title;		/* Label to display above or to right of
				 * Bargraph;  NULL means don't display a
				 * label.  Malloc'ed. */
    Cursor cursor;		/* Current cursor for window, or None.  -jules- */

    /*
     * Information used when displaying widget:
     */

    int borderWidth;		/* Width of 3-D border around window. */
    Tk_3DBorder Border; 	/* Used for drawing background. */
    int barborderWidth;         /* Width of bar border */
    int barrelief;              /* Bar relief, e.g., TK_RELIEF_RAISED */
    Tk_3DBorder barBorder;	/* Used for drawing bar. */
    XFontStruct *fontPtr;	/* Information about text font, or NULL. */
    XColor *textColorPtr;	/* Color for drawing text. */
    GC textGC;			/* GC for drawing text. */
    XColor *tickColorPtr;       /* Color for drawing ticks */
    GC tickGC;                  /* GC for drawing ticks */
    int width;			/* Desired narrow dimension of Bargraph,
				 * in pixels. */
    int max_height;		/* Desired long dimension of Bargraph,
				 * in pixels. */
    int relief;			/* Indicates whether window as a whole is
				 * raised, sunken, or flat. */
    int showValue;		/* Non-zero means to display the Bargraph value
				 * below or to the left of the slider;  zero
				 * means don't display the value. */
    int tickPixels;		/* Number of pixels required for widest tick
				 * mark.  0 means don't display ticks.*/
    XSegment *ticks;            /* The tick marks.  Malloced */
    int valuePixels;		/* Number of pixels required for value text. */

    /*
     * Miscellaneous information:
     */

    int displaybits;		/* Various flags;  see below for 
				 * definitions. */
    int userbits;
    char *userdata;
    int (*callback_func)(Tcl_Interp *,
			 Bargraph *); /* Function to be invoked on callback. */

    
    /*
     *  Alternative colour scheme
     */

    Tk_3DBorder altborder;        /* Structure used to draw border  */
    Tk_3DBorder altbarBorder;     /* structure used to draw bar  */
    XColor *a_textColor;          /* alternative text color         */
    XColor *a_tickColor;          /* alternative tick color         */

};

/*
 *  displaybits:
 *  DISPLAY_TICKS - draw the tick points
 *  DISPLAY_BAR - draw the bar
 *  DISPLAY_BORDER - draw tye border
 *  DISPLAY_TITLE - draw the title
 *  DISPLAY_VALUE - draw the textual value
 */

#define   DISPLAY_TICKS           1
#define   DISPLAY_BAR             2
#define   DISPLAY_BORDER          4
#define   DISPLAY_VALUE           8
#define   DISPLAY_TITLE          16
#define   REDRAW_PENDING         32
#define   CLEAR_NEEDED           64

#define   DISPLAY_ALL_BUT_TEXT   (DISPLAY_TICKS | DISPLAY_BAR |           \
                                  DISPLAY_BORDER)

#define   DISPLAY_ALL            (DISPLAY_TICKS | DISPLAY_BAR |           \
                                  DISPLAY_BORDER | DISPLAY_VALUE |        \
                                  DISPLAY_TITLE)


/*
 * Information used for argv parsing.
 */


static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_BORDER, "-altbackground", "background", "Background",
        ALT_BARGRAPH_BG_COLOR, Tk_Offset(Bargraph, altborder),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altbackground", "background", "Background",
        ALT_BARGRAPH_BG_MONO, Tk_Offset(Bargraph, altborder),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_BORDER, "-altbarcolor", "altbarcolor", "Foreground",
        ALT_BARGRAPH_BAR_COLOR, Tk_Offset(Bargraph, altbarBorder),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altbarcolor", "altbarcolor", "Foreground",
        ALT_BARGRAPH_BAR_MONO, Tk_Offset(Bargraph, altbarBorder),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altbarcolour", "altbarcolor", (char *) NULL,
        (char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altbc", "altbarcolor", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altbg", "altbackground", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-alttc", "alttickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
        ALT_BARGRAPH_TEXT_COLOR, Tk_Offset(Bargraph, a_textColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
        ALT_BARGRAPH_TEXT_MONO, Tk_Offset(Bargraph, a_textColor),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttextcolour", "alttextcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_BARGRAPH_TICK_COLOR, Tk_Offset(Bargraph, a_tickColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_BARGRAPH_TICK_MONO, Tk_Offset(Bargraph, a_tickColor),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttickcolour", "alttickcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif

    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_BARGRAPH_BG_COLOR, Tk_Offset(Bargraph, Border),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_BARGRAPH_BG_MONO, Tk_Offset(Bargraph, Border),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_INT, "-barborderwidth", "barborderwidth", "Barborderwidth",
	DEF_BARGRAPH_BARBORDERWIDTH, Tk_Offset(Bargraph, barborderWidth), 0},
    {TK_CONFIG_BORDER, "-barcolor", "barcolor", "Foreground",
	DEF_BARGRAPH_BAR_COLOR, Tk_Offset(Bargraph, barBorder),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-barcolor", "barcolor", "Foreground",
	DEF_BARGRAPH_BAR_MONO, Tk_Offset(Bargraph, barBorder),
	TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-barcolour", "barcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif

    {TK_CONFIG_RELIEF, "-barrelief", "barrelief", "Barrelief",
	DEF_BARGRAPH_BARRELIEF, Tk_Offset(Bargraph, barrelief), 0},
    {TK_CONFIG_DOUBLE, "-base", "base", "Base",
	DEF_BARGRAPH_BASEVALUE, Tk_Offset(Bargraph, base_value), 0},
    {TK_CONFIG_SYNONYM, "-bd", "borderwidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_INT, "-borderwidth", "borderwidth", "Borderwidth",
	DEF_BARGRAPH_BORDER_WIDTH, Tk_Offset(Bargraph, borderWidth), 0},
    {TK_CONFIG_STRING, "-command", "command", "Command",
	(char *) NULL, Tk_Offset(Bargraph, command), 0},
    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
       DEF_BARGRAPH_CURSOR, Tk_Offset(Bargraph, cursor),
       TK_CONFIG_NULL_OK}, 
    {TK_CONFIG_STRING, "-data", "data", "Data",
	DEF_USERDATA, Tk_Offset(Bargraph, userdata), 0},
    {TK_CONFIG_SYNONYM, "-fg", "barcolor", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_FONT, "-font", "font", "Font",
	DEF_BARGRAPH_FONT, Tk_Offset(Bargraph, fontPtr),
	0},
    {TK_CONFIG_INT, "-height", "height", "Height",
	DEF_BARGRAPH_LENGTH, Tk_Offset(Bargraph, max_height), 0},
    {TK_CONFIG_INT, "-interval", "interval", "Interval",
        DEF_BARGRAPH_CALLBACK_INTERVAL, Tk_Offset(Bargraph, interval), 0},
    {TK_CONFIG_DOUBLE, "-max", "max", "Max",
	DEF_BARGRAPH_MAXVALUE, Tk_Offset(Bargraph, max_value), 0},
    {TK_CONFIG_DOUBLE, "-min", "min", "Min",
	DEF_BARGRAPH_MINVALUE, Tk_Offset(Bargraph, min_value), 0},
    {TK_CONFIG_UID, "-orient", "orient", "Orient",
	DEF_BARGRAPH_ORIENT, Tk_Offset(Bargraph, orientUid), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_BARGRAPH_RELIEF, Tk_Offset(Bargraph, relief), 0},
    {TK_CONFIG_BOOLEAN, "-showvalue", "showValue", "ShowValue",
	DEF_BARGRAPH_SHOW_VALUE, Tk_Offset(Bargraph, showValue), 0},
    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Textcolor",
	DEF_BARGRAPH_TEXT_COLOR, Tk_Offset(Bargraph, textColorPtr),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Textcolor",
	DEF_BARGRAPH_TEXT_MONO, Tk_Offset(Bargraph, textColorPtr),
	TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-textcolour", "textcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Tickcolor",
	DEF_BARGRAPH_TICK_COLOR, Tk_Offset(Bargraph, tickColorPtr),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Tickcolor",
	DEF_BARGRAPH_TICK_MONO, Tk_Offset(Bargraph, tickColorPtr),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_INT, "-tickinterval", "tickInterval", "TickInterval",
	DEF_BARGRAPH_TICK_INTERVAL, Tk_Offset(Bargraph, tickInterval), 0},
    {TK_CONFIG_STRING, "-title", "title", "Title",
	DEF_BARGRAPH_TITLE, Tk_Offset(Bargraph, title), 0},
    {TK_CONFIG_INT, "-userbits", "userbits", "Userbits",
	DEF_USERBITS, Tk_Offset(Bargraph, userbits), 0},
    {TK_CONFIG_INT, "-width", "width", "Width",
	DEF_BARGRAPH_WIDTH, Tk_Offset(Bargraph, width), 0},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static int      BarHeight _ANSI_ARGS_((Bargraph *BargraphPtr, double value));
static void     Callback _ANSI_ARGS_((Bargraph *BargraphPtr));
static void	ComputeBargraphGeometry _ANSI_ARGS_((Bargraph *BargraphPtr));
static int	ConfigureBargraph _ANSI_ARGS_((Tcl_Interp *interp,
		    Bargraph *BargraphPtr, int argc, char **argv,
		    int flags));
static void	DestroyBargraph _ANSI_ARGS_((ClientData clientData));
static void	DisplayHorizontalBargraph _ANSI_ARGS_((
		    ClientData clientData));
static void	DisplayHorizontalValue _ANSI_ARGS_((Bargraph *BargraphPtr,
		    double value, double bottom));
static void	DisplayVerticalBargraph _ANSI_ARGS_((
		    ClientData clientData));
static void	DisplayVerticalValue _ANSI_ARGS_((Bargraph *BargraphPtr,
		    double value, double rightEdge));
static void	EventuallyRedrawBargraph _ANSI_ARGS_((Bargraph *BargraphPtr,
		    int what));
static void	BargraphEventProc _ANSI_ARGS_((ClientData clientData,
		    XEvent *eventPtr));
static void	BargraphMouseProc _ANSI_ARGS_((ClientData clientData,
		    XEvent *eventPtr));
static int	BargraphWidgetCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static void	SetBargraphValue _ANSI_ARGS_((Bargraph *BargraphPtr,
		    double value));
static int	ValueToPixel _ANSI_ARGS_((Bargraph *BargraphPtr, double value));
static void     SwapColours _ANSI_ARGS_((Bargraph *BargraphPtr));
static void     ReplaceColours _ANSI_ARGS_((Bargraph *BargraphPtr, int argc,
					    char *argv[]));

/*
 *--------------------------------------------------------------
 *
 * Tk_BargraphCmd --
 *
 *	This procedure is invoked to process the "Bargraph" Tcl
 *	command.  See the user documentation for details on what
 *	it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *--------------------------------------------------------------
 */

int
Tk_BargraphCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    register Bargraph *BargraphPtr;
    Tk_Window new;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " pathName ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }

    new = Tk_CreateWindowFromPath(interp, tkwin, argv[1], (char *) NULL);
    if (new == NULL) {
	return TCL_ERROR;
    }

    /*
     * Initialize fields that won't be initialized by ConfigureBargraph,
     * or which ConfigureBargraph expects to have reasonable values
     * (e.g. resource pointers).
     */

    BargraphPtr = (Bargraph *) ckalloc(sizeof(Bargraph));
    BargraphPtr->tkwin = new;
    BargraphPtr->display = Tk_Display(new);
    BargraphPtr->interp = interp;
    BargraphPtr->value = 0;
    BargraphPtr->command = NULL;
    BargraphPtr->userdata = NULL;
    BargraphPtr->title = NULL;
    BargraphPtr->Border = NULL;
    BargraphPtr->barBorder = NULL;
    BargraphPtr->cursor = None;	           /* -jules- */
    BargraphPtr->fontPtr = NULL;
    BargraphPtr->textColorPtr = NULL;
    BargraphPtr->textGC = None;
    BargraphPtr->tickColorPtr = NULL;
    BargraphPtr->tickGC = None;

    BargraphPtr->altborder = NULL;
    BargraphPtr->altbarBorder = NULL;
    BargraphPtr->a_textColor = NULL;
    BargraphPtr->a_tickColor = NULL;

    BargraphPtr->displaybits = 0;
    BargraphPtr->ticks = NULL;
    BargraphPtr->continue_callback = FALSE;
    BargraphPtr->callback_func = NULL;

    Tk_SetClass(BargraphPtr->tkwin, "Bargraph");
    Tk_CreateEventHandler(BargraphPtr->tkwin, ExposureMask|StructureNotifyMask,
	    BargraphEventProc, (ClientData) BargraphPtr);
    Tcl_CreateCommand(interp, Tk_PathName(BargraphPtr->tkwin),
	    BargraphWidgetCmd, (ClientData) BargraphPtr, (void (*)()) NULL);
    if (ConfigureBargraph(interp, BargraphPtr, argc-2, argv+2, 0) != TCL_OK) {
	goto error;
    }

    interp->result = Tk_PathName(BargraphPtr->tkwin);
    return TCL_OK;

    error:
    Tk_DestroyWindow(BargraphPtr->tkwin);
    return TCL_ERROR;
}

/*
 *--------------------------------------------------------------
 *
 * BargraphWidgetCmd --
 *
 *	This procedure is invoked to process the Tcl command
 *	that corresponds to a widget managed by this module.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int
BargraphWidgetCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Information about Bargraph
					 * widget. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register Bargraph *BargraphPtr = (Bargraph *) clientData;
    int result = TCL_OK;
    int length;
    char c;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tk_Preserve((ClientData) BargraphPtr);
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)) {
	if (argc == 2) {
	    result = Tk_ConfigureInfo(interp, BargraphPtr->tkwin, configSpecs,
		    (char *) BargraphPtr, (char *) NULL, 0);
	} else if (argc == 3) {
	    result = Tk_ConfigureInfo(interp, BargraphPtr->tkwin, configSpecs,
		    (char *) BargraphPtr, argv[2], 0);
	} else {
	    result = ConfigureBargraph(interp, BargraphPtr, argc-2, argv+2,
		    TK_CONFIG_ARGV_ONLY);
	}
    } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
	if (argc != 2) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " get\"", (char *) NULL);
	    goto error;
	}
	sprintf(interp->result, "%g", BargraphPtr->value);
    } else if ((c == 's') && (strncmp(argv[1], "set", length) == 0)) {
	double value;

	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " set value\"", (char *) NULL);
	    goto error;
	}
	if (Tcl_GetDouble(interp, argv[2], &value) != TCL_OK) {
	    goto error;
	}
	SetBargraphValue(BargraphPtr, value);
      } else if ((c == 's') && (strncmp(argv[1], "start", length) == 0)) {
	if (! BargraphPtr->continue_callback ) {
	  BargraphPtr->continue_callback = TRUE;
	  Callback(BargraphPtr);
	}
      } else if ((c == 's') && (strncmp(argv[1], "stop", length) == 0)) {
        BargraphPtr->continue_callback = FALSE;
      }
      else if ((c == 's') && (strncmp(argv[1], "swapcolours", length) == 0)) {
	  SwapColours(BargraphPtr);
	}
      else if ((c == 'r') && (strncmp(argv[1],"replacecolours",length) == 0)) {
	ReplaceColours(BargraphPtr, argc-2, argv+2);
      } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
			 "\":  must be configure, get, set, start or stop",
			 (char *) NULL);
	goto error;
      }
    Tk_Release((ClientData) BargraphPtr);
    return result;

  error:
    Tk_Release((ClientData) BargraphPtr);
    return TCL_ERROR;
}

/*
 *------------------------------------------------------------------------
 *
 *  Callback --
 *      Execute a Tcl command repeatedly until told to stop.  Involked
 *      with the start command and stopped with the stop command.
 *
 *  Results:
 *      None.
 *
 *  Side Effects:
 *      Timer queue is changed with the addition of a command to be
 *      executed periodically.
 *
 *------------------------------------------------------------------------
 */


static void Callback(BargraphPtr)
     Bargraph *BargraphPtr;
{
  int no_errors = TRUE;
  int result;

  if (BargraphPtr->callback_func != NULL) 
    SetBargraphValue(BargraphPtr,
		     (*(BargraphPtr->callback_func))(BargraphPtr->interp,BargraphPtr));

  if ( BargraphPtr->command != NULL && BargraphPtr->command[0] != '\0' ) {
    result = Tcl_Eval( BargraphPtr->interp, BargraphPtr->command );
    if ( result != TCL_OK ) {
      /*TkBindError(BargraphPtr->interp);*/
      no_errors = FALSE;
    }
  }

  if ( BargraphPtr->continue_callback && no_errors )
    Tk_CreateTimerHandler( BargraphPtr->interval, (void *)Callback,
                          (ClientData) BargraphPtr );

  return;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyBargraph --
 *
 *	This procedure is invoked by Tk_EventuallyFree or Tk_Release
 *	to clean up the internal structure of a button at a safe time
 *	(when no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the Bargraph is freed up.
 *
 *----------------------------------------------------------------------
 */

static void
DestroyBargraph(clientData)
    ClientData clientData;	/* Info about Bargraph widget. */
{
    register Bargraph *BargraphPtr = (Bargraph *) clientData;

    if (BargraphPtr->command != NULL)
	ckfree(BargraphPtr->command);

    if (BargraphPtr->title != NULL)
	ckfree(BargraphPtr->title);

    if (BargraphPtr->userdata != NULL)
	ckfree(BargraphPtr->userdata);

    if (BargraphPtr->Border != NULL)
	Tk_Free3DBorder(BargraphPtr->Border);

    if (BargraphPtr->barBorder != NULL)
	Tk_Free3DBorder(BargraphPtr->barBorder);

    if (BargraphPtr->altbarBorder != NULL)
      Tk_Free3DBorder(BargraphPtr->altbarBorder);

    if (BargraphPtr->fontPtr != NULL)
	Tk_FreeFontStruct(BargraphPtr->fontPtr);

    if (BargraphPtr->textColorPtr != NULL)
	Tk_FreeColor(BargraphPtr->textColorPtr);

    if (BargraphPtr->textGC != None)
	Tk_FreeGC(BargraphPtr->display,BargraphPtr->textGC);

    if (BargraphPtr->tickColorPtr != NULL)
	Tk_FreeColor(BargraphPtr->tickColorPtr);

    if (BargraphPtr->tickGC != None)
	Tk_FreeGC(BargraphPtr->display,BargraphPtr->tickGC);

    if (BargraphPtr->a_textColor != NULL)
      Tk_FreeColor(BargraphPtr->a_textColor);

    if (BargraphPtr->a_tickColor != NULL)
      Tk_FreeColor(BargraphPtr->a_tickColor);

    if (BargraphPtr->ticks != NULL)
      free(BargraphPtr->ticks);

    if (BargraphPtr->cursor != None) {
	Tk_FreeCursor(BargraphPtr->display,BargraphPtr->cursor);	/* -jules- */
    }

    ckfree((char *) BargraphPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConfigureBargraph --
 *
 *	This procedure is called to process an argv/argc list, plus
 *	the Tk option database, in order to configure (or
 *	reconfigure) a Bargraph widget.
 *
 * Results:
 *	The return value is a standard Tcl result.  If TCL_ERROR is
 *	returned, then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as colors, border width,
 *	etc. get set for BargraphPtr;  old resources get freed,
 *	if there were any.
 *
 *----------------------------------------------------------------------
 */

static int
ConfigureBargraph(interp, BargraphPtr, argc, argv, flags)
    Tcl_Interp *interp;		    /* Used for error reporting. */
    register Bargraph *BargraphPtr; /* Information about widget;  may or may
				     * not already have values for some fields
                                     */
    int argc;			    /* Number of valid entries in argv. */
    char **argv;		    /* Arguments. */
    int flags;			    /* Flags to pass to Tk_ConfigureWidget. */
{
    XGCValues gcValues;
    GC newGC;
    int length;

    if (Tk_ConfigureWidget(interp, BargraphPtr->tkwin, configSpecs,
	    argc, argv, (char *) BargraphPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * A few options need special processing, such as parsing the
     * orientation or setting the background from a 3-D border.
     */

    length = strlen(BargraphPtr->orientUid);
    if (strncmp(BargraphPtr->orientUid, "vertical", length) == 0) {
	BargraphPtr->vertical = 1;
    } else if (strncmp(BargraphPtr->orientUid, "horizontal", length) == 0) {
	BargraphPtr->vertical = 0;
    } else {
	Tcl_AppendResult(interp, "bad orientation \"", BargraphPtr->orientUid,
		"\": must be vertical or horizontal", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Make sure that the tick interval has the right sign so that
     * addition moves from min_value to max_value.
     */

    if (BargraphPtr->tickInterval < 0)
      BargraphPtr->tickInterval = -BargraphPtr->tickInterval;


    /*
     * Set the Bargraph value to itself;  all this does is to make sure
     * that the Bargraph's value is within the new acceptable range for
     * the Bargraph.
     */

    SetBargraphValue(BargraphPtr, BargraphPtr->value);

    Tk_SetBackgroundFromBorder(BargraphPtr->tkwin, BargraphPtr->Border);

    gcValues.font = BargraphPtr->fontPtr->fid;
    gcValues.foreground = BargraphPtr->textColorPtr->pixel;
    newGC = Tk_GetGC(BargraphPtr->tkwin, GCForeground|GCFont, &gcValues);
    if (BargraphPtr->textGC != None) {
	Tk_FreeGC(BargraphPtr->display,BargraphPtr->textGC);
    }
    BargraphPtr->textGC = newGC;

    gcValues.foreground = BargraphPtr->tickColorPtr->pixel;
    newGC = Tk_GetGC(BargraphPtr->tkwin, GCForeground, &gcValues);
    if (BargraphPtr->tickGC != None) {
	Tk_FreeGC(BargraphPtr->display,BargraphPtr->tickGC);
    }
    BargraphPtr->tickGC = newGC;

    /*
     * Recompute display-related information, and let the geometry
     * manager know how much space is needed now.
     */

    ComputeBargraphGeometry(BargraphPtr);

    EventuallyRedrawBargraph(BargraphPtr, DISPLAY_ALL | CLEAR_NEEDED);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeBargraphGeometry --
 *
 *	This procedure is called to compute various geometrical
 *	information for a Bargraph, such as where various things get
 *	displayed.  It's called when the window is reconfigured.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Display-related numbers get changed in *scrollPtr.  The
 *	geometry manager gets told about the window's preferred size.
 *
 *----------------------------------------------------------------------
 */

static void
ComputeBargraphGeometry(BargraphPtr)
    register Bargraph *BargraphPtr;		/* Information about widget. */
{
  XCharStruct bbox;
  char valueString[30];
  int dummy;
  int lineHeight = BargraphPtr->fontPtr->ascent +
                   BargraphPtr->fontPtr->descent;
  int tt = 0, sv = 0, st = 0;
  /* flags for title, show_value and show_ticks respectively */
  double minv = BargraphPtr->min_value;
  double maxv = BargraphPtr->max_value;
  double real_max, real_min;
  double basev;


  /*
   * Ensure that the base value is between the minimum and maximum values
   */
  basev = BargraphPtr->base_value;
  real_max = max( maxv, minv );
  real_min = min( maxv, minv );
  if ( basev < real_min )
    basev = BargraphPtr->base_value = real_min;
  else if ( basev > real_max )
    basev = BargraphPtr->base_value = real_max;

  /*
   * Calculate text extents for the value.
   * This value is used when bargraph is horizontal or vertical
   */
  if (BargraphPtr->showValue) {
    sv = 1;
    sprintf(valueString, "%g", BargraphPtr->min_value);
    XTextExtents(BargraphPtr->fontPtr, valueString, strlen(valueString),
		 &dummy, &dummy, &dummy, &bbox);
    BargraphPtr->valuePixels = bbox.rbearing + bbox.lbearing;
    sprintf(valueString, "%g", BargraphPtr->max_value);
    XTextExtents(BargraphPtr->fontPtr, valueString, strlen(valueString),
		 &dummy, &dummy, &dummy, &bbox);
    if (BargraphPtr->valuePixels < bbox.rbearing + bbox.lbearing)
      BargraphPtr->valuePixels = bbox.rbearing + bbox.lbearing;
  } else
    BargraphPtr->valuePixels = 0;

  if (BargraphPtr->tickInterval != 0) st = 1;
  if (hasatitle(BargraphPtr)) tt = 1;

  /* Set a flag indicating whether or not the bar grows "up" or "down" */
  if ( minv <= maxv )
    BargraphPtr->grow_up = TRUE;
  else
    BargraphPtr->grow_up = FALSE;


    /*
     *   -------------------------------------------
     *   |\_______________________________________/|
     *   | |        Title  (optional)            | |
     *   | |-------------------------------------| |
     *   | |       |      Ticks (opt)            | |
     *   | | Value | #####################       | |
     *   | | (opt) | #####################       | |
     *   |/---------------------------------------\|
     *    ------------------------------------------
     *                  ^-- BorderWidth
     */

  if (! BargraphPtr->vertical /* i.e. horizontal */ ) {  

    if (BargraphPtr->tickInterval != 0)
      BargraphPtr->tickPixels = lineHeight + TICK_LENGTH + PADDING;
    else
      BargraphPtr->tickPixels = 0;

/*
 *  This code will have the bug that if the text extents for the ticks are
 *  greater than Bargraph->max_height that the text will overlap somewhere
 *  in the middle of the window because of the way text against window borders
 *  are drawn.
 */

    if ( lineHeight > BargraphPtr->width )
      BargraphPtr->width = lineHeight;

    Tk_GeometryRequest(BargraphPtr->tkwin,
		       /* width */
		       BargraphPtr->borderWidth*2 +
		       BargraphPtr->max_height + BargraphPtr->valuePixels
		       + (2+sv)*PADDING,
		       /* height */
		       BargraphPtr->borderWidth*2 +
		       tt*(lineHeight+PADDING) + 2*PADDING +
		       st*BargraphPtr->tickPixels +
		       BargraphPtr->width);
    /*
     * Calculate the offset in pixels the base value is from the left of the
     * window
     */
    BargraphPtr->basePixels = BargraphPtr->borderWidth + PADDING +
                              sv*(BargraphPtr->valuePixels + PADDING) +
                              BargraphPtr->max_height *
                              ( (basev - minv) / (maxv - minv) );

  } else {       /* the bargraph is horizontal */

    /*
     *  ____________________
     *  |\________________/|
     *  | |  Title (opt) | |
     *  | |--------------| |
     *  | |     |        | |
     *  | |     |   ###  | |
     *  | |Ticks|   ###  | |
     *  | |(opt)|   ###  | |
     *  | |     |   ###  | |
     *  | |     |   ###  | |
     *  | |     |   ###  | |
     *  | |--------------| |
     *  | |  Value (opt) | |
     *  |/----------------\|
     *  --------------------
     *       ^-- borderWidth
     */

    /* Calculate the extents for the ticks text */
    if ( st ) {
      sprintf(valueString, "%g", BargraphPtr->min_value);
      XTextExtents(BargraphPtr->fontPtr, valueString, strlen(valueString),
		   &dummy, &dummy, &dummy, &bbox);
      BargraphPtr->tickPixels = bbox.rbearing + bbox.lbearing;

      sprintf(valueString, "%g", BargraphPtr->max_value);
      XTextExtents(BargraphPtr->fontPtr, valueString, strlen(valueString),
		   &dummy, &dummy, &dummy, &bbox);
      if (BargraphPtr->tickPixels < bbox.rbearing + bbox.lbearing)
	BargraphPtr->tickPixels = bbox.rbearing + bbox.lbearing;
      BargraphPtr->tickPixels += PADDING + TICK_LENGTH;
    }
    else
      BargraphPtr->tickPixels = 0;

    /*
     *  Value is always displayed directly below the bar, so the bar needs
     *  to be wide enough so that the value text isn't clipped.
     */
    BargraphPtr->width = max( BargraphPtr->width, BargraphPtr->valuePixels );

    Tk_GeometryRequest(BargraphPtr->tkwin,
		       /* width */
		       2*BargraphPtr->borderWidth +
		       BargraphPtr->width + BargraphPtr->tickPixels +
		       2*PADDING,
		       /* height */
		       2*BargraphPtr->borderWidth +
		       BargraphPtr->max_height + (tt+sv)*lineHeight +
		       (2+tt+sv)*PADDING );

    /*
     * Calculate the offset in pixels the base value is from the top of the
     * window
     */
    BargraphPtr->basePixels = BargraphPtr->borderWidth + PADDING +
                              tt*(lineHeight + PADDING) +
	                      BargraphPtr->max_height *
	                      ( 1.0 - (basev - minv) /
			              (maxv - minv) );
  }

  Tk_SetInternalBorder(BargraphPtr->tkwin, BargraphPtr->borderWidth);
}

/*
 *--------------------------------------------------------------
 *
 * DisplayVerticalBargraph --
 *
 *	This procedure redraws the contents of a vertical Bargraph
 *	window.  It is invoked as a do-when-idle handler, so it only
 *	runs when there's nothing else for the application to do.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information appears on the screen.
 *
 *--------------------------------------------------------------
 */

static void
DisplayVerticalBargraph(clientData)
    ClientData clientData;	/* Information about widget. */
{
  register Bargraph *BargraphPtr = (Bargraph *) clientData;
  register Tk_Window tkwin = BargraphPtr->tkwin;
  double value = BargraphPtr->value;
  int tt = 0, sv = 0, st = 0;
  double minv = BargraphPtr->min_value;
  double maxv = BargraphPtr->max_value;
  double basev = BargraphPtr->base_value;
  int bd = BargraphPtr->borderWidth;
  int win_width = Tk_Width(tkwin);

  /* Variable declarations used in the text (title and value) routines */
  XFontStruct *fp = BargraphPtr->fontPtr;
  XCharStruct bbox;
  char valueString[30];
  int strwidth;
  int x, dummy;
  int lineHeight = BargraphPtr->fontPtr->ascent +
                   BargraphPtr->fontPtr->descent;


  BargraphPtr->displaybits &= ~REDRAW_PENDING;
  if ((BargraphPtr->tkwin == NULL) || !Tk_IsMapped(tkwin)) {
    return;
  }

  if ( hasatitle(BargraphPtr) )         tt = 1;
  if ( BargraphPtr->showValue )         sv = 1;
  if ( BargraphPtr->tickInterval != 0 ) st = 1;

  /*
   *  Clear the window if necessary
   */
  if ( BargraphPtr->displaybits & CLEAR_NEEDED )
    XClearWindow( Tk_Display(tkwin), Tk_WindowId(tkwin) );

  /*
   *  Display the title, centered in the window if there is enough space.
   *  Otherwise left justified and clipped on the right.
   */
  if ( hasatitle(BargraphPtr) && BargraphPtr->displaybits & DISPLAY_TITLE ) {
    strwidth = XTextWidth( fp, BargraphPtr->title, strlen(BargraphPtr->title));
    XTextExtents(fp, BargraphPtr->title, strlen(BargraphPtr->title), &dummy,
		 &dummy, &dummy, &bbox);
    if ( strwidth < win_width - 2*bd )
      x = (win_width - (bbox.lbearing + bbox.rbearing))/2;
    else
      x = bd + PADDING;

    XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), bd, bd,
	       win_width - 2*bd, lineHeight + PADDING, False);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
		BargraphPtr->textGC, x, fp->max_bounds.ascent + bd,
		BargraphPtr->title, strlen(BargraphPtr->title));
  }

  /*
   *  Display the value, centered below the bar.
   */
  if ( BargraphPtr->showValue && BargraphPtr->displaybits & DISPLAY_VALUE ) {
    sprintf( valueString, "%g", value );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);

    XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin),
	       win_width - bd - PADDING - BargraphPtr->width,
	       Tk_Height(tkwin) - lineHeight - PADDING - bd,
	       BargraphPtr->width, lineHeight + PADDING, False); 
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		/* x */
		win_width - bd - PADDING -
		(BargraphPtr->width + (bbox.lbearing + bbox.rbearing))/2,
		/* y */
		/* Don't pad values as numbers don't have descenders */
		Tk_Height(tkwin) - fp->max_bounds.descent - bd,
		valueString, strlen(valueString));
  }

  /*
   *  Display the ticks
   */
  if ( BargraphPtr->tickInterval != 0 &&
      BargraphPtr->displaybits & DISPLAY_TICKS ) {
    /*
     * compute and draw the ticks
     */
    double pixel_interval = BargraphPtr->max_height *
                            BargraphPtr->tickInterval /
			    ((minv < maxv) ? (maxv-minv) : (minv-maxv));
    int num_ticks = BargraphPtr->max_height / (int)pixel_interval + 1;
    int x_offset = bd + PADDING + BargraphPtr->tickPixels - TICK_LENGTH;
    int i, start, end;

    if (BargraphPtr->ticks != NULL)
      free(BargraphPtr->ticks);

    /*
     *  Allocate the upper limit of ticks we will need
     */
    BargraphPtr->ticks=(XSegment *) malloc( (num_ticks+2) * sizeof(XSegment));

    start = min( minv, maxv );  end = max( minv, maxv );
    num_ticks = 0;

    for ( i = basev; i > start; i-=BargraphPtr->tickInterval, num_ticks++ ) {
      BargraphPtr->ticks[num_ticks].x1 = x_offset;
      BargraphPtr->ticks[num_ticks].x2 = x_offset + TICK_LENGTH;
      BargraphPtr->ticks[num_ticks].y1 =
	BargraphPtr->ticks[num_ticks].y2 = ValueToPixel(BargraphPtr, i);
    }

    BargraphPtr->ticks[num_ticks].x1 = x_offset;
    BargraphPtr->ticks[num_ticks].x2 = x_offset + TICK_LENGTH;
    BargraphPtr->ticks[num_ticks].y1 =
      BargraphPtr->ticks[num_ticks++].y2 = ValueToPixel(BargraphPtr, start);

    for ( i = basev+BargraphPtr->tickInterval; i < end;
	 i+=BargraphPtr->tickInterval, num_ticks++ ) {
      BargraphPtr->ticks[num_ticks].x1 = x_offset;
      BargraphPtr->ticks[num_ticks].x2 = x_offset + TICK_LENGTH;
      BargraphPtr->ticks[num_ticks].y1 =
	BargraphPtr->ticks[num_ticks].y2 = ValueToPixel(BargraphPtr, i);
    }

    BargraphPtr->ticks[num_ticks].x1 = x_offset;
    BargraphPtr->ticks[num_ticks].x2 = x_offset + TICK_LENGTH;
    BargraphPtr->ticks[num_ticks].y1 =
      BargraphPtr->ticks[num_ticks++].y2 = ValueToPixel(BargraphPtr, end);

    XDrawSegments( Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->tickGC,
		  BargraphPtr->ticks, num_ticks );

    /*
     * draw min_value and max_value
     */
    sprintf( valueString, "%g", maxv );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		bd + 2*PADDING + BargraphPtr->valuePixels - TICK_LENGTH -
			   (bbox.lbearing + bbox.rbearing),
		bd + tt*(lineHeight+PADDING) + PADDING + fp->max_bounds.ascent,
		valueString, strlen(valueString) );

    sprintf( valueString, "%g", minv );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		/* x */
		bd + 2*PADDING + BargraphPtr->valuePixels - TICK_LENGTH -
			   (bbox.lbearing + bbox.rbearing),
		/* y */
		Tk_Height(tkwin) - bd - sv*(lineHeight+PADDING) -
		PADDING + fp->max_bounds.ascent/2 - 1,
		valueString, strlen(valueString) );

    /*
     * draw basev if required
     */
    if ( basev != minv && basev != maxv ) {
      sprintf( valueString, "%g", basev );
      XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy,
		   &dummy, &bbox);
      XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		  /* x */
		  bd + PADDING + ( BargraphPtr->valuePixels - TICK_LENGTH -
			   (bbox.lbearing + bbox.rbearing) ),
		  /* y */
		  ValueToPixel(BargraphPtr, basev) + fp->max_bounds.descent,
		  valueString, strlen(valueString) );
    }
  }

  /*
   *  Limit the value of the bar so that it doesn't overwrite any other
   *  information in the window.  Have the test here so that the textual
   *  value isn't effected.
   */
  if ( maxv > minv ) {
    if ( value > maxv )
      value = maxv;
    else
      if ( value < minv )
	value = minv;
  } else {      /* maxv <= minv */
    if ( value < maxv )
      value = maxv;
    else
      if ( value > minv )
	value = minv;
  }

  /*
   *  Display the bar
   */
  if ( BargraphPtr->displaybits & DISPLAY_BAR ) {
    int height = BarHeight( BargraphPtr, value );
    int y = ValueToPixel( BargraphPtr, value );   /* y co-ordinate of value */

    /* Erase entire bar region */
    XClearArea( Tk_Display(tkwin), Tk_WindowId(tkwin),
	       /* x */
	       (Tk_Width(tkwin) + BargraphPtr->tickPixels -
		BargraphPtr->width)/2,
	       /* y */
	       bd + tt*(lineHeight+PADDING)+PADDING,
	       /* width */
	       BargraphPtr->width,
	       /* height */
	       BargraphPtr->max_height+1, False );

    if ( (minv < maxv && value < basev) || (maxv < minv && value > basev) )
      y -= height + 1;

    /* Draw bar */
#if TK_MAJOR_VERSION > 3
    Tk_Fill3DRectangle( tkwin, Tk_WindowId(tkwin),
		       BargraphPtr->barBorder, 
		       /* x */
		       (Tk_Width(tkwin) + BargraphPtr->tickPixels -
			BargraphPtr->width)/2,
		       y,
		       /* width */
		       BargraphPtr->width,
		       height,
		       BargraphPtr->barborderWidth, BargraphPtr->barrelief );
#else
    Tk_Fill3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
		       BargraphPtr->barBorder, 
		       /* x */
		       (Tk_Width(tkwin) + BargraphPtr->tickPixels -
			BargraphPtr->width)/2,
		       y,
		       /* width */
		       BargraphPtr->width,
		       height,
		       BargraphPtr->barborderWidth, BargraphPtr->barrelief );
#endif
  }

  /*
   *  Display the border
   */
  if ( BargraphPtr->displaybits & DISPLAY_BORDER )
#if TK_MAJOR_VERSION > 3
    Tk_Draw3DRectangle( tkwin, Tk_WindowId(tkwin),
		       BargraphPtr->Border, 0, 0, Tk_Width(tkwin),
		       Tk_Height(tkwin), BargraphPtr->borderWidth,
		       BargraphPtr->relief);
#else
    Tk_Draw3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
		       BargraphPtr->Border, 0, 0, Tk_Width(tkwin),
		       Tk_Height(tkwin), BargraphPtr->borderWidth,
		       BargraphPtr->relief);
#endif

  if ( BargraphPtr->displaybits & CLEAR_NEEDED )
    BargraphPtr->displaybits &= ~CLEAR_NEEDED;
}

/*
 *--------------------------------------------------------------
 *
 * DisplayHorizontalBargraph --
 *
 *	This procedure redraws the contents of a horizontal Bargraph
 *	window.  It is invoked as a do-when-idle handler, so it only
 *	runs when there's nothing else for the application to do.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Information appears on the screen.
 *
 *--------------------------------------------------------------
 */

static void
DisplayHorizontalBargraph(clientData)
    ClientData clientData;	/* Information about widget. */
{
  register Bargraph *BargraphPtr = (Bargraph *) clientData;
  register Tk_Window tkwin = BargraphPtr->tkwin;
  double value = BargraphPtr->value;
  int tt = 0, sv = 0, st = 0;
  double minv = BargraphPtr->min_value;
  double maxv = BargraphPtr->max_value;
  double basev = BargraphPtr->base_value;
  int bd = BargraphPtr->borderWidth;
  int win_width = Tk_Width(tkwin);

  /* Variable declarations used in the text (title and value) routines */
  XFontStruct *fp = BargraphPtr->fontPtr;
  XCharStruct bbox;
  char valueString[30];
  int strwidth;
  int x, dummy;
  int lineHeight = BargraphPtr->fontPtr->ascent +
                   BargraphPtr->fontPtr->descent;


  BargraphPtr->displaybits &= ~REDRAW_PENDING;
  if ((BargraphPtr->tkwin == NULL) || !Tk_IsMapped(tkwin)) {
    return;
  }

  if ( hasatitle(BargraphPtr) )         tt = 1;
  if ( BargraphPtr->showValue )         sv = 1;
  if ( BargraphPtr->tickInterval != 0 ) st = 1;

  /*
   *  Clear the window if necessary
   */
  if ( BargraphPtr->displaybits & CLEAR_NEEDED )
    XClearWindow( Tk_Display(tkwin), Tk_WindowId(tkwin) );

  /*
   *  Display the title, centered in the window if there is enough space.
   *  Otherwise left justified and clipped on the right.
   */
  if ( hasatitle(BargraphPtr) && BargraphPtr->displaybits & DISPLAY_TITLE ) {
    strwidth = XTextWidth( fp, BargraphPtr->title, strlen(BargraphPtr->title));
    XTextExtents(fp, BargraphPtr->title, strlen(BargraphPtr->title), &dummy,
		 &dummy, &dummy, &bbox);
    if ( strwidth < win_width - 2*bd )
      x = (win_width - (bbox.lbearing + bbox.rbearing))/2;
    else
      x = bd + PADDING;

    XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), bd, bd,
	       win_width - 2*bd, lineHeight + PADDING, False);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
		BargraphPtr->textGC, x, fp->max_bounds.ascent + bd,
		BargraphPtr->title, strlen(BargraphPtr->title));
  }

  /*
   *  Display the value, right justified to the left of the bar and centered
   *  within the bar.
   */
  if ( BargraphPtr->showValue && BargraphPtr->displaybits & DISPLAY_VALUE ) {
    sprintf( valueString, "%g", value );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);

    XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin),
	       bd + PADDING,
	       Tk_Height(tkwin) - bd - PADDING -
	       (BargraphPtr->width + lineHeight)/2,
	       BargraphPtr->valuePixels, lineHeight + PADDING, False); 
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		/* x */
		bd + PADDING + BargraphPtr->valuePixels -
		(bbox.lbearing + bbox.rbearing),
		/* y */
		Tk_Height(tkwin) - bd - PADDING + lineHeight -
		(BargraphPtr->width + lineHeight)/2,
		valueString, strlen(valueString));
  }

  /*
   *  Display the ticks
   */

  if ( BargraphPtr->tickInterval != 0 &&
      BargraphPtr->displaybits & DISPLAY_TICKS ) {
    /*
     * compute and draw the ticks
     */
    double pixel_interval = BargraphPtr->max_height *
                            BargraphPtr->tickInterval /
			    ((minv < maxv) ? (maxv-minv) : (minv-maxv));
    int num_ticks = BargraphPtr->max_height / (int)pixel_interval + 1;
    int y_offset = bd + PADDING + tt*(lineHeight+PADDING) +
                   BargraphPtr->tickPixels - TICK_LENGTH;
    int i, start, end;

    if (BargraphPtr->ticks != NULL)
      free(BargraphPtr->ticks);

    /*
     *  Allocate the upper limit of ticks we will need
     */
    BargraphPtr->ticks=(XSegment *) malloc( (num_ticks+2) * sizeof(XSegment));

    start = min( minv, maxv );  end = max( minv, maxv );
    num_ticks = 0;

    for ( i = basev; i > start; i-=BargraphPtr->tickInterval, num_ticks++ ) {
      BargraphPtr->ticks[num_ticks].y1 = y_offset;
      BargraphPtr->ticks[num_ticks].y2 = y_offset + TICK_LENGTH;
      BargraphPtr->ticks[num_ticks].x1 =
	BargraphPtr->ticks[num_ticks].x2 = ValueToPixel(BargraphPtr, i);
    }

    BargraphPtr->ticks[num_ticks].y1 = y_offset;
    BargraphPtr->ticks[num_ticks].y2 = y_offset + TICK_LENGTH;
    BargraphPtr->ticks[num_ticks].x1 =
      BargraphPtr->ticks[num_ticks++].x2 = ValueToPixel(BargraphPtr, start);

    for ( i = basev+BargraphPtr->tickInterval; i < end;
	 i+=BargraphPtr->tickInterval, num_ticks++ ) {
      BargraphPtr->ticks[num_ticks].y1 = y_offset;
      BargraphPtr->ticks[num_ticks].y2 = y_offset + TICK_LENGTH;
      BargraphPtr->ticks[num_ticks].x1 =
	BargraphPtr->ticks[num_ticks].x2 = ValueToPixel(BargraphPtr, i);
    }

    BargraphPtr->ticks[num_ticks].y1 = y_offset;
    BargraphPtr->ticks[num_ticks].y2 = y_offset + TICK_LENGTH;
    BargraphPtr->ticks[num_ticks].x1 =
      BargraphPtr->ticks[num_ticks++].x2 = ValueToPixel(BargraphPtr, end);

    XDrawSegments( Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->tickGC,
		  BargraphPtr->ticks, num_ticks );

    /*
     * draw min_value and max_value
     */
    sprintf( valueString, "%g", maxv );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		/* x */
		Tk_Width(tkwin) - bd - PADDING -
		(bbox.lbearing + bbox.rbearing),
		/* y */
		bd + PADDING + tt*(lineHeight+PADDING) +
		BargraphPtr->tickPixels - TICK_LENGTH,
		valueString, strlen(valueString) );

    sprintf( valueString, "%g", minv );
    XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy, &dummy,
		 &bbox);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		/* x */
		bd + PADDING + BargraphPtr->valuePixels -
		(bbox.lbearing + bbox.rbearing)/2,
		/* y */
		bd + PADDING + tt*(lineHeight+PADDING) +
		BargraphPtr->tickPixels - TICK_LENGTH,
		valueString, strlen(valueString) );

    /*
     * draw basev if required
     */
    if ( basev != minv && basev != maxv ) {
      sprintf( valueString, "%g", basev );
      XTextExtents(fp, valueString, strlen(valueString), &dummy, &dummy,
		   &dummy, &bbox);
      XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin), BargraphPtr->textGC,
		  /* x */
		  ValueToPixel(BargraphPtr, basev) -
		  (bbox.lbearing + bbox.rbearing)/2,
		  /* y */
		  bd + PADDING + tt*(lineHeight+PADDING) +
		  BargraphPtr->tickPixels - TICK_LENGTH,
		  valueString, strlen(valueString) );
    }
  }

  /*
   *  Limit the value of the bar so that it doesn't overwrite any other
   *  information in the window.  Have the test here so that the textual
   *  value isn't effected.
   */
  if ( maxv > minv ) {
    if ( value > maxv )
      value = maxv;
    else
      if ( value < minv )
	value = minv;
  } else {      /* maxv <= minv */
    if ( value < maxv )
      value = maxv;
    else
      if ( value > minv )
	value = minv;
  }

  /*
   *  Display the bar
   */

  if ( BargraphPtr->displaybits & DISPLAY_BAR ) {
    int height = BarHeight( BargraphPtr, value );
    int y = ValueToPixel( BargraphPtr, value );   /* y co-ordinate of value */

    /* Erase entire bar region */
    XClearArea( Tk_Display(tkwin), Tk_WindowId(tkwin),
	       /* x */
	       bd + PADDING + sv*(BargraphPtr->valuePixels + PADDING),
	       /* y */
	       Tk_Height(tkwin) - bd - PADDING - BargraphPtr->width,
	       /* width */
	       BargraphPtr->max_height+1,
	       /* height */
	       BargraphPtr->width, False );

    if ( (minv < maxv && value > basev) || (maxv < minv && value < basev) )
      y -= height + 1;

    /* Draw bar */
#if TK_MAJOR_VERSION > 3
    Tk_Fill3DRectangle( tkwin, Tk_WindowId(tkwin),
		       BargraphPtr->barBorder, 
		       /* x */
		       y,
		       /* y */
		       Tk_Height(tkwin) - bd - PADDING - BargraphPtr->width,
		       /* width */
		       height,
		       /* height */
		       BargraphPtr->width,
		       BargraphPtr->barborderWidth, BargraphPtr->barrelief );
#else
    Tk_Fill3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
		       BargraphPtr->barBorder, 
		       /* x */
		       y,
		       /* y */
		       Tk_Height(tkwin) - bd - PADDING - BargraphPtr->width,
		       /* width */
		       height,
		       /* height */
		       BargraphPtr->width,
		       BargraphPtr->barborderWidth, BargraphPtr->barrelief );
#endif
  }

  /*
   *  Display the border
   */
  if ( BargraphPtr->displaybits & DISPLAY_BORDER )
#if TK_MAJOR_VERSION > 3
    Tk_Draw3DRectangle( tkwin, Tk_WindowId(tkwin),
		       BargraphPtr->Border, 0, 0, Tk_Width(tkwin),
		       Tk_Height(tkwin), BargraphPtr->borderWidth,
		       BargraphPtr->relief);
#else
    Tk_Draw3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
		       BargraphPtr->Border, 0, 0, Tk_Width(tkwin),
		       Tk_Height(tkwin), BargraphPtr->borderWidth,
		       BargraphPtr->relief);
#endif

  if ( BargraphPtr->displaybits & CLEAR_NEEDED )
    BargraphPtr->displaybits &= ~CLEAR_NEEDED;
}

/*
 *----------------------------------------------------------------------
 *
 * BarHeight --
 *
 *	Given a value for the Bargraph, return the height of the
 *	bar.  The value must be restricted to lie within the
 *	defined range for the Bargraph.
 *
 * Results:
 *	An integer value giving the height of the bar.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int      BarHeight(BargraphPtr, value)
     Bargraph *BargraphPtr;
     double value;
{
  double basev = BargraphPtr->base_value;
  double minv = min( BargraphPtr->min_value, BargraphPtr->max_value );
  double maxv = max( BargraphPtr->min_value, BargraphPtr->max_value );
  int height;

  if (minv == maxv)
    height = BargraphPtr->max_height;
  else {
    height = BargraphPtr->max_height *
             ( fabs(( value - basev ) / ( maxv - minv )) );
    height++;
  }

  return height;
}

/*
 *----------------------------------------------------------------------
 *
 * ValueToPixel --
 *
 *	Given a reading of the Bargraph, return the x-coordinate or
 *	y-coordinate corresponding to that reading, depending on
 *	whether the Bargraph is vertical or horizontal, respectively.
 *      The value must be restricted to lie within the
 *	defined range for the Bargraph.
 *
 * Results:
 *	An integer value giving the pixel location corresponding
 *	to reading.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ValueToPixel(BargraphPtr, value)
    register Bargraph *BargraphPtr;	/* Information about widget. */
    double value;			/* Reading of the widget.    */
{
  int pixel = BargraphPtr->basePixels;
  double basev = BargraphPtr->base_value;

  if ( BargraphPtr->vertical )
    if ( BargraphPtr->grow_up )
      pixel += (value >= basev ? -1 : 1) * BarHeight( BargraphPtr, value );
    else
      pixel += (value >= basev ? 1 : -1) * BarHeight( BargraphPtr, value );
  else      /* bar is horizontal */
    if ( BargraphPtr->grow_up )
      pixel += (value >= basev ? 1 : -1) * BarHeight( BargraphPtr, value );
    else
      pixel += (value >= basev ? -1 : 1) * BarHeight( BargraphPtr, value );

  return pixel+1;
}

/*
 *--------------------------------------------------------------
 *
 * BargraphEventProc --
 *
 *	This procedure is invoked by the Tk dispatcher for various
 *	events on Bargraphs.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	When the window gets deleted, internal structures get
 *	cleaned up.  When it gets exposed, it is redisplayed.
 *
 *--------------------------------------------------------------
 */

static void
BargraphEventProc(clientData, eventPtr)
    ClientData clientData;	/* Information about window. */
    XEvent *eventPtr;		/* Information about event. */
{
  Bargraph *BargraphPtr = (Bargraph *) clientData;

  if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
    EventuallyRedrawBargraph(BargraphPtr, DISPLAY_ALL);
  } else if (eventPtr->type == DestroyNotify) {
    Tcl_DeleteCommand(BargraphPtr->interp, Tk_PathName(BargraphPtr->tkwin));
    BargraphPtr->tkwin = NULL;
    if (BargraphPtr->displaybits & DISPLAY_ALL) {
      if (BargraphPtr->vertical) {
	Tk_CancelIdleCall(DisplayVerticalBargraph, (ClientData) BargraphPtr);
      } else {
	Tk_CancelIdleCall(DisplayHorizontalBargraph,
			  (ClientData) BargraphPtr);
      }
    }
    Tk_EventuallyFree((ClientData) BargraphPtr, DestroyBargraph);
  } else if (eventPtr->type == ConfigureNotify) {
    ComputeBargraphGeometry(BargraphPtr);
  }
}

/*
 *--------------------------------------------------------------
 *
 * SetBargraphValue --
 *
 *	This procedure changes the value of a Bargraph and invokes
 *	a Tcl command to reflect the current position of a Bargraph
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A Tcl command is invoked, and an additional error-processing
 *	command may also be invoked.  The Bargraph's slider is redrawn.
 *
 *--------------------------------------------------------------
 */

static void
SetBargraphValue(BargraphPtr, value)
    register Bargraph *BargraphPtr;	/* Info about widget. */
    double value;		/* New value for Bargraph.  Gets
				 * adjusted if it's off the Bargraph. */
{
    int result;
    char string[20];

    if (value == BargraphPtr->value)
      return;

    BargraphPtr->value = value;
    EventuallyRedrawBargraph(BargraphPtr, DISPLAY_BAR | DISPLAY_VALUE);

}

/*
 *--------------------------------------------------------------
 *
 * EventuallyRedrawBargraph --
 *
 *	Arrange for part or all of a Bargraph widget to redrawn at
 *	the next convenient time in the future.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If "what" is DISPLAY_SLIDER then just the slider and the
 *	value readout will be redrawn;  if "what" is DISPLAY_ALL
 *	then the entire widget will be redrawn.
 *
 *--------------------------------------------------------------
 */

static void
EventuallyRedrawBargraph(BargraphPtr, what)
    register Bargraph *BargraphPtr;	/* Information about widget. */
    int what;			/* What to redraw */
{

  if ( (BargraphPtr->tkwin == NULL) || !Tk_IsMapped(BargraphPtr->tkwin) ||
       (BargraphPtr->displaybits & REDRAW_PENDING) ) {
    return;
  }

  BargraphPtr->displaybits = what | REDRAW_PENDING;
  if (BargraphPtr->vertical) {
    Tk_DoWhenIdle(DisplayVerticalBargraph, (ClientData) BargraphPtr);
  } else {
    Tk_DoWhenIdle(DisplayHorizontalBargraph, (ClientData) BargraphPtr);
  }

}

/*
 *--------------------------------------------------------------
 *
 * SwapColours --
 *
 *     Save the current colour scheme so that it may be
 *     restored at some later stage.
 *
 * Results:
 *     None.
 *
 * Side effects:
 *     Colour scheme is swapped and Bargraph is redisplayed with
 *     the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void SwapColours(Bargraph *BargraphPtr)
{
  Tk_3DBorder tempb;
  XColor     *tempc;

/*
 *  Swap a and b using c as the temporary variable.
 *  NOTE:  This only works because the reference counts to the XColors are
 *         unchanged because the pointers are swapped consistently.
 */
#define SWAP(a,b,c) { (c) = (a); (a) = (b); (b) = (c); }

  SWAP(BargraphPtr->altborder, BargraphPtr->Border, tempb);
  SWAP(BargraphPtr->altbarBorder, BargraphPtr->barBorder, tempb);

  SWAP(BargraphPtr->a_textColor, BargraphPtr->textColorPtr, tempc);
  SWAP(BargraphPtr->a_tickColor, BargraphPtr->tickColorPtr, tempc);

#undef SWAP

  ConfigureBargraph(BargraphPtr->interp, BargraphPtr, 0, NULL,
		    TK_CONFIG_ARGV_ONLY);
}

/*
 *--------------------------------------------------------------
 *
 * ReplaceColours --
 *
 *     Store the current colour scheme and replace it with
 *     the new one.
 *
 * Results:
 *     None.
 *
 * Side effects:
 *     Bargraph is displayed with the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void ReplaceColours(Bargraph *BargraphPtr, int argc, char *argv[])
{
#if TK_MAJOR_VERSION > 3
  BargraphPtr->altborder = Tk_Get3DBorder(BargraphPtr->interp,
					  BargraphPtr->tkwin,
					  Tk_NameOf3DBorder(BargraphPtr->Border));

  BargraphPtr->altbarBorder = Tk_Get3DBorder(BargraphPtr->interp,
					     BargraphPtr->tkwin,
					     Tk_NameOf3DBorder(BargraphPtr->barBorder));

  BargraphPtr->a_textColor = Tk_GetColorByValue(BargraphPtr->tkwin,
						BargraphPtr->textColorPtr);

  BargraphPtr->a_tickColor = Tk_GetColorByValue(BargraphPtr->tkwin,
						BargraphPtr->tickColorPtr);
#else
  BargraphPtr->altborder = Tk_Get3DBorder(BargraphPtr->interp,
					  BargraphPtr->tkwin,
					  (Colormap) None,
					  Tk_NameOf3DBorder(BargraphPtr->Border));

  BargraphPtr->altbarBorder = Tk_Get3DBorder(BargraphPtr->interp,
					     BargraphPtr->tkwin,
					     (Colormap) None,
					     Tk_NameOf3DBorder(BargraphPtr->barBorder));

  BargraphPtr->a_textColor = Tk_GetColorByValue(BargraphPtr->interp,
						BargraphPtr->tkwin,
						(Colormap) None,
						BargraphPtr->textColorPtr);

  BargraphPtr->a_tickColor = Tk_GetColorByValue(BargraphPtr->interp,
						BargraphPtr->tkwin,
						(Colormap) None,
						BargraphPtr->tickColorPtr);
#endif

  ConfigureBargraph(BargraphPtr->interp, BargraphPtr, argc, argv,
		    TK_CONFIG_ARGV_ONLY);
}
