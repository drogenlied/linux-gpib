/* 
 * tkStripchart.c --
 *
 *	This module implements "Stripchart" widgets for the Tk
 *	toolkit.
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

/*
 *  BUGS:
 *   sometimes the callback procedure doesn't work (????)
 *    (it can often be made to work by setting a (different) value and
 *     restarting the callback procedure.)
 *
 *  CHANGES:
 *   would be nicer having tick interval instead of numticks.
 *
 */

#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <tk.h>
#if TK_MAJOR_VERSION < 4
#  include <tkConfig.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 *  Default stripchart configuration values
 */

#define DEF_STRIPCHART_BG_COLOR            "gray77"
#define DEF_STRIPCHART_STRIP_COLOR         "red"
#define DEF_STRIPCHART_TEXT_COLOR          "black"
#define DEF_STRIPCHART_TICK_COLOR          "blue2"
#define DEF_STRIPCHART_CURSOR                 ((char *) NULL)

#define ALT_STRIPCHART_BG_COLOR            "red2"
#define ALT_STRIPCHART_STRIP_COLOR         "green"
#define ALT_STRIPCHART_TEXT_COLOR          "white"
#define ALT_STRIPCHART_TICK_COLOR          "gray70"

        /* for monochrome displays */

#define DEF_STRIPCHART_BG_MONO             "white"
#define DEF_STRIPCHART_STRIP_MONO          "black"
#define DEF_STRIPCHART_TEXT_MONO           "black"
#define DEF_STRIPCHART_TICK_MONO           "black"

#define ALT_STRIPCHART_BG_MONO             "black"
#define ALT_STRIPCHART_STRIP_MONO          "white"
#define ALT_STRIPCHART_TEXT_MONO           "white"
#define ALT_STRIPCHART_TICK_MONO           "white"

#define DEF_STRIPCHART_NUMSTRIPS           "40"
#define DEF_STRIPCHART_NUMTICKS           "11"
#define DEF_STRIPCHART_BORDER_WIDTH        "3"
#define DEF_STRIPCHART_FONT                "*-Helvetica-Bold-R-Normal-*-120-*"
#define DEF_STRIPCHART_MINVALUE            "0"
#define DEF_STRIPCHART_MAXVALUE            "1000"
#define DEF_STRIPCHART_CALLBACK_INTERVAL   "500"
#define DEF_STRIPCHART_TITLE               (char *)NULL
#define DEF_STRIPCHART_HEIGHT              "80"
#define DEF_STRIPCHART_RELIEF              "raised"
#define DEF_STRIPCHART_TICK_INTERVAL       "50"
#define DEF_STRIPCHART_WIDTH               "2"
#define DEF_STRIPCHART_STRIPBORDERWIDTH    "0"
#define DEF_STRIPCHART_STRIPRELIEF         "raised"
#define DEF_STRIPCHART_SHOWTICKS           "true"
#define DEF_USERBITS                       "0"
#define DEF_USERDATA                       (char *) NULL
#define DEF_GUARANTEE_DRAW                 "false"

#define  MAX_STRIPS              100
#define  MAX_TICKS               40
#define  PADDING                 2
#define  hasatitle(d)            (((d)->title != NULL) && \
                                  ((d)->title[0] != '\0'))
#define  abs(x)                  ( (x) < 0 ? -(x) : (x) )

/*
 * A data structure of the following type is kept for each
 * Stripchart that currently exists for this process:
 */


typedef struct strip_struct Stripchart;
struct strip_struct {
    Tk_Window tkwin;		/* Window that embodies the Stripchart.  NULL
				 * means that the window has been destroyed
				 * but the data structures haven't yet been
				 * cleaned up.*/
    Display *display;
    Tcl_Interp *interp;		/* Interpreter associated with
				 * widget.  Used to delete widget
				 * command.  */
    Tk_Uid screenName;		/* If this window isn't a toplevel window
				 * then this is NULL;  otherwise it gives
				 * the name of the screen on which window
				 * is displayed. */
    Tk_3DBorder border;		/* Structure used to draw 3-D border and
				 * background. */
    int borderWidth;		/* Width of 3-D border (if any). */
    int relief;			/* 3-d effect: TK_RELIEF_RAISED etc. */
    Cursor cursor;		/* Current cursor for window, or None.  -jules- */

    /*
     * strip stuff
     */
    Tk_3DBorder stripBorder;    /* Structure used to draw the strips */
    int stripBorderWidth;
    int stripRelief;
    int strip_width;            /* width of a strip */
    int max_height;             /* maximum height of a strip */
    int min_value;
    int max_value;              /* maximum value of a strip */
    int num_strips;             /* the number of strips */
    int num_ticks;              /* the number of ticks to display */
    int value[MAX_STRIPS];      /* the data to be displayed in strip form. */

    int stripstodisplay;        /* how many of the num_strips should be
				 * displayed. */
    int scrollrequired;
    int guarantee_draw;

    int grow_up;
    XFontStruct *fontPtr;       /* Information about text font, or NULL. */
    XColor *textColorPtr;       /* Color for drawing text. */
    GC textGC;                  /* GC for drawing text. */
    XColor *tickColorPtr;       /* Color for drawing ticks. */
    GC tickGC;                  /* GC for drawing ticks. */
    int showticks;

    /*
     * call back stuff
     */
    char *command;            /* Command used for callback. Malloc'ed. */
    int continue_callback;    /* boolean flag used to terminate the callback */
    unsigned long interval;   /* interval (mS) between callbacks             */
    int (*callback_func)(Tcl_Interp *interp,
			 Stripchart *stripPtr); /* Function to be invoked on callback. */

    char *title;              /* Label to display above of stripchart;
                               * NULL means don't display a
                               * title.  Malloc'ed. */

    int displaybits;	      /* Various flags;  see below for
			       * definitions. */
    int userbits;
    char *userdata;

    /*
     *  Alternative colour scheme
     */

    Tk_3DBorder altborder;        /* Structure used to draw border  */
    Tk_3DBorder altstripBorder;  /* structure used to draw strip  */
    XColor *a_textColor;          /* alternative text color         */
    XColor *a_tickColor;          /* alternative tick color         */

};


/*
 * displaybits for Stripcharts:
 *
 * REDRAW_PENDING:		Non-zero means a DoWhenIdle handler
 *				has already been queued to redraw
 *				this window.
 * CLEAR_NEEDED:		Need to clear the window when redrawing.
 * DISPLAY_TITLE:               Draw the title, if there is one.
 * DISPLAY_STRIPS:              Draw the strips.
 * DISPLAY_BORDER:              Draw the border.
 * DISPLAY_TICKS:               Draw the ticks.
 * DISPLAY_ALL:                 Display everything.
 */

#define   REDRAW_PENDING		1
#define   CLEAR_NEEDED	        	2
#define   DISPLAY_TITLE                 4
#define   DISPLAY_STRIPS                8
#define   DISPLAY_BORDER               16
#define   DISPLAY_TICKS                32
#define   DISPLAY_ALL                  (DISPLAY_TITLE | DISPLAY_STRIPS | \
					DISPLAY_BORDER | DISPLAY_TICKS)

static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_BORDER, "-altbackground", "altbackground", "Background",
        ALT_STRIPCHART_BG_COLOR, Tk_Offset(Stripchart, altborder),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altbackground", "altbackground", "Background",
        ALT_STRIPCHART_BG_MONO, Tk_Offset(Stripchart, altborder),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altbg", "altbackground", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altfg", "altstripcolor", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altsc", "altstripcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_BORDER, "-altstripcolor", "altstripcolor",
       "Foreground", ALT_STRIPCHART_STRIP_COLOR,
       Tk_Offset(Stripchart, altstripBorder), TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altstripcolor", "altstripcolor",
       "Foreground", ALT_STRIPCHART_STRIP_MONO,
       Tk_Offset(Stripchart, altstripBorder), TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altstripcolour", "altstripcolor",
       (char *) NULL, (char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-alttc", "alttickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
        ALT_STRIPCHART_TEXT_COLOR, Tk_Offset(Stripchart, a_textColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
        ALT_STRIPCHART_TEXT_MONO, Tk_Offset(Stripchart, a_textColor),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttextcolour", "alttextcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_STRIPCHART_TICK_COLOR, Tk_Offset(Stripchart, a_tickColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_STRIPCHART_TICK_MONO, Tk_Offset(Stripchart, a_tickColor),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttickcolour", "alttickcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif

    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_STRIPCHART_BG_COLOR, Tk_Offset(Stripchart, border),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_STRIPCHART_BG_MONO, Tk_Offset(Stripchart, border),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_INT, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_STRIPCHART_BORDER_WIDTH, Tk_Offset(Stripchart, borderWidth), 0},
    {TK_CONFIG_STRING, "-command", "command", "Command",
        (char *) NULL, Tk_Offset(Stripchart, command), 0},
    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
       DEF_STRIPCHART_CURSOR, Tk_Offset(Stripchart, cursor),
       TK_CONFIG_NULL_OK}, 

    {TK_CONFIG_STRING, "-data", "data", "Data",
        DEF_USERDATA, Tk_Offset(Stripchart, userdata), 0},
    {TK_CONFIG_SYNONYM, "-fg", "stripcolor", (char *) NULL,
        (char *) NULL, 0, 0},
    {TK_CONFIG_FONT, "-font", "font", "Font",
        DEF_STRIPCHART_FONT, Tk_Offset(Stripchart, fontPtr),
        0},
    {TK_CONFIG_BOOLEAN, "-guaranteedrawing", "guaranteedrawing",
        "Guaranteedrawing", DEF_GUARANTEE_DRAW,
        Tk_Offset(Stripchart, guarantee_draw), 0},
    {TK_CONFIG_INT, "-height", "height", "Height", DEF_STRIPCHART_HEIGHT,
        Tk_Offset(Stripchart, max_height), 0},
    {TK_CONFIG_INT, "-interval", "interval", "Interval",
        DEF_STRIPCHART_CALLBACK_INTERVAL, Tk_Offset(Stripchart, interval), 0},
    {TK_CONFIG_INT, "-max", "max", "Max",
        DEF_STRIPCHART_MAXVALUE, Tk_Offset(Stripchart, max_value), 0},
    {TK_CONFIG_INT, "-min", "min", "Min",
        DEF_STRIPCHART_MINVALUE, Tk_Offset(Stripchart, min_value), 0},
    {TK_CONFIG_INT, "-numstrips", "numstrips", "Numstrips",
        DEF_STRIPCHART_NUMSTRIPS, Tk_Offset(Stripchart, num_strips), 0},
    {TK_CONFIG_INT, "-numticks", "numticks", "Numticks",
        DEF_STRIPCHART_NUMTICKS, Tk_Offset(Stripchart, num_ticks), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_STRIPCHART_RELIEF, Tk_Offset(Stripchart, relief), 0},
    {TK_CONFIG_BOOLEAN, "-showticks", "showticks", "Showticks",
        DEF_STRIPCHART_SHOWTICKS, Tk_Offset(Stripchart, showticks), 0},
    {TK_CONFIG_INT, "-stripborderwidth", "stripborderwidth",
        "Stripborderwidth", DEF_STRIPCHART_STRIPBORDERWIDTH,
        Tk_Offset(Stripchart, stripBorderWidth), 0},
    {TK_CONFIG_BORDER, "-stripcolor", "stripcolor", "Stripcolor",
	DEF_STRIPCHART_STRIP_COLOR, Tk_Offset(Stripchart, stripBorder),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-stripcolor", "stripcolor", "Stripcolor",
	DEF_STRIPCHART_STRIP_MONO, Tk_Offset(Stripchart, stripBorder),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-stripcolour", "stripcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif
    {TK_CONFIG_RELIEF, "-striprelief", "striprelief", "Striprelief",
        DEF_STRIPCHART_STRIPRELIEF, Tk_Offset(Stripchart, stripRelief), 0},
    {TK_CONFIG_INT, "-stripwidth", "stripwidth", "Stripwidth",
        DEF_STRIPCHART_WIDTH, Tk_Offset(Stripchart, strip_width), 0},
    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Textcolor",
        DEF_STRIPCHART_TEXT_COLOR, Tk_Offset(Stripchart, textColorPtr),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Textcolor",
        DEF_STRIPCHART_TEXT_MONO, Tk_Offset(Stripchart, textColorPtr),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-textcolour", "textcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Tickcolor",
        DEF_STRIPCHART_TICK_COLOR, Tk_Offset(Stripchart, tickColorPtr),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Tickcolor",
        DEF_STRIPCHART_TICK_MONO, Tk_Offset(Stripchart, tickColorPtr),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_STRING, "-title", "title", "Title",
        DEF_STRIPCHART_TITLE, Tk_Offset(Stripchart, title), 0},
    {TK_CONFIG_BOOLEAN, "-up", "up", "Up", "true",
        Tk_Offset(Stripchart, grow_up), 0},
    {TK_CONFIG_INT, "-userbits", "userbits", "Userbits",
        DEF_USERBITS, Tk_Offset(Stripchart, userbits), 0},
    {TK_CONFIG_INT, "-width", "width", "Width", DEF_STRIPCHART_WIDTH,
        Tk_Offset(Stripchart, strip_width), 0},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static void     Callback _ANSI_ARGS_((Stripchart *StripchartPtr));
static void     ComputeStripchartGeometry _ANSI_ARGS_((
		    Stripchart *StripchartPtr));
static int	ConfigureStripchart _ANSI_ARGS_((Tcl_Interp *interp,
		    Stripchart *StripchartPtr, int argc, char **argv,
		    int flags));
static void	DestroyStripchart _ANSI_ARGS_((ClientData clientData));
static void	DisplayStripchart _ANSI_ARGS_((ClientData clientData));
static void     DrawStripi _ANSI_ARGS_((Stripchart *StripchartPtr, int i));
static void     EventuallyRedrawStripchart _ANSI_ARGS_((
		    Stripchart *StripchartPtr, int displaybits));
static void     ReplaceColours _ANSI_ARGS_((Stripchart *StripchartPtr,
					    int argc, char *argv[]));
static void     SetStripchartValue _ANSI_ARGS_((Stripchart *StripchartPtr,
                    int value));
static void     ScrollStrips _ANSI_ARGS_((Stripchart *StripchartPtr));
static void	StripchartEventProc _ANSI_ARGS_((ClientData clientData,
		    XEvent *eventPtr));
static int	StripchartWidgetCmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));
static void     SetStripchartValue _ANSI_ARGS_((Stripchart *StripchartPtr,
						int value));
static void     SwapColours _ANSI_ARGS_((Stripchart *StripchartPtr));

/*
 *--------------------------------------------------------------
 *
 * Tk_StripchartCmd --
 *
 *	This procedure is invoked to process the "Stripchart" and
 *	"toplevel" Tcl commands.  See the user documentation for
 *	details on what it does.
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
Tk_StripchartCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    Tk_Window new;
    register Stripchart *StripchartPtr;


    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " pathName ?options?\"", (char *) NULL);
	return TCL_ERROR;
    }

    /*
     * Create the window.
     */

    new = Tk_CreateWindowFromPath(interp, tkwin, argv[1], (char *)NULL);
    if (new == NULL) {
	return TCL_ERROR;
    }

    Tk_SetClass(new, "Stripchart");
    StripchartPtr = (Stripchart *) ckalloc(sizeof(Stripchart));
    StripchartPtr->tkwin = new;
    StripchartPtr->display = Tk_Display(new);
    StripchartPtr->interp = interp;
    StripchartPtr->border = NULL;
    StripchartPtr->stripBorder = NULL;
    StripchartPtr->fontPtr = NULL;
    StripchartPtr->textColorPtr = NULL;
    StripchartPtr->textGC = None;
    StripchartPtr->tickColorPtr = NULL;
    StripchartPtr->tickGC = None;
    StripchartPtr->cursor = None;	           /* -jules- */

    StripchartPtr->altborder = NULL;
    StripchartPtr->altstripBorder = NULL;
    StripchartPtr->a_textColor = NULL;
    StripchartPtr->a_tickColor = NULL;

    StripchartPtr->command = NULL;
    StripchartPtr->userdata = NULL;
    StripchartPtr->callback_func = NULL;
    
    StripchartPtr->title = NULL;
    StripchartPtr->stripstodisplay = 0;
    StripchartPtr->scrollrequired = 0;
    StripchartPtr->displaybits = 0;

    Tk_CreateEventHandler(StripchartPtr->tkwin,
			  ExposureMask|StructureNotifyMask,
			  StripchartEventProc, (ClientData) StripchartPtr);
    Tcl_CreateCommand(interp, Tk_PathName(StripchartPtr->tkwin),
		      StripchartWidgetCmd, (ClientData) StripchartPtr,
		      (void (*)()) NULL);

    if (ConfigureStripchart(interp, StripchartPtr, argc-2, argv+2, 0)
	!= TCL_OK) {
	Tk_DestroyWindow(StripchartPtr->tkwin);
	return TCL_ERROR;
    }

    interp->result = Tk_PathName(StripchartPtr->tkwin);
    return TCL_OK;
}

/*
 *--------------------------------------------------------------
 *
 * StripchartWidgetCmd --
 *
 *	This procedure is invoked to process the Tcl command
 *	that corresponds to a Stripchart widget.  See the user
 *	documentation for details on what it does.
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
StripchartWidgetCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Information about Stripchart widget. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
  register Stripchart *StripchartPtr = (Stripchart *) clientData;
  int result = TCL_OK;
  int length;
  char c;
  
  if (argc < 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"",
		     argv[0], " option ?arg arg ...?\"", (char *) NULL);
    return TCL_ERROR;
  }
  Tk_Preserve((ClientData) StripchartPtr);
  c = argv[1][0];
  length = strlen(argv[1]);
  if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)) {
    if (argc == 2) {
      result = Tk_ConfigureInfo(interp, StripchartPtr->tkwin, configSpecs,
				(char *) StripchartPtr, (char *) NULL, 0);
    } else if (argc == 3) {
      result = Tk_ConfigureInfo(interp, StripchartPtr->tkwin, configSpecs,
				(char *) StripchartPtr, argv[2], 0);
    } else {
      result = ConfigureStripchart(interp, StripchartPtr, argc-2, argv+2,
				   TK_CONFIG_ARGV_ONLY);
    }
  } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
    if (argc != 2) {
      Tcl_AppendResult(interp, "wrong # args: should be \"",
		       argv[0], " get\"", (char *) NULL);
      goto error;
    }
    sprintf(interp->result, "%d",
	    StripchartPtr->value[StripchartPtr->stripstodisplay-1]);
  } else if ((c == 's') && (strncmp(argv[1], "set", length) == 0)) {
    int value;

    if (argc != 3) {
      Tcl_AppendResult(interp, "wrong # args: should be \"",
		       argv[0], " set value\"", (char *) NULL);
      goto error;
    }
    if (Tcl_GetInt(interp, argv[2], &value) != TCL_OK) {
      goto error;
    }
    SetStripchartValue(StripchartPtr, value);
  } else if ((c == 's') && (strncmp(argv[1], "start", length) == 0)) {
    if (! StripchartPtr->continue_callback ) {
      StripchartPtr->continue_callback = TRUE;
      Callback(StripchartPtr);
    }
  } else if ((c == 's') && (strncmp(argv[1], "stop", length) == 0)) {
    StripchartPtr->continue_callback = FALSE;
  } else if ((c == 'r') && (strncmp(argv[1], "reset", length) == 0)) {
    StripchartPtr->stripstodisplay = 0;
    StripchartPtr->scrollrequired = FALSE;
    EventuallyRedrawStripchart(StripchartPtr, DISPLAY_ALL | CLEAR_NEEDED);
  } else if ((c == 's') && (strncmp(argv[1], "swapcolours", length) == 0)) {
    SwapColours(StripchartPtr);
  } else if ((c == 'r') && (strncmp(argv[1],"replacecolours",length) == 0)) {
    ReplaceColours(StripchartPtr, argc-2, argv+2);
  } else {
    Tcl_AppendResult(interp, "bad option \"", argv[1],
		     "\":  must be configure, get, set, reset, start or stop",
		     (char *) NULL);
    goto error;
  }

  Tk_Release((ClientData) StripchartPtr);
  return result;

 error:
  Tk_Release((ClientData) StripchartPtr);
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

static void Callback(StripchartPtr)
     Stripchart *StripchartPtr;
{
  int no_errors = TRUE;
  int result;

  if (StripchartPtr->callback_func != NULL) 
    SetStripchartValue(StripchartPtr,
		 (*(StripchartPtr->callback_func))(StripchartPtr->interp,StripchartPtr));

  if ( StripchartPtr->command != NULL && StripchartPtr->command[0] != '\0' ) {
    result = Tcl_Eval(StripchartPtr->interp, StripchartPtr->command);
    if ( result != TCL_OK ) {
      /*TkBindError(StripchartPtr->interp);*/
      no_errors = FALSE;
    }
  }

  if ( StripchartPtr->continue_callback && no_errors )
    Tk_CreateTimerHandler( StripchartPtr->interval, (void *)Callback,
                          (ClientData) StripchartPtr );

  return;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyStripchart --
 *
 *	This procedure is invoked by Tk_EventuallyFree or Tk_Release
 *	to clean up the internal structure of a Stripchart at a safe time
 *	(when no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the Stripchart is freed up.
 *
 *----------------------------------------------------------------------
 */

static void
DestroyStripchart(clientData)
    ClientData clientData;	/* Info about Stripchart widget. */
{
    register Stripchart *StripchartPtr = (Stripchart *) clientData;

    if (StripchartPtr->border != NULL)
      Tk_Free3DBorder(StripchartPtr->border);

    if (StripchartPtr->altborder != NULL)
      Tk_Free3DBorder(StripchartPtr->altborder);

    if (StripchartPtr->stripBorder != NULL)
      Tk_Free3DBorder(StripchartPtr->stripBorder);

    if (StripchartPtr->altstripBorder != NULL)
      Tk_Free3DBorder(StripchartPtr->altstripBorder);

    if (StripchartPtr->title != NULL)
      ckfree(StripchartPtr->title);

    if (StripchartPtr->command != NULL)
      ckfree(StripchartPtr->command);

    if (StripchartPtr->value != NULL)
      free(StripchartPtr->value);

    if (StripchartPtr->fontPtr != NULL)
        Tk_FreeFontStruct(StripchartPtr->fontPtr);

    if (StripchartPtr->textColorPtr != NULL)
        Tk_FreeColor(StripchartPtr->textColorPtr);

    if (StripchartPtr->textGC != None)
        Tk_FreeGC(StripchartPtr->display,StripchartPtr->textGC);

    if (StripchartPtr->tickColorPtr != NULL)
        Tk_FreeColor(StripchartPtr->tickColorPtr);

    if (StripchartPtr->tickGC != None)
        Tk_FreeGC(StripchartPtr->display,StripchartPtr->tickGC);

    if (StripchartPtr->a_textColor != NULL)
      Tk_FreeColor(StripchartPtr->a_textColor);

    if (StripchartPtr->a_tickColor != NULL)
      Tk_FreeColor(StripchartPtr->a_tickColor);

    if (StripchartPtr->cursor != None) {
	Tk_FreeCursor(StripchartPtr->display,StripchartPtr->cursor);	/* -jules- */
    }

    ckfree((char *) StripchartPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConfigureStripchart --
 *
 *	This procedure is called to process an argv/argc list, plus
 *	the Tk option database, in order to configure (or
 *	reconfigure) a Stripchart widget.
 *
 * Results:
 *	The return value is a standard Tcl result.  If TCL_ERROR is
 *	returned, then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as text string, colors, font,
 *	etc. get set for StripchartPtr;  old resources get freed, if there
 *	were any.
 *
 *----------------------------------------------------------------------
 */

static int
ConfigureStripchart(interp, StripchartPtr, argc, argv, flags)
     Tcl_Interp *interp;		/* Used for error reporting. */
     register Stripchart *StripchartPtr;  /* Information about widget. */
    int argc;			/* Number of valid entries in argv. */
    char **argv;		/* Arguments. */
    int flags;			/* Flags to pass to Tk_ConfigureWidget. */
{
  XGCValues gcValues;
  GC newGC;

  if (Tk_ConfigureWidget(interp, StripchartPtr->tkwin, configSpecs,
			 argc, argv, (char *) StripchartPtr, flags) != TCL_OK)
    return TCL_ERROR;

  Tk_SetBackgroundFromBorder(StripchartPtr->tkwin, StripchartPtr->border);

  gcValues.font = StripchartPtr->fontPtr->fid;
  gcValues.foreground = StripchartPtr->textColorPtr->pixel;
  newGC = Tk_GetGC(StripchartPtr->tkwin, GCForeground|GCFont, &gcValues);
  if (StripchartPtr->textGC != None) {
    Tk_FreeGC(StripchartPtr->display,StripchartPtr->textGC);
  }
  StripchartPtr->textGC = newGC;

  gcValues.foreground = StripchartPtr->tickColorPtr->pixel;
  newGC = Tk_GetGC(StripchartPtr->tkwin, GCForeground, &gcValues);
  if (StripchartPtr->tickGC != None) {
    Tk_FreeGC(StripchartPtr->display,StripchartPtr->tickGC);
  }
  StripchartPtr->tickGC = newGC;

  if (StripchartPtr->num_strips > MAX_STRIPS)
    StripchartPtr->num_strips = MAX_STRIPS;

  if (StripchartPtr->num_ticks > MAX_TICKS)
    StripchartPtr->num_ticks = MAX_TICKS;

  if (StripchartPtr->min_value > StripchartPtr->max_value) {
    int temp = StripchartPtr->min_value;
    StripchartPtr->min_value = StripchartPtr->max_value;
    StripchartPtr->max_value = temp;
  }

  /*
   * Recompute display-related information, and let the geometry
   * manager know how much space is needed now.
   */
  ComputeStripchartGeometry(StripchartPtr);

  EventuallyRedrawStripchart(StripchartPtr, DISPLAY_ALL | CLEAR_NEEDED);

  return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeStripchartGeometry --
 *
 *      This procedure is called to compute various geometrical
 *      information for a stripchart, such as where various things get
 *      displayed.  It's called when the window is reconfigured.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void
ComputeStripchartGeometry(StripchartPtr)
    register Stripchart *StripchartPtr;      /* Information about widget. */
{
  int tt = hasatitle(StripchartPtr);
  int bd = StripchartPtr->borderWidth;
  int lineHeight = StripchartPtr->fontPtr->ascent +
                   StripchartPtr->fontPtr->descent;

  Tk_GeometryRequest(StripchartPtr->tkwin,
		     /* width */
		     2*(bd + PADDING) + StripchartPtr->num_strips *
		     StripchartPtr->strip_width,
		     /* height */
		     2*(bd + PADDING) + tt*(lineHeight + PADDING) +
		     StripchartPtr->max_height );

  Tk_SetInternalBorder(StripchartPtr->tkwin, StripchartPtr->borderWidth);
}

/*
 *----------------------------------------------------------------------
 *
 * DisplayStripchart --
 *
 *	This procedure is invoked to display a Stripchart widget.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Commands are output to X to display the Stripchart in its
 *	current mode.
 *
 *----------------------------------------------------------------------
 */

static void
DisplayStripchart(clientData)
    ClientData clientData;	/* Information about widget. */
{
  register Stripchart *StripchartPtr = (Stripchart *) clientData;
  register Tk_Window tkwin = StripchartPtr->tkwin;
  int bd = StripchartPtr->borderWidth;
  int i, tt = hasatitle(StripchartPtr);

  /* Variable declarations used in the title drawing routines */
  XFontStruct *fp = StripchartPtr->fontPtr;
  XCharStruct bbox;
  int x, dummy;
  int lineHeight = StripchartPtr->fontPtr->ascent +
                   StripchartPtr->fontPtr->descent;


  StripchartPtr->displaybits &= ~REDRAW_PENDING;
  if ((StripchartPtr->tkwin == NULL) || !Tk_IsMapped(tkwin))
    return;

  /*
   *  Clear the window if necessary
   */
  if ( StripchartPtr->displaybits & CLEAR_NEEDED )
    XClearWindow( Tk_Display(tkwin), Tk_WindowId(tkwin) );

  /*
   *  Display the title, centered in the window if there is enough space.
   *  Otherwise left justified and clipped on the right.
   */
  if (tt && StripchartPtr->displaybits & DISPLAY_TITLE ) {
    XTextExtents(fp, StripchartPtr->title, strlen(StripchartPtr->title),
		 &dummy, &dummy, &dummy, &bbox);
    if ( bbox.lbearing + bbox.rbearing < Tk_Width(tkwin) - 2*bd )
      x = (Tk_Width(tkwin) - (bbox.lbearing + bbox.rbearing))/2;
    else
      x = bd + PADDING;

    XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), bd, bd,
               Tk_Width(tkwin) - 2*bd, lineHeight + PADDING, False);
    XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
                StripchartPtr->textGC, x, fp->max_bounds.ascent + bd,
                StripchartPtr->title, strlen(StripchartPtr->title));
  }

  /*
   *  draw the strips
   */
  if (StripchartPtr->displaybits & CLEAR_NEEDED) {
    StripchartPtr->displaybits &= ~CLEAR_NEEDED;
    for(i=1; i<=StripchartPtr->stripstodisplay; i++)
      DrawStripi(StripchartPtr, i);
  }
  else {
    if (StripchartPtr->stripstodisplay == StripchartPtr->num_strips) {
      if (! StripchartPtr->scrollrequired)
	StripchartPtr->scrollrequired = TRUE;
      else
	ScrollStrips(StripchartPtr);
    }
    DrawStripi(StripchartPtr, StripchartPtr->stripstodisplay);
  }

  /*
   *  Display the border
   */
  if ( StripchartPtr->displaybits & DISPLAY_BORDER ) {
#if TK_MAJOR_VERSION > 3
    Tk_Draw3DRectangle( tkwin, Tk_WindowId(tkwin),
                       StripchartPtr->border, 0, 0, Tk_Width(tkwin),
                       Tk_Height(tkwin), StripchartPtr->borderWidth,
                       StripchartPtr->relief);
#else
    Tk_Draw3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
                       StripchartPtr->border, 0, 0, Tk_Width(tkwin),
                       Tk_Height(tkwin), StripchartPtr->borderWidth,
                       StripchartPtr->relief);
#endif
  }

}

/*
 *--------------------------------------------------------------
 *
 * StripchartEventProc --
 *
 *	This procedure is invoked by the Tk dispatcher on
 *	structure changes to a Stripchart.  For Stripcharts with 3D
 *	borders, this procedure is also invoked for exposures.
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
StripchartEventProc(clientData, eventPtr)
    ClientData clientData;	/* Information about window. */
    register XEvent *eventPtr;	/* Information about event. */
{
  register Stripchart *StripchartPtr = (Stripchart *) clientData;

  if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
    if ((StripchartPtr->relief != TK_RELIEF_FLAT) &&
	(StripchartPtr->tkwin != NULL) &&
	!(StripchartPtr->displaybits & REDRAW_PENDING)) {
      EventuallyRedrawStripchart(StripchartPtr, DISPLAY_ALL | CLEAR_NEEDED);
      /*  A clear isn't technically needed as the bitmap is clear when the
       *  window is exposed.  This flag is used to indicate that all strips
       *  need to be drawn, not just the most recent one.
       */
    }
  } else if (eventPtr->type == DestroyNotify) {
    Tcl_DeleteCommand(StripchartPtr->interp,
		      Tk_PathName(StripchartPtr->tkwin));
    StripchartPtr->tkwin = NULL;
    if (StripchartPtr->displaybits & REDRAW_PENDING) {
      Tk_CancelIdleCall(DisplayStripchart, (ClientData) StripchartPtr);
    }
    Tk_EventuallyFree((ClientData) StripchartPtr, DestroyStripchart);
  }
}

/*
 *--------------------------------------------------------------
 *
 * SetStripchartValue --
 *
 *      This procedure changes the value of a Stripchart and invokes
 *      a Tcl command to reflect the current position of a Stripchart
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      (Re) Sets the scrollrequired flag.
 *
 *--------------------------------------------------------------
 */

static void
SetStripchartValue(StripchartPtr, value)
    register Stripchart *StripchartPtr;     /* Info about widget. */
    int value;                  /* New value for Stripchart.  Gets
                                 * adjusted if it's off the Stripchart. */
{
  int i;
  int std = StripchartPtr->stripstodisplay;
  int maxvalues = StripchartPtr->num_strips;

  if (std != maxvalues) {
    StripchartPtr->value[std] = value;
    StripchartPtr->stripstodisplay++;
  }
  else {
    for (i=0; i<maxvalues-1; i++ )
      StripchartPtr->value[i] = StripchartPtr->value[i+1];
    StripchartPtr->value[maxvalues-1] = value;
  }

  EventuallyRedrawStripchart(StripchartPtr, DISPLAY_STRIPS);
}

/*
 *--------------------------------------------------------------
 *
 * EventuallyRedrawStripchart --
 *
 *      Arrange for part or all of a stripchart widget to redrawn at
 *      the next convenient time in the future.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void
EventuallyRedrawStripchart(StripchartPtr, displaybits)
   register Stripchart *StripchartPtr;     /* Information about widget. */
   int displaybits;
{

  if ( (StripchartPtr->tkwin == NULL) || !Tk_IsMapped(StripchartPtr->tkwin)
       || (StripchartPtr->displaybits & REDRAW_PENDING) ) {
    return;
  }

  StripchartPtr->displaybits = displaybits | REDRAW_PENDING;
  if (StripchartPtr->guarantee_draw)
    DisplayStripchart((ClientData) StripchartPtr);
  else
    Tk_DoWhenIdle(DisplayStripchart, (ClientData) StripchartPtr);
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
 *     Colour scheme is swapped and Stripchart is redisplayed with
 *     the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void SwapColours(Stripchart *StripchartPtr)
{
  Tk_3DBorder tempb;
  XColor     *tempc;

/*
 *  Swap a and b using c as the temporary variable.
 *  NOTE:  This only works because the reference counts to the XColors are
 *         unchanged because the pointers are swapped consistently.
 */
#define SWAP(a,b,c) { (c) = (a); (a) = (b); (b) = (c); }

  SWAP(StripchartPtr->altborder, StripchartPtr->border, tempb);
  SWAP(StripchartPtr->altstripBorder, StripchartPtr->stripBorder, tempb);

  SWAP(StripchartPtr->a_textColor, StripchartPtr->textColorPtr, tempc);
  SWAP(StripchartPtr->a_tickColor, StripchartPtr->tickColorPtr, tempc);

#undef SWAP

  ConfigureStripchart(StripchartPtr->interp, StripchartPtr, 0, NULL,
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
 *     Stripchart is displayed with the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void ReplaceColours(Stripchart *StripchartPtr, int argc, char *argv[])
{
#if TK_MAJOR_VERSION > 3
  StripchartPtr->altborder = Tk_Get3DBorder(StripchartPtr->interp,
					    StripchartPtr->tkwin,
					    Tk_NameOf3DBorder(StripchartPtr->border));

  StripchartPtr->altstripBorder = Tk_Get3DBorder(StripchartPtr->interp,
						 StripchartPtr->tkwin,
						 Tk_NameOf3DBorder(StripchartPtr->stripBorder));

  StripchartPtr->a_textColor = Tk_GetColorByValue(StripchartPtr->tkwin,
						  StripchartPtr->textColorPtr);

  StripchartPtr->a_tickColor = Tk_GetColorByValue(StripchartPtr->tkwin,
						  StripchartPtr->tickColorPtr);
#else
  StripchartPtr->altborder = Tk_Get3DBorder(StripchartPtr->interp,
					    StripchartPtr->tkwin,
					    (Colormap) None,
					    Tk_NameOf3DBorder(StripchartPtr->border));

  StripchartPtr->altstripBorder = Tk_Get3DBorder(StripchartPtr->interp,
						 StripchartPtr->tkwin,
						 (Colormap) None,
						 Tk_NameOf3DBorder(StripchartPtr->stripBorder));

  StripchartPtr->a_textColor = Tk_GetColorByValue(StripchartPtr->interp,
						  StripchartPtr->tkwin,
						  (Colormap) None,
						  StripchartPtr->textColorPtr);

  StripchartPtr->a_tickColor = Tk_GetColorByValue(StripchartPtr->interp,
						  StripchartPtr->tkwin,
						  (Colormap) None,
						  StripchartPtr->tickColorPtr);
#endif

  ConfigureStripchart(StripchartPtr->interp, StripchartPtr, argc, argv,
		      TK_CONFIG_ARGV_ONLY);
}

/*
 *--------------------------------------------------------------
 *
 * DrawStripi --
 *
 *     Draw the i-th strip of the stripchart.
 *
 * Results:
 *     None.
 *
 * Side effects:
 *     A new strip is drawn.
 *
 *--------------------------------------------------------------
 */
void DrawStripi(Stripchart *SPtr, int i)
{
  register Tk_Window tkwin = SPtr->tkwin;
  int lineHeight = SPtr->fontPtr->ascent + SPtr->fontPtr->descent;
  int x = SPtr->borderWidth + PADDING + (i-1)*SPtr->strip_width;
  int y = SPtr->borderWidth + PADDING +
              hasatitle(SPtr) * (lineHeight + PADDING);
  int w = SPtr->strip_width;
  int h;
  int maxv = SPtr->max_value;
  int minv = SPtr->min_value;
  XSegment ticks[MAX_TICKS];

  if (i < 1 || i > SPtr->num_strips)
    return;

  /* Clear any strip that might be below this one */
  XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), x, y, w, SPtr->max_height,
	     FALSE);

  /* Calculate the height of the bar */
  h = SPtr->max_height * (SPtr->value[i-1] - minv) / (maxv - minv);
  if ( h > SPtr->max_height )
    h = SPtr->max_height;
  if ( h == 0 )
    h = 1;

  /* Adject the origin of the bars rectangle depending on its origin */
  if (SPtr->grow_up)
    y += (SPtr->max_height - h);

  /* Draw the sucker */
#if TK_MAJOR_VERSION > 3
  Tk_Fill3DRectangle(tkwin, Tk_WindowId(tkwin),
		     SPtr->stripBorder, x, y, w, h,
		     SPtr->stripBorderWidth,
		     SPtr->stripRelief);
#else
  Tk_Fill3DRectangle(Tk_Display(tkwin), Tk_WindowId(tkwin),
		     SPtr->stripBorder, x, y, w, h,
		     SPtr->stripBorderWidth,
		     SPtr->stripRelief);
#endif

  /* Draw the ticks */
  if (SPtr->showticks)
    for ( i=0; i<SPtr->num_ticks; i++ ) {
      ticks[i].x1 = x;
      ticks[i].x2 = x + SPtr->strip_width - 1;
      ticks[i].y1 = ticks[i].y2 = SPtr->borderWidth + PADDING +
	            hasatitle(SPtr)*(lineHeight + PADDING) +
		    i*SPtr->max_height /(SPtr->num_ticks-1);
    }
  XDrawSegments(Tk_Display(tkwin), Tk_WindowId(tkwin), SPtr->tickGC, ticks,
		SPtr->num_ticks);
}

/*
 *--------------------------------------------------------------
 *
 * ScrollStrips --
 *
 *     Scroll the strips in the stripchart region to the left
 *     by the width of a strip pixels.
 *
 * Results:
 *     None.
 *
 * Side effects:
 *     Display changes.
 *
 *--------------------------------------------------------------
 */
void ScrollStrips(Stripchart *SPtr)
{
  register Tk_Window tkwin = SPtr->tkwin;
  int lineHeight = SPtr->fontPtr->ascent +  SPtr->fontPtr->descent;  
  int src_x = SPtr->borderWidth + PADDING + SPtr->strip_width;
  int src_y = SPtr->borderWidth + PADDING +
              hasatitle(SPtr) * (lineHeight + PADDING);
  int dest_x = src_x - SPtr->strip_width;
  int dest_y = src_y;
  int w = (SPtr->num_strips - 1) * SPtr->strip_width;
  int h = SPtr->max_height;

  XCopyArea(Tk_Display(tkwin), Tk_WindowId(tkwin), Tk_WindowId(tkwin),
                             /*   drawable src   ,    drawable dest    */
	    Tk_GetGC(tkwin, 0, NULL), src_x, src_y, w, h, dest_x, dest_y);
}


