/* 
 * tkTurndial.c --
 *
 *	This module implements a turndial widgets for the Tk toolkit.
 *	A turndial displays a knob that can be turned to change a
 *	value;  it also displays numeric labels and a textual label,
 *	if desired. The programmer's interface is very similar to the
 *	scale widget of the Tk toolkit.
 *
 *	The code is written for tk4.0b4. But if the flag TK36 is defined,
 *	the code is tk3.6 compatible.
 *
 * Copyright (c) 1995 Marco Beijersbergen (beijersb@rulhm1.leidenuniv.nl)
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * Based upon tkScale.c of the tk4.0b3 package:
 *   Copyright (c) 1990-1994 The Regents of the University of California.
 *   Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 *   See their text in the file "license.terms.tk" for information on
 *   usage and redistribution that came with that file,
 *   and for a DISCLAIMER OF ALL WARRANTIES on their behalf.
 */

static char sccsid[] = "@(#) tkTurndial.c 1.08 95/06/29 10:44:35";

#include <tk.h>

#if TK_MAJOR_VERSION < 4
#define TK36
#endif

#if 0
#ifndef TK36
#include "tkPort.h"
#else
#include "tkConfig.h"
#endif
#endif


#if 0
#include "tkInt.h"
#include "default.h"
#endif


/* defines from tkInt.h and default.h */

#define BLACK           "Black"
#define WHITE           "White"

#define NORMAL_BG       "#d9d9d9"
#define ACTIVE_BG       "#ececec"
#define SELECT_BG       "#c3c3c3"
#define TROUGH           "#c3c3c3"
#define INDICATOR       "#b03060"
#define DISABLED        "#a3a3a3"

extern Tk_Uid                        tkActiveUid;
extern Tk_Uid                        tkDisabledUid;
extern Tk_Uid                        tkNormalUid;

#include <math.h>
#include <stdlib.h>

#ifdef TK36
#define ACTIVE_BG	LIGHTPINK1
#define NORMAL_BG	BISQUE1
#else
#if !defined(ACTIVE_BG)
#define ACTIVE_BG       BISQUE2
#define NORMAL_BG	BISQUE1
#endif
#endif


/*
 * Defaults for turndials:
 */

#define DEF_TURNDIAL_ACTIVE_BG_COLOR	ACTIVE_BG
#define DEF_TURNDIAL_BG_COLOR		NORMAL_BG
#define DEF_TURNDIAL_HIGHLIGHT_BG	NORMAL_BG
#define DEF_TURNDIAL_KNOB_COLOR		NORMAL_BG


#define DEF_TURNDIAL_ACTIVE_BG_MONO	BLACK
#define DEF_TURNDIAL_BG_MONO		WHITE
#define DEF_TURNDIAL_BIG_INCREMENT	"0"
#define DEF_TURNDIAL_BORDER_WIDTH	"2"
#define DEF_TURNDIAL_COMMAND		""
#define DEF_TURNDIAL_CURSOR		""
#define DEF_TURNDIAL_DIGITS		"0"
#define DEF_TURNDIAL_FONT		"-Adobe-Helvetica-Bold-R-Normal--*-120-*-*-*-*-*-*"
#define DEF_TURNDIAL_FG_COLOR		BLACK
#define DEF_TURNDIAL_FG_MONO		BLACK
#define DEF_TURNDIAL_FROM		"0"
#define DEF_TURNDIAL_HIGHLIGHT		BLACK
#define DEF_TURNDIAL_HIGHLIGHT_WIDTH	"2"
#define DEF_TURNDIAL_LABEL		""
#define DEF_TURNDIAL_RELIEF		"flat"
#define DEF_TURNDIAL_REPEAT_DELAY	"300"
#define DEF_TURNDIAL_REPEAT_INTERVAL	"100"
#define DEF_TURNDIAL_RESOLUTION		"1"
#define DEF_TURNDIAL_TICK_COLOR		BLACK
#define DEF_TURNDIAL_TICK_MONO		BLACK
#define DEF_TURNDIAL_MARK_COLOR		WHITE
#define DEF_TURNDIAL_MARK_MONO		WHITE
#define DEF_TURNDIAL_SHOW_VALUE		"0"
#define DEF_TURNDIAL_STATE		"normal"
#define DEF_TURNDIAL_TAKE_FOCUS		(char *) NULL
#define DEF_TURNDIAL_MINORTICK_INTERVAL	"5"
#define DEF_TURNDIAL_TICK_INTERVAL	"20"
#define DEF_TURNDIAL_TO			"100"
#define DEF_TURNDIAL_VARIABLE		""
#define DEF_TURNDIAL_WIDTH		"15"
#define DEF_TURNDIAL_KNOB_MONO		BLACK
#define DEF_TURNDIAL_KNOB_BORDER_WIDTH	"3"
#define DEF_TURNDIAL_RADIUS		"20"
#define DEF_TURNDIAL_BEGIN_ANGLE	"-150"
#define DEF_TURNDIAL_END_ANGLE		"150"
#define DEF_TURNDIAL_SHOW_TAGS		"1"

#define MINORTICK_LENGTH 0.25	/* Length of minor tick (fract. of radius). */
#define TICK_LENGTH	0.40	/* Length of major tick (fract. of radius). */
#define MARK_LENGTH	0.80	/* Length of mark (fraction of radius). */
#define MARK_WIDTH	0.15	/* Width of mark (fraction of radius). */
#define KNOB_POINTS	24	/* Nr of points for the knob circle. */
#define MAX_TICKS	1000	/* Max nr of ticks to draw. */

#define deg_to_rad(x)			(M_PI*(x)/180.0)

/*
 * A data structure of the following type is kept for each turndial
 * widget managed by this file:
 */

typedef struct {
    Tk_Window tkwin;		/* Window that embodies the turndial.  NULL
				 * means that the window has been destroyed
				 * but the data structures haven't yet been
				 * cleaned up.*/
    Display *display;		/* Display containing widget.  Used, among
				 * other things, so that resources can be
				 * freed even after tkwin has gone away. */
    Tcl_Interp *interp;		/* Interpreter associated with turndial. */
#ifndef TK36
    Tcl_Command widgetCmd;	/* Token for turndial's widget command. */
#endif
    int radius;			/* Desired radius of central knob. */
    int beginAngle;		/* Desired starting angle in degrees. */
    int endAngle;		/* Desired end angle of scale in degrees. */
    double value;		/* Current value of turndial. */
    char *varName;		/* Name of variable (malloc'ed) or NULL.
				 * If non-NULL, turndial's value tracks
				 * the contents of this variable and
				 * vice versa. */
    double fromValue;		/* Value corresponding to left or top of
				 * turndial. */
    double toValue;		/* Value corresponding to right or bottom
				 * of turndial. */
    double minorTickInterval;	/* Distance between minor tick marks;
				 * 0 means don't display any minor ticks. */
    double tickInterval;	/* Distance between major ticks; 0 means
				   don't display any major ticks. */
    double resolution;		/* If > 0, all values are rounded to an
				 * even multiple of this value. */
    int digits;			/* Number of significant digits to print
				 * in values.  0 means we get to choose the
				 * number based on resolution and/or the
				 * range of the turndial. */
    char valueFormat[10];	/* Sprintf conversion specifier computed from
				 * digits and other information; used for
				 * the value. */
    char tagFormat[10];		/* Sprintf conversion specifier used for the
				 * values next to the major ticks. */
    double bigIncrement;	/* Amount to use for large increments to
				 * turndial value (0 means we pick a value). */
    char *command;		/* Command prefix to use when invoking Tcl
				 * commands because the turndial value
				 * changed. NULL means don't invoke commands.
				 * Malloc'ed. */
    int repeatDelay;		/* How long to wait before auto-repeating
				 * on scrolling actions (in ms). */
    int repeatInterval;		/* Interval between autorepeats (in ms). */
    char *label;		/* Label to display above the turndial;
				 * NULL means don't display a label.
				 * Malloc'ed. */
    int labelLength;		/* Number of non-NULL chars. in label. */
    Tk_Uid state;		/* Normal or disabled.  Value cannot be
				 * changed when turndial is disabled. */
    int showValue;		/* Non-zero means to display the turndial value
				 * below or to the left of the slider;  zero
				 * means don't display the value. */
    int showTags;		/* Non-zero means to display scale values along
				 * the major tick marks.
				 */


    /*
     * Information used when displaying widget:
     */

    int borderWidth;		/* Width of 3-D border around window. */
    Tk_3DBorder bgBorder;	/* Used for drawing background areas. */

    int knobBorderWidth;	/* Width of 3-D border around the knob. */
    Tk_3DBorder knobBorder;	/* For drawing the knob */
    Tk_3DBorder activeBorder;	/* For drawing the knob when active. */
    XPoint knob[KNOB_POINTS];	/* Points that make up the knob polygon. */

    XColor *tickColorPtr;	/* Color of all ticks. */
    GC tickGC;			/* GC for drawing ticks. */
    XColor *markColorPtr;	/* Color of the mark. */
    GC markGC;			/* GC for drawing the mark on the knob. */
    GC copyGC;			/* Used for copying from pixmap onto screen. */
    XFontStruct *fontPtr;	/* Information about text font, or NULL. */
    XColor *textColorPtr;	/* Color for drawing text. */
    GC textGC;			/* GC for drawing text in normal mode. */
    int relief;			/* Indicates whether window as a whole is
				 * raised, sunken, or flat. */
    int highlightWidth;		/* Width in pixels of highlight to draw
				 * around widget when it has the focus.
				 * <= 0 means don't draw a highlight. */
    XColor *highlightBgColorPtr;
				/* Color for drawing traversal highlight
				 * area when highlight is off. */
    XColor *highlightColorPtr;	/* Color for drawing traversal highlight. */
    int inset;			/* Total width of all borders, including
				 * traversal highlight and 3-D border.
				 * Indicates how much interior stuff must
				 * be offset from outside edges to leave
				 * room for borders. */
    /*
     * Layout information for turndials, assuming that window
     * gets the size it requested:
     */

    int centerX;		/* Location of the center of the knob */
    int centerY;		/* in pixels. */
    int minorTickInnerR;	/* Inner radius of minor tick marks. */
    int minorTickOuterR;	/* Outer radius of minor tick marks. */
    int tickInnerR;		/* Inner radius of major tick marks. */
    int tickOuterR;		/* Outer radius of major tick marks. */
    int tagR;			/* Inner radius of tags. */
    int labelY;			/* Top of label. */
    int valueY;			/* Top of value. */

    /*
     * Miscellaneous information:
     */

    Cursor cursor;		/* Current cursor for window, or None. */
    char *takeFocus;		/* Value of -takefocus option;  not used in
				 * the C code, but used by keyboard traversal
				 * scripts.  Malloc'ed, but may be NULL. */
    int flags;			/* Various flags;  see below for
				 * definitions. */
} Turndial;

/*
 * Flag bits for turndials:
 *
 * REDRAW_KNOB -		1 means mark on knob (and numerical readout)
 *				needs to be redrawn.
 * REDRAW_OTHER -		1 means other stuff besides mark and value
 *				need to be redrawn.
 * REDRAW_ALL -			1 means the entire widget needs to be redrawn.
 * ACTIVE -			1 means the widget is active (the mouse is
 *				in its window).
 * BUTTON_PRESSED -		1 means a button press is in progress, so
 *				mark should be draggable.
 * INVOKE_COMMAND -		1 means the turndial's command needs to be
 *				invoked during the next redisplay (the
 *				value of the turndial has changed since the
 *				last time the command was invoked).
 * SETTING_VAR -		1 means that the associated variable is
 *				being set by us, so there's no need for
 *				TurndialVarProc to do anything.
 * NEVER_SET -			1 means that the turndial's value has never
 *				been set before (so must invoke -command and
 *				set associated variable even if the value
 *				doesn't appear to have changed).
 * GOT_FOCUS -			1 means that the focus is currently in
 *				this widget.
 */

#define REDRAW_KNOB		1
#define REDRAW_OTHER		2
#define REDRAW_ALL		3
#define ACTIVE			4
#define BUTTON_PRESSED		8
#define INVOKE_COMMAND		0x10
#define SETTING_VAR		0x20
#define NEVER_SET		0x40
#define GOT_FOCUS		0x80

/*
 * Symbolic values for the active parts of a knob.  These are
 * the values that may be returned by the TurndialElement procedure.
 */

#define OTHER		0
#define KNOB		1
#define LEFT		2
#define RIGHT		3

/*
 * Space to leave between turndial area and text, and between text and
 * edge of window.
 */

#define SPACING 2

/*
 * How many characters of space to provide when formatting the
 * turndial's value:
 */

#define PRINT_CHARS 150

/*
 * Information used for argv parsing.
 */

static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_BORDER, "-activebackground", "activeBackground", "Foreground",
	DEF_TURNDIAL_ACTIVE_BG_COLOR, Tk_Offset(Turndial, activeBorder),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-activebackground", "activeBackground", "Foreground",
	DEF_TURNDIAL_ACTIVE_BG_MONO, Tk_Offset(Turndial, activeBorder),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_TURNDIAL_BG_COLOR, Tk_Offset(Turndial, bgBorder),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
	DEF_TURNDIAL_BG_MONO, Tk_Offset(Turndial, bgBorder),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_INT, "-beginangle", "beginAngle", "BeginAngle",
	DEF_TURNDIAL_BEGIN_ANGLE, Tk_Offset(Turndial, beginAngle), 0},
    {TK_CONFIG_DOUBLE, "-bigincrement", "bigIncrement", "BigIncrement",
	DEF_TURNDIAL_BIG_INCREMENT, Tk_Offset(Turndial, bigIncrement), 0},
    {TK_CONFIG_SYNONYM, "-bd", "borderWidth", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_PIXELS, "-borderwidth", "borderWidth", "BorderWidth",
	DEF_TURNDIAL_BORDER_WIDTH, Tk_Offset(Turndial, borderWidth), 0},
    {TK_CONFIG_STRING, "-command", "command", "Command",
	DEF_TURNDIAL_COMMAND, Tk_Offset(Turndial, command),
	TK_CONFIG_NULL_OK},
    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
	DEF_TURNDIAL_CURSOR, Tk_Offset(Turndial, cursor), TK_CONFIG_NULL_OK},
    {TK_CONFIG_INT, "-digits", "digits", "Digits",
	DEF_TURNDIAL_DIGITS, Tk_Offset(Turndial, digits), 0},
    {TK_CONFIG_INT, "-endangle", "endAngle", "EndAngle",
	DEF_TURNDIAL_END_ANGLE, Tk_Offset(Turndial, endAngle), 0},
    {TK_CONFIG_SYNONYM, "-fg", "foreground", (char *) NULL,
	(char *) NULL, 0, 0},
    {TK_CONFIG_FONT, "-font", "font", "Font",
	DEF_TURNDIAL_FONT, Tk_Offset(Turndial, fontPtr),
	0},
    {TK_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_TURNDIAL_FG_COLOR, Tk_Offset(Turndial, textColorPtr),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-foreground", "foreground", "Foreground",
	DEF_TURNDIAL_FG_MONO, Tk_Offset(Turndial, textColorPtr),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_DOUBLE, "-from", "from", "From",
	DEF_TURNDIAL_FROM, Tk_Offset(Turndial, fromValue), 0},
    {TK_CONFIG_COLOR, "-highlightbackground", "highlightBackground",
	"HighlightBackground", DEF_TURNDIAL_HIGHLIGHT_BG,
	Tk_Offset(Turndial, highlightBgColorPtr), 0},
    {TK_CONFIG_COLOR, "-highlightcolor", "highlightColor", "HighlightColor",
	DEF_TURNDIAL_HIGHLIGHT, Tk_Offset(Turndial, highlightColorPtr), 0},
    {TK_CONFIG_PIXELS, "-highlightthickness", "highlightThickness",
	"HighlightThickness",
	DEF_TURNDIAL_HIGHLIGHT_WIDTH, Tk_Offset(Turndial, highlightWidth), 0},
    {TK_CONFIG_PIXELS, "-knobborderwidth", "knobBorderWidth",
	"KnobBorderWidth", DEF_TURNDIAL_KNOB_BORDER_WIDTH,
	Tk_Offset(Turndial, knobBorderWidth), 0},
    {TK_CONFIG_BORDER, "-knobcolor", "knobColor", "KnobColor",
	DEF_TURNDIAL_KNOB_COLOR, Tk_Offset(Turndial, knobBorder),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-knobcolor", "knobColor", "KnobColor",
	DEF_TURNDIAL_KNOB_MONO, Tk_Offset(Turndial, knobBorder),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_STRING, "-label", "label", "Label",
	DEF_TURNDIAL_LABEL, Tk_Offset(Turndial, label), TK_CONFIG_NULL_OK},
    {TK_CONFIG_COLOR, "-markcolor", "markColor", "MarkColor",
	DEF_TURNDIAL_MARK_COLOR, Tk_Offset(Turndial, markColorPtr),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-markcolor", "markColor", "MarkColor",
	DEF_TURNDIAL_MARK_MONO, Tk_Offset(Turndial, markColorPtr),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_DOUBLE, "-minortickinterval", "minorTickInterval",
	"MinorTickInterval", DEF_TURNDIAL_MINORTICK_INTERVAL,
	Tk_Offset(Turndial, minorTickInterval), 0},
    {TK_CONFIG_PIXELS, "-radius", "radius", "Radius",
	DEF_TURNDIAL_RADIUS, Tk_Offset(Turndial, radius), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
	DEF_TURNDIAL_RELIEF, Tk_Offset(Turndial, relief), 0},
    {TK_CONFIG_INT, "-repeatdelay", "repeatDelay", "RepeatDelay",
	DEF_TURNDIAL_REPEAT_DELAY, Tk_Offset(Turndial, repeatDelay), 0},
    {TK_CONFIG_INT, "-repeatinterval", "repeatInterval", "RepeatInterval",
	DEF_TURNDIAL_REPEAT_INTERVAL, Tk_Offset(Turndial, repeatInterval), 0},
    {TK_CONFIG_DOUBLE, "-resolution", "resolution", "Resolution",
	DEF_TURNDIAL_RESOLUTION, Tk_Offset(Turndial, resolution), 0},
    {TK_CONFIG_BOOLEAN, "-showtags", "showTags", "ShowTags",
	DEF_TURNDIAL_SHOW_TAGS, Tk_Offset(Turndial, showTags), 0},
    {TK_CONFIG_BOOLEAN, "-showvalue", "showValue", "ShowValue",
	DEF_TURNDIAL_SHOW_VALUE, Tk_Offset(Turndial, showValue), 0},
    {TK_CONFIG_UID, "-state", "state", "State",
	DEF_TURNDIAL_STATE, Tk_Offset(Turndial, state), 0},
    {TK_CONFIG_STRING, "-takefocus", "takeFocus", "TakeFocus",
	DEF_TURNDIAL_TAKE_FOCUS, Tk_Offset(Turndial, takeFocus),
	TK_CONFIG_NULL_OK},
    {TK_CONFIG_COLOR, "-tickcolor", "tickColor", "TickColor",
	DEF_TURNDIAL_TICK_COLOR, Tk_Offset(Turndial, tickColorPtr),
	TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-tickcolor", "tickColor", "TickColor",
	DEF_TURNDIAL_TICK_MONO, Tk_Offset(Turndial, tickColorPtr),
	TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_DOUBLE, "-tickinterval", "tickInterval", "TickInterval",
	DEF_TURNDIAL_TICK_INTERVAL, Tk_Offset(Turndial, tickInterval), 0},
    {TK_CONFIG_DOUBLE, "-to", "to", "To",
	DEF_TURNDIAL_TO, Tk_Offset(Turndial, toValue), 0},
    {TK_CONFIG_STRING, "-variable", "variable", "Variable",
	DEF_TURNDIAL_VARIABLE, Tk_Offset(Turndial, varName),
	TK_CONFIG_NULL_OK},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0}
};

/*
 * Entry point to the turndial command; the only non-static
 * name in this module.
 */

int			Tk_TurndialCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
			    
/*
 * Forward declaration of functions used in this file:
 */

static void		ComputeFormat _ANSI_ARGS_((Turndial *turndialPtr));
static void		ComputeTurndialGeometry _ANSI_ARGS_((Turndial
			    *turndialPtr));
static int		ConfigureTurndial _ANSI_ARGS_((Tcl_Interp *interp,
			    Turndial *turndialPtr, int argc, char **argv,
			    int flags));
static void		DestroyTurndial _ANSI_ARGS_((ClientData clientData));
static void		DisplayTurndial _ANSI_ARGS_((ClientData clientData));
static void		DisplayTicks _ANSI_ARGS_((Turndial *turndialPtr,
			    Drawable drawable, int minor));
static void		DisplayTick _ANSI_ARGS_((Turndial *turndialPtr,
			    Drawable drawable, GC gc, double tickValue,
			    int innerR, int outerR, int tagR));
static void		DisplayValue _ANSI_ARGS_((Turndial *turndialPtr,
			    Drawable drawable, double value, int rightEdge));
static void		EventuallyRedrawTurndial _ANSI_ARGS_((Turndial
			    *turndialPtr, int what));
static double		PixelToValue _ANSI_ARGS_((Turndial *turndialPtr,
			    int x, int y));
static double		RoundToResolution _ANSI_ARGS_((Turndial *turndialPtr,
			    double value));
static void		TurndialCmdDeletedProc _ANSI_ARGS_((
			    ClientData clientData));
static int		TurndialElement _ANSI_ARGS_((Turndial *turndialPtr,
			    int x, int y));
static void		TurndialEventProc _ANSI_ARGS_((ClientData clientData,
			    XEvent *eventPtr));
static char *		TurndialVarProc _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, char *name1, char *name2,
			    int flags));
static int		TurndialWidgetCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char **argv));
static void		SetTurndialValue _ANSI_ARGS_((Turndial *turndialPtr,
			    double value, int setVar, int invokeCommand));
static double		ValueToAngle _ANSI_ARGS_((Turndial *turndialPtr,
			    double value));
static void		ValueToPixel _ANSI_ARGS_((Turndial *turndialPtr,
			    double value, int *xPtr, int *yPtr));

/*
 *--------------------------------------------------------------
 *
 * Tk_TurndialCmd --
 *
 *	This procedure is invoked to process the "turndial" Tcl
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
Tk_TurndialCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Main window associated with
				 * interpreter. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    char **argv;		/* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    register Turndial *turndialPtr;
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
     * Initialize fields that won't be initialized by ConfigureTurndial,
     * or which ConfigureTurndial expects to have reasonable values
     * (e.g. resource pointers).
     */

    turndialPtr = (Turndial *) ckalloc(sizeof(Turndial));
    turndialPtr->tkwin = new;
    turndialPtr->display = Tk_Display(new);
    turndialPtr->interp = interp;
#ifndef TK36
    turndialPtr->widgetCmd = Tcl_CreateCommand(interp,
	    Tk_PathName(turndialPtr->tkwin), TurndialWidgetCmd,
	    (ClientData) turndialPtr, TurndialCmdDeletedProc);
#else
    Tcl_CreateCommand(interp,
	    Tk_PathName(turndialPtr->tkwin), TurndialWidgetCmd,
	    (ClientData) turndialPtr, TurndialCmdDeletedProc);
#endif
    turndialPtr->radius = 0;
    turndialPtr->centerX = 0;
    turndialPtr->centerY = 0;
    turndialPtr->value = 0;
    turndialPtr->varName = NULL;
    turndialPtr->fromValue = 0;
    turndialPtr->toValue = 0;
    turndialPtr->minorTickInterval = 0;
    turndialPtr->tickInterval = 0;
    turndialPtr->resolution = 1;
    turndialPtr->bigIncrement = 0.0;
    turndialPtr->command = NULL;
    turndialPtr->repeatDelay = 0;
    turndialPtr->repeatInterval = 0;
    turndialPtr->label = NULL;
    turndialPtr->labelLength = 0;
    turndialPtr->state = tkNormalUid;

    turndialPtr->borderWidth = 0;
    turndialPtr->bgBorder = NULL;
    turndialPtr->knobBorderWidth = 0;
    turndialPtr->knobBorder = NULL;
    turndialPtr->activeBorder = NULL;
    turndialPtr->tickColorPtr = NULL;
    turndialPtr->tickGC = None;
    turndialPtr->markColorPtr = NULL;
    turndialPtr->markGC = None;
    turndialPtr->copyGC = None;
    turndialPtr->fontPtr = NULL;
    turndialPtr->textColorPtr = NULL;
    turndialPtr->textGC = None;
    turndialPtr->relief = TK_RELIEF_FLAT;
    turndialPtr->highlightWidth = 0;
    turndialPtr->highlightBgColorPtr = NULL;
    turndialPtr->highlightColorPtr = NULL;
    turndialPtr->inset = 0;
    turndialPtr->showValue = 0;
    turndialPtr->minorTickInnerR = 0;
    turndialPtr->minorTickOuterR = 0;
    turndialPtr->tickInnerR = 0;
    turndialPtr->tickOuterR = 0;
    turndialPtr->tagR = 0;
    turndialPtr->labelY = 0;
    turndialPtr->valueY = 0;
    turndialPtr->cursor = None;
    turndialPtr->takeFocus = NULL;
    turndialPtr->flags = NEVER_SET;

    Tk_SetClass(turndialPtr->tkwin, "Turndial");
    Tk_CreateEventHandler(turndialPtr->tkwin,
/*@*/
#if 0
#ifndef TK36
	    ExposureMask|StructureNotifyMask|FocusChangeMask,
#else
	    ExposureMask|StructureNotifyMask|FocusChangeMask|EnterWindowMask|
	    LeaveWindowMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
#endif
#else
	    ExposureMask|StructureNotifyMask|FocusChangeMask|EnterWindowMask|
	    LeaveWindowMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
#endif


            TurndialEventProc, (ClientData) turndialPtr);
    if (ConfigureTurndial(interp, turndialPtr, argc-2, argv+2, 0) != TCL_OK) {
	goto error;
    }

    interp->result = Tk_PathName(turndialPtr->tkwin);
    return TCL_OK;

    error:
    Tk_DestroyWindow(turndialPtr->tkwin);
    return TCL_ERROR;
}

/*
 *--------------------------------------------------------------
 *
 * TurndialWidgetCmd --
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
TurndialWidgetCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Information about turndial
					 * widget. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register Turndial *turndialPtr = (Turndial *) clientData;
    int result = TCL_OK;
    size_t length;
    int c;

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"",
		argv[0], " option ?arg arg ...?\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tk_Preserve((ClientData) turndialPtr);
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "cget", length) == 0)
	    && (length >= 2)) {
#ifndef TK36
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cget option\"",
		    (char *) NULL);
	    goto error;
	}
	result = Tk_ConfigureValue(interp, turndialPtr->tkwin, configSpecs,
		(char *) turndialPtr, argv[2], 0);
#endif
    } else if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)
	    && (length >= 3)) {
	if (argc == 2) {
	    result = Tk_ConfigureInfo(interp, turndialPtr->tkwin, configSpecs,
		    (char *) turndialPtr, (char *) NULL, 0);
	} else if (argc == 3) {
	    result = Tk_ConfigureInfo(interp, turndialPtr->tkwin, configSpecs,
		    (char *) turndialPtr, argv[2], 0);
	} else {
	    result = ConfigureTurndial(interp, turndialPtr, argc-2, argv+2,
		    TK_CONFIG_ARGV_ONLY);
	}
    } else if ((c == 'c') && (strncmp(argv[1], "coords", length) == 0)
	    && (length >= 3)) {
	int x, y ;
	double value;

	if ((argc != 2) && (argc != 3)) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " coords ?value?\"", (char *) NULL);
	    goto error;
	}
	if (argc == 3) {
	    if (Tcl_GetDouble(interp, argv[2], &value) != TCL_OK) {
		goto error;
	    }
	} else {
	    value = turndialPtr->value;
	}
	ValueToPixel(turndialPtr, value, &x, &y);
	sprintf(interp->result, "%d %d", x, y);
    } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
	double value;
	int x, y;

	if ((argc != 2) && (argc != 4)) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " get ?x y?\"", (char *) NULL);
	    goto error;
	}
	if (argc == 2) {
	    value = turndialPtr->value;
	} else {
	    if ((Tcl_GetInt(interp, argv[2], &x) != TCL_OK)
		    || (Tcl_GetInt(interp, argv[3], &y) != TCL_OK)) {
		goto error;
	    }
	    value = PixelToValue(turndialPtr, x, y);
	}
	sprintf(interp->result, turndialPtr->valueFormat, value);
    } else if ((c == 'i') && (strncmp(argv[1], "identify", length) == 0)) {
	int x, y, thing;

	if (argc != 4) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " identify x y\"", (char *) NULL);
	    goto error;
	}
	if ((Tcl_GetInt(interp, argv[2], &x) != TCL_OK)
		|| (Tcl_GetInt(interp, argv[3], &y) != TCL_OK)) {
	    goto error;
	}
	thing = TurndialElement(turndialPtr, x,y);
	switch (thing) {
	    case KNOB:		interp->result = "knob";	break;
	    case LEFT:		interp->result = "left";	break;
	    case RIGHT:		interp->result = "right";	break;
	}
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
	if (turndialPtr->state != tkDisabledUid) {
	    SetTurndialValue(turndialPtr, value, 1, 1);
	}
    } else {
	Tcl_AppendResult(interp, "bad option \"", argv[1],
		"\": must be cget, configure, coords, get, identify, or set",
		(char *) NULL);
	goto error;
    }
    Tk_Release((ClientData) turndialPtr);
    return result;

    error:
    Tk_Release((ClientData) turndialPtr);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyTurndial --
 *
 *	This procedure is invoked by Tk_EventuallyFree or Tk_Release
 *	to clean up the internal structure of a button at a safe time
 *	(when no-one is using it anymore).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Everything associated with the turndial is freed up.
 *
 *----------------------------------------------------------------------
 */

static void
DestroyTurndial(clientData)
    ClientData clientData;	/* Info about turndial widget. */
{
    register Turndial *turndialPtr = (Turndial *) clientData;

    /*
     * Free up all the stuff that requires special handling, then
     * let Tk_FreeOptions handle all the standard option-related
     * stuff.
     */

    if (turndialPtr->varName != NULL) {
	Tcl_UntraceVar(turndialPtr->interp, turndialPtr->varName,
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		TurndialVarProc, (ClientData) turndialPtr);
    }
    if (turndialPtr->tickGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->tickGC);
    }
    if (turndialPtr->markGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->markGC);
    }
    if (turndialPtr->copyGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->copyGC);
    }
    if (turndialPtr->textGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->textGC);
    }
    Tk_FreeOptions(configSpecs, (char *) turndialPtr, turndialPtr->display, 0);
    ckfree((char *) turndialPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConfigureTurndial --
 *
 *	This procedure is called to process an argv/argc list, plus
 *	the Tk option database, in order to configure (or
 *	reconfigure) a turndial widget.
 *
 * Results:
 *	The return value is a standard Tcl result.  If TCL_ERROR is
 *	returned, then interp->result contains an error message.
 *
 * Side effects:
 *	Configuration information, such as colors, border width,
 *	etc. get set for turndialPtr;  old resources get freed,
 *	if there were any.
 *
 *----------------------------------------------------------------------
 */

static int
ConfigureTurndial(interp, turndialPtr, argc, argv, flags)
    Tcl_Interp *interp;			/* Used for error reporting. */
    register Turndial *turndialPtr;	/* Information about widget; may or may
					 * not already have values for some 
					 * fields. */
    int argc;				/* Number of valid entries in argv. */
    char **argv;			/* Arguments. */
    int flags;				/* Flags to pass to Tk_ConfigureWidget. */
{
    XGCValues gcValues;
    GC newGC;

    /*
     * Eliminate any existing trace on a variable monitored by the turndial.
     */

    if (turndialPtr->varName != NULL) {
	Tcl_UntraceVar(interp, turndialPtr->varName, 
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		TurndialVarProc, (ClientData) turndialPtr);
    }

    if (Tk_ConfigureWidget(interp, turndialPtr->tkwin, configSpecs,
	    argc, argv, (char *) turndialPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * If the turndial is tied to the value of a variable, then set up
     * a trace on the variable's value and set the turndial's value from
     * the value of the variable, if it exists.
     */

    if (turndialPtr->varName != NULL) {
	char *stringValue, *end;
	double value;

	stringValue = Tcl_GetVar(interp, turndialPtr->varName, TCL_GLOBAL_ONLY);
	if (stringValue != NULL) {
	    value = strtod(stringValue, &end);
	    if ((end != stringValue) && (*end == 0)) {
		turndialPtr->value = value;
	    }
	}
	Tcl_TraceVar(interp, turndialPtr->varName,
		TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		TurndialVarProc, (ClientData) turndialPtr);
    }

    /*
     * Process some of the turndial's parameters to make sure they are
     * reasonable.
     */

    turndialPtr->tickInterval = fabs(turndialPtr->tickInterval);
    turndialPtr->minorTickInterval = fabs(turndialPtr->minorTickInterval);

    /*
     * Set the turndial value to itself;  all this does is to make sure
     * that the turndial's value is within the new acceptable range for
     * the turndial and reflect the value in the associated variable,
     * if any.
     */

    ComputeFormat(turndialPtr);
    SetTurndialValue(turndialPtr, turndialPtr->value, 1, 1);

    if (turndialPtr->label != NULL) {
	turndialPtr->labelLength = strlen(turndialPtr->label);
    } else {
	turndialPtr->labelLength = 0;
    }

    if ((turndialPtr->state != tkNormalUid)
	    && (turndialPtr->state != tkDisabledUid)
	    && (turndialPtr->state != tkActiveUid)) {
	Tcl_AppendResult(interp, "bad state value \"", turndialPtr->state,
		"\":  must be normal, active, or disabled", (char *) NULL);
	turndialPtr->state = tkNormalUid;
	return TCL_ERROR;
    }

    Tk_SetBackgroundFromBorder(turndialPtr->tkwin, turndialPtr->bgBorder);

    /*
     * Create GC's.
     */

    gcValues.foreground = turndialPtr->tickColorPtr->pixel;
    newGC = Tk_GetGC(turndialPtr->tkwin, GCForeground, &gcValues);
    if (turndialPtr->tickGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->tickGC);
    }
    turndialPtr->tickGC = newGC;

    gcValues.foreground = turndialPtr->markColorPtr->pixel;
    gcValues.line_width = (int)rint(MARK_WIDTH*turndialPtr->radius);
    newGC = Tk_GetGC(turndialPtr->tkwin, GCForeground|GCLineWidth, &gcValues);
    if (turndialPtr->markGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->markGC);
    }
    turndialPtr->markGC = newGC;

    if (turndialPtr->copyGC == None) {
	gcValues.graphics_exposures = False;
	turndialPtr->copyGC = Tk_GetGC(turndialPtr->tkwin, GCGraphicsExposures,
	    &gcValues);
    }

    if (turndialPtr->highlightWidth < 0) {
	turndialPtr->highlightWidth = 0;
    }
    gcValues.font = turndialPtr->fontPtr->fid;
    gcValues.foreground = turndialPtr->textColorPtr->pixel;
    newGC = Tk_GetGC(turndialPtr->tkwin, GCForeground|GCFont, &gcValues);
    if (turndialPtr->textGC != None) {
	Tk_FreeGC(turndialPtr->display, turndialPtr->textGC);
    }
    turndialPtr->textGC = newGC;

    turndialPtr->inset = turndialPtr->highlightWidth + turndialPtr->borderWidth;

    /*
     * Recompute display-related information, and let the geometry
     * manager know how much space is needed now.
     */

    ComputeTurndialGeometry(turndialPtr);

    EventuallyRedrawTurndial(turndialPtr, REDRAW_ALL);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeFormat --
 *
 *	This procedure is invoked to recompute the "format" fields
 *	of a turndial's widget record, which determines how the value
 *	of the turndial is converted to a string.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The format fields of turndialPtr are modified.
 *
 *----------------------------------------------------------------------
 */

static void
ComputeFormat(turndialPtr)
    Turndial *turndialPtr;			/* Information about turndial widget. */
{
    double maxValue, x;
    int mostSigDigit, numDigits, leastSigDigit, afterDecimal;
    int eDigits, fDigits, tagLeastSigDigit;

    /*
     * Compute the displacement from the decimal of the most significant
     * digit required for any number in the turndial's range.
     */

    maxValue = fabs(turndialPtr->fromValue);
    x = fabs(turndialPtr->toValue);
    if (x > maxValue) {
	maxValue = x;
    }
    if (maxValue == 0) {
	maxValue = 1;
    }
    mostSigDigit = floor(log10(maxValue));

    /*
     * If the number of significant digits wasn't specified explicitly,
     * compute it. It's the difference between the most significant
     * digit needed to represent any number on the turndial and the
     * most significant digit of the smallest difference between
     * numbers on the turndial.  In other words, display enough digits so
     * that at least one digit will be different between any two adjacent
     * positions of the turndial.
     */

    numDigits = turndialPtr->digits;
    if (numDigits <= 0) {
	if  (turndialPtr->resolution > 0) {
	    /*
	     * A resolution was specified for the turndial, so just use it.
	     */

	    leastSigDigit = floor(log10(turndialPtr->resolution));
	} else {
	    /*
	     * No resolution was specified, so compute the difference
	     * in value between adjacent pixels at the knob circumference
	     * and use it for the least
	     * significant digit.
	     */

	    x = fabs(turndialPtr->fromValue - turndialPtr->toValue);
	    if (turndialPtr->radius > 0) {
		x /= 6.28*turndialPtr->radius;
	    }
	    if (x > 0){
		leastSigDigit = floor(log10(x));
	    } else {
		leastSigDigit = 0;
	    }
	}
	numDigits = mostSigDigit - leastSigDigit + 1;
	if (numDigits < 1) {
	    numDigits = 1;
	}
    }

    /*
     * Compute the number of characters required using "e" format and
     * "f" format, and then choose whichever one takes fewer characters.
     */

    eDigits = numDigits + 4;
    if (numDigits > 1) {
	eDigits++;			/* Decimal point. */
    }
    afterDecimal = numDigits - mostSigDigit - 1;
    if (afterDecimal < 0) {
	afterDecimal = 0;
    }
    fDigits = (mostSigDigit >= 0) ? mostSigDigit + afterDecimal : afterDecimal;
    if (afterDecimal > 0) {
	fDigits++;			/* Decimal point. */
    }
    if (mostSigDigit < 0) {
	fDigits++;			/* Zero to left of decimal point. */
    }
    if (fDigits <= eDigits) {
	sprintf(turndialPtr->valueFormat, "%%.%df", afterDecimal);
    } else {
	sprintf(turndialPtr->valueFormat, "%%.%de", numDigits-1);
    }

    /*
     * Determine the format of the tag field. Each digit after the decimal
     * point should be significant.
     */
    strcpy(turndialPtr->tagFormat,turndialPtr->valueFormat);
    if (turndialPtr->tickInterval != 0) {
	tagLeastSigDigit = floor(log10(turndialPtr->tickInterval));
	if (tagLeastSigDigit < 0)
	    sprintf(turndialPtr->tagFormat,"%%.%df",-tagLeastSigDigit);
	else
	    sprintf(turndialPtr->tagFormat,"%%.0f");
	
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeTurndialGeometry --
 *
 *	This procedure is called to compute various geometrical
 *	information for a turndial, such as where various things get
 *	displayed.  It's called when the window is reconfigured.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Display-related numbers get changed in *turndialPtr.  The
 *	geometry manager gets told about the window's preferred size.
 *
 *----------------------------------------------------------------------
 */

static void
ComputeTurndialGeometry(turndialPtr)
    register Turndial *turndialPtr;		/* Information about widget. */
{
    XCharStruct bbox;
    char valueString[PRINT_CHARS];
    int dummy, y, xr, yr, i, width;
    double angle;

    int lineHeight = turndialPtr->fontPtr->ascent +
				turndialPtr->fontPtr->descent;

    /*
     * Calculate knob polygon points.
     */
 
    xr = yr = turndialPtr->radius;
    for ( i = 0; i < KNOB_POINTS; ++i) {
	angle = 2*M_PI * i / (KNOB_POINTS-1);
	turndialPtr->knob[i].x = turndialPtr->centerX + 
			(int)rint(xr * sin(angle) );
	turndialPtr->knob[i].y = turndialPtr->centerY +
			(int)rint(yr * cos(angle) );
    }

    /*
     * Radii of the minortickmarks.
     */

    if (turndialPtr->minorTickInterval != 0.0) {
	turndialPtr->minorTickInnerR = turndialPtr->radius;
	turndialPtr->minorTickOuterR = turndialPtr->minorTickInnerR +
		(int)rint(MINORTICK_LENGTH * turndialPtr->radius);
	xr = yr = turndialPtr->minorTickOuterR;
    }

    /*
     * Location of major ticks, both the ticks and the label.
     */

    if (turndialPtr->tickInterval != 0.0) {
	int width;

	/*
	 * Make room for major minorticks.
	 */

	turndialPtr->tickInnerR = turndialPtr->radius;
	turndialPtr->tickOuterR = turndialPtr->tickInnerR + 
		(int)rint(TICK_LENGTH * turndialPtr->radius);
	xr = yr = turndialPtr->tickOuterR;

	if (turndialPtr->showTags) {
	    /*
	     * Tags should lean against this radius.
	     */
	    xr += SPACING;
	    yr += SPACING;
	    turndialPtr->tagR = xr;

	    /*
	     * Compute the width needed to display
	     * the turndial tags by formatting strings for the
	     * lower and upper limit.
	     */

	    sprintf(valueString, turndialPtr->tagFormat,
		turndialPtr->fromValue);
	    XTextExtents(turndialPtr->fontPtr, valueString,
		(int) strlen(valueString), &dummy, &dummy, &dummy, &bbox);
	    width = bbox.rbearing - bbox.lbearing;
	    sprintf(valueString, turndialPtr->tagFormat, turndialPtr->toValue);
	    XTextExtents(turndialPtr->fontPtr, valueString,
		(int) strlen(valueString), &dummy, &dummy, &dummy, &bbox);
	    if (bbox.rbearing - bbox.lbearing > width)
		width = bbox.rbearing-bbox.lbearing;

	    xr += width;
	    yr += lineHeight;
        }
    }
    xr += SPACING;
    yr += SPACING;

    /*
     * X geometry. Take account of the knob and tickmarks.
     */

    turndialPtr->centerX = turndialPtr->inset + xr;
    width = 2*(turndialPtr->inset + xr);

    /*
     * Also make sure the value fits.
     */

    if (turndialPtr->showValue) {
	int valueWidth, valueCenterX;

	/*
	 * Determine the max width of the value from the upper and lower limit,
	 * using the value's format string this time.
	 */

  	sprintf(valueString, turndialPtr->valueFormat, turndialPtr->fromValue);
	XTextExtents(turndialPtr->fontPtr, valueString,
	    (int) strlen(valueString), &dummy, &dummy, &dummy, &bbox);
	valueWidth = bbox.rbearing - bbox.lbearing;
	sprintf(valueString, turndialPtr->valueFormat, turndialPtr->toValue);
	XTextExtents(turndialPtr->fontPtr, valueString,
	    (int) strlen(valueString), &dummy, &dummy, &dummy, &bbox);
	if (bbox.rbearing - bbox.lbearing > valueWidth)
	    valueWidth = bbox.rbearing-bbox.lbearing;

	/*
	 * Correct the center and width so that the value will fit.
	 */

	valueCenterX = turndialPtr->inset + SPACING + valueWidth/2;
 	if (valueCenterX > turndialPtr->centerX) {
	    turndialPtr->centerX = valueCenterX;
	    width = 2*(turndialPtr->inset + SPACING) + valueWidth;
	}
    }

    /*
     * Determine the Y geometry, working from top to bottom.
     */

    y = turndialPtr->inset;
    if (turndialPtr->labelLength != 0) {
	y += SPACING;
	turndialPtr->labelY = y;
	y += lineHeight + SPACING;
    }
    y += yr;
    turndialPtr->centerY = y;
    y += yr;
    if (turndialPtr->showValue) {
	    y += SPACING;
	    turndialPtr->valueY = y;
	    y += lineHeight + SPACING;
    }

    Tk_GeometryRequest(turndialPtr->tkwin, width, y + turndialPtr->inset);
    Tk_SetInternalBorder(turndialPtr->tkwin, turndialPtr->inset);
}

/*
 *--------------------------------------------------------------
 *
 * DisplayTicks --
 *
 *	This procedure draws the tickmarks corresponding to the
 *	specified interval. If the flag major is false, there is
 *	minor tickmarks are drawn; otherwise major tickmarks are
 *	drawn.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Either the minor or the major ticks are drawn in the
 *	drawable.
 *
 *--------------------------------------------------------------
 */

static void
DisplayTicks(turndialPtr, drawable, minorTicks)
    Turndial *turndialPtr;		/* Widget record for turndial. */
    Drawable drawable;			/* Where to display turndial (window
					 * or pixmap). */
    int minorTicks;				/* 0: minor, 1: major ticks */
{
    double from, to, interval, tickValue;
    int i, sign;

    if (minorTicks)
	interval = turndialPtr->minorTickInterval;
    else 
	interval = turndialPtr->tickInterval;

    if (interval == 0.0)
	return;

    from = turndialPtr->fromValue;
    to = turndialPtr->toValue;
    sign = (to > from ? 1 : -1);

    if ((to-from != 0.0) && (interval/(to-from) > MAX_TICKS))
	return;

    /*
     * Round the lower tick towards the end value
     */

    if (from < to)
	i = ceil(from/interval);
    else
	i = floor(from/interval);

    for (tickValue = i*interval;
	    sign*tickValue <= sign*to; i += sign, tickValue = i*interval) {
	/*
	 * The RoundToResolution call gets rid of accumulated
	 * round-off errors, if any.
	 */

	tickValue = RoundToResolution(turndialPtr, tickValue);

	if (minorTicks) {
	    /*
	     * Prevent drawing at the position of a major tick.
	     */
	    if (fmod(tickValue,turndialPtr->tickInterval) != 0.0) {
		DisplayTick(turndialPtr, drawable, turndialPtr->tickGC,
		    tickValue, turndialPtr->minorTickInnerR,
		    turndialPtr->minorTickOuterR, 0);
	    }
	}
	else {
	    DisplayTick(turndialPtr, drawable, turndialPtr->tickGC, tickValue,
		turndialPtr->tickInnerR, turndialPtr->tickOuterR,
		turndialPtr->tagR);
	}
    }
}

/*
 *--------------------------------------------------------------
 *
 * DisplayTick --
 *
 *	This procedure draws a single minortickmark corresponding to the
 *	value specified.
 *
 * Results:
 *	There is no return value.
 *
 * Side effects:
 *	A single minortick appears on the screen.
 *
 *--------------------------------------------------------------
 */

static void
DisplayTick(turndialPtr, drawable, gc, value, innerR, outerR, tagR)
    Turndial *turndialPtr;		/* Widget record for turndial. */
    Drawable drawable;			/* Where to display turndial (window
					 * or pixmap). */
    GC gc;				/* GC to use for drawing */
    double value;			/* Value the minortick should represent. */
    int innerR;				/* Radial coord of begin of minortick. */
    int outerR;				/* Radial coord of end of minortick. */
    int tagR;				/* Radial coord of label; 0=none. */
{
    double angle, c, s;
    XCharStruct bbox;
    char valueString[PRINT_CHARS];
    int length, dummy, x, y, centerX, centerY, x1, y1, x2, y2;

    /*
     * Determine endpoints of the minortick.
     */

    angle = ValueToAngle(turndialPtr, value);
    s = sin(angle);
    c = cos(angle);
    centerX = turndialPtr->centerX;
    centerY = turndialPtr->centerY;
    x1 = centerX + (int)rint(innerR*s);
    y1 = centerY - (int)rint(innerR*c);
    x2 = centerX + (int)rint(outerR*s);
    y2 = centerY - (int)rint(outerR*c);

    /*
     * Draw the tick.
     */

    XDrawLine(turndialPtr->display, drawable, gc, x1, y1, x2, y2);

    /*
     * Display a label next to the minortick. The location of the label is
     * corrected assuming that the label has the form of an ellips,
     * which is of course only approximately true.
     */

    if (tagR > 0 && (turndialPtr->showTags != 0)) {
	int width, height;
	sprintf(valueString, turndialPtr->tagFormat, value);
	length = strlen(valueString);
	XTextExtents(turndialPtr->fontPtr, valueString, length,
	    &dummy, &dummy, &dummy, &bbox);
	width = bbox.rbearing - bbox.lbearing;
	height = bbox.ascent + bbox.descent;
	x = turndialPtr->centerX + (int)rint(tagR*s);
	x += 0.5*width*(s-1);
	y = turndialPtr->centerY - (int)rint(tagR*c);
	y -= 0.5*height*(c-1);
	XDrawString(turndialPtr->display, drawable, turndialPtr->textGC,
	    x, y, valueString, length);
	
    }
}

/*
 *----------------------------------------------------------------------
 *
 * DisplayTurndial --
 *
 *	This procedure is invoked as an idle handler to redisplay
 *	the contents of a turndial widget.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The turndial gets redisplayed.
 *
 *----------------------------------------------------------------------
 */

static void
DisplayTurndial(clientData)
    ClientData clientData;	/* Widget record for turndial. */
{
    Turndial *turndialPtr = (Turndial *) clientData;
    Tk_Window tkwin = turndialPtr->tkwin;
    Pixmap pixmap;
    XRectangle drawnArea;
    Tk_3DBorder knobBorder;
    double angle, c, s, markInnerR, markOuterR;

    if ((turndialPtr->tkwin == NULL) || !Tk_IsMapped(turndialPtr->tkwin)) {
	goto done;
    }

    /*
     * Invoke the turndial's command if needed.
     */

    Tk_Preserve((ClientData) turndialPtr);
    if ((turndialPtr->flags & INVOKE_COMMAND) &&
				(turndialPtr->command != NULL)) {
	char commandString[PRINT_CHARS];
	int result;

	sprintf(commandString, turndialPtr->valueFormat, turndialPtr->value);
	result = Tcl_VarEval(turndialPtr->interp, turndialPtr->command,
		" ", commandString, (char *) NULL);
	if (result != TCL_OK) {
	    Tcl_AddErrorInfo(turndialPtr->interp,
		    "\n    (command executed by turndial)");
	    Tk_BackgroundError(turndialPtr->interp);
	}
    }
    turndialPtr->flags &= ~INVOKE_COMMAND;
    if (turndialPtr->tkwin == NULL) {
	Tk_Release((ClientData) turndialPtr);
	return;
    }
    Tk_Release((ClientData) turndialPtr);

#ifndef TK36
    /*
     * In order to avoid screen flashes, this procedure redraws
     * the turndial in a pixmap, then copies the pixmap to the
     * screen in a single operation.  This means that there's no
     * point in time where the on-sreen image has been cleared.
     */
    pixmap = Tk_GetPixmap(turndialPtr->display, Tk_WindowId(tkwin),
	    Tk_Width(tkwin), Tk_Height(tkwin), Tk_Depth(tkwin));
#else
    pixmap = Tk_WindowId(tkwin);
#endif

    drawnArea.x = 0;
    drawnArea.y = 0;
    drawnArea.width = Tk_Width(tkwin);
    drawnArea.height = Tk_Height(tkwin);

    /*
     * Reduce redraw area to minimal size. In case of a value change, only the
     * knob and the value need to be redrawn. 
     */

    if (!(turndialPtr->flags & REDRAW_OTHER)) {

	/*
	 * At least include the knob.
	 */

	drawnArea.x = turndialPtr->centerX - turndialPtr->radius;
	drawnArea.y = turndialPtr->centerY - turndialPtr->radius;
	drawnArea.width = 2*turndialPtr->radius;
	drawnArea.height = 2*turndialPtr->radius;

	/*
	 * Correct to include value as well. We can hardly know how wide the
	 * string is; the upper and lower limit is only a guess for a
	 * non-fixed-width font. Therefore redraw the entire width.
	 */

	if (turndialPtr->showValue) {
	    drawnArea.x = turndialPtr->inset;
	    drawnArea.width = Tk_Width(tkwin) - 2*turndialPtr->inset;
	    drawnArea.height = turndialPtr->valueY
		+ turndialPtr->fontPtr->ascent + turndialPtr->fontPtr->descent
		- drawnArea.y; 
	}
    }

    /*
     * Fill drawnArea with the background.
     */
#ifndef TK36
    Tk_Fill3DRectangle(tkwin, pixmap, turndialPtr->bgBorder,
	    drawnArea.x, drawnArea.y, drawnArea.width,
	    drawnArea.height, 0, TK_RELIEF_FLAT);
#else
    Tk_Fill3DRectangle(Tk_Display(tkwin), pixmap, turndialPtr->bgBorder,
	    drawnArea.x, drawnArea.y, drawnArea.width,
	    drawnArea.height, 0, TK_RELIEF_FLAT);
#endif

    /*
     * Display minor and major ticks.
     */

    DisplayTicks(turndialPtr, pixmap, 1); /* minor */
    DisplayTicks(turndialPtr, pixmap, 0); /* major */

    /*
     * Display the knob.
     */

    if (turndialPtr->state == tkActiveUid) {
	knobBorder = turndialPtr->activeBorder;
    } else {
	knobBorder = turndialPtr->knobBorder;
    }
#ifndef TK36
    Tk_Fill3DPolygon(tkwin, pixmap,
	    knobBorder, turndialPtr->knob,
	    KNOB_POINTS, turndialPtr->knobBorderWidth, TK_RELIEF_RAISED);
#else
    Tk_Fill3DPolygon(Tk_Display(tkwin), pixmap,
	    knobBorder, turndialPtr->knob,
	    KNOB_POINTS, turndialPtr->knobBorderWidth, TK_RELIEF_RAISED);
#endif

   /*
    * Draw the mark.
    */

    angle = ValueToAngle(turndialPtr,turndialPtr->value);
    c = cos(angle);
    s = sin(angle);
    markInnerR = turndialPtr->radius - MARK_LENGTH*turndialPtr->radius;
    markOuterR = turndialPtr->radius - turndialPtr->knobBorderWidth;
    XDrawLine(turndialPtr->display, pixmap, turndialPtr->markGC,
	(int)rint(turndialPtr->centerX + s*markInnerR),
	(int)rint(turndialPtr->centerY - c*markInnerR),
	(int)rint(turndialPtr->centerX + s*markOuterR),
	(int)rint(turndialPtr->centerY - c*markOuterR) );

    /*
     * Display the value centered, if required.
     */

    if (turndialPtr->showValue) {
	char valueString[PRINT_CHARS];
	XCharStruct bbox;
	int dummy, length;

	sprintf(valueString, turndialPtr->valueFormat, turndialPtr->value);
	length = strlen(valueString);
	XTextExtents(turndialPtr->fontPtr, valueString, length,
	    &dummy, &dummy, &dummy, &bbox);
	XDrawString(turndialPtr->display, pixmap, turndialPtr->textGC,
	    turndialPtr->centerX - (bbox.rbearing - bbox.lbearing)/2,
	    turndialPtr->valueY + turndialPtr->fontPtr->ascent,
	    valueString, length);
    }

    /*
     * Draw the label north of the turndial.
     */

    if ((turndialPtr->flags && REDRAW_OTHER) &&
		(turndialPtr->labelLength != 0)) {
	XCharStruct bbox;
	int dummy;
	
	XTextExtents(turndialPtr->fontPtr, turndialPtr->label,
	    turndialPtr->labelLength, &dummy, &dummy, &dummy, &bbox); 
	XDrawString(turndialPtr->display, pixmap, turndialPtr->textGC,
	    turndialPtr->centerX - (bbox.rbearing-bbox.lbearing)/2,
	    turndialPtr->labelY + turndialPtr->fontPtr->ascent,
	    turndialPtr->label, turndialPtr->labelLength);
    }
  
    /*
     * Handle border and traversal highlight.
     */

    if (turndialPtr->flags & REDRAW_OTHER) {
	if (turndialPtr->relief != TK_RELIEF_FLAT) {
#ifndef TK36
	    Tk_Draw3DRectangle(tkwin, pixmap, turndialPtr->bgBorder,
#else
	    Tk_Draw3DRectangle(Tk_Display(tkwin), pixmap, turndialPtr->bgBorder,
#endif
		    turndialPtr->highlightWidth, turndialPtr->highlightWidth,
		    Tk_Width(tkwin) - 2*turndialPtr->highlightWidth,
		    Tk_Height(tkwin) - 2*turndialPtr->highlightWidth,
		    turndialPtr->borderWidth, turndialPtr->relief);

	}
#ifndef TK36
	if (turndialPtr->highlightWidth != 0) {
	    GC gc;

	    if (turndialPtr->flags & GOT_FOCUS) {
		gc = Tk_GCForColor(turndialPtr->highlightColorPtr, pixmap);
	    } else {
		gc = Tk_GCForColor(turndialPtr->highlightBgColorPtr, pixmap);
	    }
	    Tk_DrawFocusHighlight(tkwin, gc, turndialPtr->highlightWidth,
		pixmap);
	}
#endif
    }

    /*
     * Copy the information from the off-screen pixmap onto the screen,
     * then delete the pixmap.
     */

#ifndef TK36
    XCopyArea(turndialPtr->display, pixmap, Tk_WindowId(tkwin),
	    turndialPtr->copyGC, drawnArea.x, drawnArea.y, drawnArea.width,
	    drawnArea.height, drawnArea.x, drawnArea.y);
    Tk_FreePixmap(turndialPtr->display, pixmap);
#endif

    done:
    turndialPtr->flags &= ~REDRAW_ALL;
}

/*
 *----------------------------------------------------------------------
 *
 * TurndialElement --
 *
 *	Determine which part of a turndial widget lies under a given
 *	point.
 *
 * Results:
 *	The return value is either TROUGH1, MARK, TROUGH2, or
 *	OTHER, depending on which of the turndial's active elements
 *	(if any) is under the point at (x,y).
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
TurndialElement(turndialPtr, x, y)
    Turndial *turndialPtr;		/* Widget record for turndial. */
    int x, y;			/* Coordinates within turndialPtr's window. */
{
    int dx, dy;

    dx = x - turndialPtr->centerX;
    dy = y - turndialPtr->centerY;
    if (dx*dx+dy*dy <= turndialPtr->radius*turndialPtr->radius)
	return KNOB;
    else if (dx < 0)
	return LEFT;
    else
	return RIGHT;
}

/*
 *----------------------------------------------------------------------
 *
 * PixelToValue --
 *
 *	Given a pixel within a turndial window, return the turndial
 *	reading corresponding to that pixel.
 *
 * Results:
 *	A double-precision turndial reading.  If the value is outside
 *	the legal range for the turndial then it's rounded to the nearest
 *	end of the turndial.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static double
PixelToValue(turndialPtr, x, y)
    register Turndial *turndialPtr;		/* Information about widget. */
    int x, y;				/* Coordinates of point within
					 * window. */
{
    double value, beginRad, endRad, angleRange, valueRange, fraction, angle;
    double center;
    int dx, dy;

    beginRad = deg_to_rad(turndialPtr->beginAngle);
    endRad = deg_to_rad(turndialPtr->endAngle);
    angleRange = endRad-beginRad;
    valueRange = turndialPtr->toValue - turndialPtr->fromValue;

    if (valueRange == 0.0 || angleRange == 0.0)
	value = turndialPtr->fromValue;
    else {
	dx = x - turndialPtr->centerX;
	dy = y - turndialPtr->centerY;
	if (dx == 0 && dy == 0)
	    angle = 0;
	else
	    /*
	     * Angle will be zero at 12 o'clock and positive clockwise.
	     */
	    angle = atan2((double)dx,(double)-dy);
	/*
	 * Make angle flip sign just opposite the center.
	 */
	center = (beginRad+endRad)/2;
	angle = fmod(angle-center+5*M_PI,2*M_PI)-M_PI;
	/*
	 * Angle is zero at the center. Convert to a fraction.
	 */
        fraction = 0.5+angle/angleRange;
	if (fraction < 0.0)
	    fraction = 0.0;
	else if (fraction > 1.0)
	    fraction = 1.0;

	value = turndialPtr->fromValue + fraction * valueRange;
    }

    return RoundToResolution(turndialPtr, value);
}

/*
 *----------------------------------------------------------------------
 *
 * ValueToAngle --
 *
 *	Given a reading of the turndial, return the corresponding
 *	angle of the knob.
 *
 * Results:
 *	The angle in radians of the knob that corresponds to the reading
 *	that was specified in the call. Zero is 12 o'clock, positive is clockwise.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static double
ValueToAngle(turndialPtr, value)
    register Turndial *turndialPtr;		/* Information about widget. */
    double value;				/* Reading of the widget. */
{
    double angleRange, valueRange, fraction;

    valueRange = turndialPtr->toValue - turndialPtr->fromValue;
    if (valueRange == 0.0)
	fraction = 0.0;
    else {
	fraction = (value-turndialPtr->fromValue)/valueRange;
	if (fraction < 0.0)
	    fraction = 0.0;
	else if (fraction > 1.0)
	    fraction = 1.0;
    }

    angleRange = deg_to_rad(turndialPtr->endAngle - turndialPtr->beginAngle);
    return deg_to_rad(turndialPtr->beginAngle) + fraction * angleRange;
}

/*
 *----------------------------------------------------------------------
 *
 * ValueToPixel --
 *
 *	Given a reading of the turndial, return the x and
 *	y-coordinate corresponding to that reading.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The integers whose pointers are passed are set to a value giving
 *	the pixel location corresponding
 *	to reading.  The value is restricted to lie within the
 *	defined range for the turndial.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
ValueToPixel(turndialPtr, value, xPtr, yPtr)
    register Turndial *turndialPtr;		/* Information about widget. */
    double value;				/* Reading of the widget. */
    int *xPtr, *yPtr;				/* Integers to store the result. */
{
    double angle;

    angle = ValueToAngle(turndialPtr, value);
    *xPtr = turndialPtr->centerX + turndialPtr->radius * sin(angle);
    *yPtr = turndialPtr->centerY + turndialPtr->radius * cos(angle);
}

/*
 *--------------------------------------------------------------
 *
 * TurndialEventProc --
 *
 *	This procedure is invoked by the Tk dispatcher for various
 *	events on turndials.
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
TurndialEventProc(clientData, eventPtr)
    ClientData clientData;	/* Information about window. */
    XEvent *eventPtr;		/* Information about event. */
{
    Turndial *turndialPtr = (Turndial *) clientData;
    if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0)) {
	EventuallyRedrawTurndial(turndialPtr, REDRAW_ALL);
    } else if (eventPtr->type == DestroyNotify) {
	if (turndialPtr->tkwin != NULL) {
#ifndef TK36
	    turndialPtr->tkwin = NULL;
	    Tcl_DeleteCommand(turndialPtr->interp,
		    Tcl_GetCommandName(turndialPtr->interp,
			turndialPtr->widgetCmd));

#else
	    Tcl_DeleteCommand(turndialPtr->interp,
			 Tk_PathName(turndialPtr->tkwin));
	    turndialPtr->tkwin = NULL;
#endif
	}
	if (turndialPtr->flags & REDRAW_ALL) {
	    Tk_CancelIdleCall(DisplayTurndial, (ClientData) turndialPtr);
	}
	Tk_EventuallyFree((ClientData) turndialPtr, DestroyTurndial);
    } else if (eventPtr->type == ConfigureNotify) {
	ComputeTurndialGeometry(turndialPtr);
 	EventuallyRedrawTurndial(turndialPtr, REDRAW_ALL);
   } else if (eventPtr->type == FocusIn) {
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    turndialPtr->flags |= GOT_FOCUS;
	    if (turndialPtr->highlightWidth > 0) {
		EventuallyRedrawTurndial(turndialPtr, REDRAW_ALL);
	    }
	}
    } else if (eventPtr->type == FocusOut) {
	if (eventPtr->xfocus.detail != NotifyInferior) {
	    turndialPtr->flags &= ~GOT_FOCUS;
	    if (turndialPtr->highlightWidth > 0) {
		EventuallyRedrawTurndial(turndialPtr, REDRAW_ALL);
	    }
	}
    }
/*@*/
/*#ifdef TK36*/

    /*
     * Handle mouse events
     */
    else if (turndialPtr->state != tkDisabledUid) {
	Tk_Preserve((ClientData) turndialPtr);
        if (eventPtr->type == EnterNotify) {
	    turndialPtr->state = tkActiveUid;
	    EventuallyRedrawTurndial(turndialPtr, REDRAW_KNOB);
        } else if ((eventPtr->type == LeaveNotify) && 
		!(turndialPtr->flags & BUTTON_PRESSED)) {
	    turndialPtr->state = tkNormalUid;
	    EventuallyRedrawTurndial(turndialPtr, REDRAW_KNOB);
	} else if ((eventPtr->type == MotionNotify) &&
			(turndialPtr->flags & BUTTON_PRESSED)) {
	    SetTurndialValue(turndialPtr,  PixelToValue(turndialPtr,
		eventPtr->xmotion.x, eventPtr->xmotion.y),1,1);
	} else if ((eventPtr->type == ButtonPress)
		&& (eventPtr->xbutton.button == Button1)
		&& (eventPtr->xbutton.state == 0)) {
            turndialPtr->flags |= BUTTON_PRESSED;
	    SetTurndialValue(turndialPtr, PixelToValue(turndialPtr,
		eventPtr->xbutton.x, eventPtr->xbutton.y),1,1);
	} else if ((eventPtr->type == ButtonRelease) 
			&& (eventPtr->xbutton.button == Button1)
			&& (turndialPtr->flags & BUTTON_PRESSED) ) {
            turndialPtr->flags &= ~BUTTON_PRESSED;
	    SetTurndialValue(turndialPtr, PixelToValue(turndialPtr,
		eventPtr->xbutton.x, eventPtr->xbutton.y),1,1);
        }
        Tk_Release((ClientData) turndialPtr);
    }
/*#endif*/
}

/*
 *----------------------------------------------------------------------
 *
 * TurndialCmdDeletedProc --
 *
 *	This procedure is invoked when a widget command is deleted.  If
 *	the widget isn't already in the process of being destroyed,
 *	this command destroys it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The widget is destroyed.
 *
 *----------------------------------------------------------------------
 */

static void
TurndialCmdDeletedProc(clientData)
    ClientData clientData;	/* Pointer to widget record for widget. */
{
    Turndial *turndialPtr = (Turndial *) clientData;
    Tk_Window tkwin = turndialPtr->tkwin;

    /*
     * This procedure could be invoked either because the window was
     * destroyed and the command was then deleted (in which case tkwin
     * is NULL) or because the command was deleted, and then this procedure
     * destroys the widget.
     */

    if (tkwin != NULL) {
	turndialPtr->tkwin = NULL;
	Tk_DestroyWindow(tkwin);
    }
}

/*
 *--------------------------------------------------------------
 *
 * SetTurndialValue --
 *
 *	This procedure changes the value of a turndial and invokes
 *	a Tcl command to reflect the current position of a turndial
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A Tcl command is invoked, and an additional error-processing
 *	command may also be invoked.  The turndial's slider is redrawn.
 *
 *--------------------------------------------------------------
 */

static void
SetTurndialValue(turndialPtr, value, setVar, invokeCommand)
    register Turndial *turndialPtr;	/* Info about widget. */
    double value;		/* New value for turndial.  Gets adjusted
				 * if it's off the turndial. */
    int setVar;			/* Non-zero means reflect new value through
				 * to associated variable, if any. */
    int invokeCommand;		/* Non-zero means invoked -command option
				 * to notify of new value, 0 means don't. */
{
    char string[PRINT_CHARS];

    value = RoundToResolution(turndialPtr, value);
    /*printf("rounded: %f\n",value);*/
    if ((turndialPtr->fromValue < turndialPtr->toValue)) {
        if (value < turndialPtr->fromValue)
	    value = turndialPtr->fromValue;
        if (value > turndialPtr->toValue)
	    value = turndialPtr->toValue;
    }
    else {
	if (value > turndialPtr->fromValue)
	    value = turndialPtr->fromValue;
	if (value < turndialPtr->toValue)
	    value = turndialPtr->toValue;
    }
    /*printf("limited: %f\n",value);*/
    if (turndialPtr->flags & NEVER_SET) {
	turndialPtr->flags &= ~NEVER_SET;
    } else if (turndialPtr->value == value) {
	return;
    }
    turndialPtr->value = value;
    if (invokeCommand) {
	turndialPtr->flags |= INVOKE_COMMAND;
    }
    EventuallyRedrawTurndial(turndialPtr, REDRAW_KNOB);

    if (setVar && (turndialPtr->varName != NULL)) {
	sprintf(string, turndialPtr->valueFormat, turndialPtr->value);
	turndialPtr->flags |= SETTING_VAR;
	Tcl_SetVar(turndialPtr->interp, turndialPtr->varName, string,
	       TCL_GLOBAL_ONLY);
	turndialPtr->flags &= ~SETTING_VAR;
    }
}

/*
 *--------------------------------------------------------------
 *
 * EventuallyRedrawTurndial --
 *
 *	Arrange for part or all of a turndial widget to redrawn at
 *	the next convenient time in the future.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If "what" is REDRAW_KNOB then just the knob and the
 *	value readout will be redrawn;  if "what" is REDRAW_ALL
 *	then the entire widget will be redrawn.
 *
 *--------------------------------------------------------------
 */

static void
EventuallyRedrawTurndial(turndialPtr, what)
    register Turndial *turndialPtr;	/* Information about widget. */
    int what;			/* What to redraw:  REDRAW_KNOB
				 * or REDRAW_ALL. */
{
    if ((what == 0) || (turndialPtr->tkwin == NULL)
	    || !Tk_IsMapped(turndialPtr->tkwin)) {
	return;
    }
    if ((turndialPtr->flags & REDRAW_ALL) == 0) {
	Tk_DoWhenIdle(DisplayTurndial, (ClientData) turndialPtr);
    }
    turndialPtr->flags |= what;
}

/*
 *--------------------------------------------------------------
 *
 * RoundToResolution --
 *
 *	Round a given floating-point value to the nearest multiple
 *	of the turndial's resolution.
 *
 * Results:
 *	The return value is the rounded result.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static double
RoundToResolution(turndialPtr, value)
    Turndial *turndialPtr;	/* Information about turndial widget. */
    double value;		/* Value to round. */
{
    double rem;

    if (turndialPtr->resolution <= 0) {
	return value;
    }
    rem = fmod(value, turndialPtr->resolution);
    if (rem < 0) {
	rem = turndialPtr->resolution + rem;
    }
    value -= rem;
    if (rem >= turndialPtr->resolution/2) {
	value += turndialPtr->resolution;
    }
    return value;
}

/*
 *----------------------------------------------------------------------
 *
 * TurndialVarProc --
 *
 *	This procedure is invoked by Tcl whenever someone modifies a
 *	variable associated with a turndial widget.
 *
 * Results:
 *	NULL is always returned.
 *
 * Side effects:
 *	The value displayed in the turndial will change to match the
 *	variable's new value.  If the variable has a bogus value then
 *	it is reset to the value of the turndial.
 *
 *----------------------------------------------------------------------
 */

    /* ARGSUSED */
static char *
TurndialVarProc(clientData, interp, name1, name2, flags)
    ClientData clientData;	/* Information about button. */
    Tcl_Interp *interp;		/* Interpreter containing variable. */
    char *name1;		/* Name of variable. */
    char *name2;		/* Second part of variable name. */
    int flags;			/* Information about what happened. */
{
    register Turndial *turndialPtr = (Turndial *) clientData;
    char *stringValue, *end, *result;
    double value;

    /*
     * If the variable is unset, then immediately recreate it unless
     * the whole interpreter is going away.
     */

    if (flags & TCL_TRACE_UNSETS) {
	if ((flags & TCL_TRACE_DESTROYED) && !(flags & TCL_INTERP_DESTROYED)) {
	    Tcl_TraceVar(interp, turndialPtr->varName,
		    TCL_GLOBAL_ONLY|TCL_TRACE_WRITES|TCL_TRACE_UNSETS,
		    TurndialVarProc, clientData);
	    turndialPtr->flags |= NEVER_SET;
	    SetTurndialValue(turndialPtr, turndialPtr->value, 1, 0);
	}
	return (char *) NULL;
    }

    /*
     * If we came here because we updated the variable (in SetTurndialValue),
     * then ignore the trace.  Otherwise update the turndial with the value
     * of the variable.
     */

    if (turndialPtr->flags & SETTING_VAR) {
	return (char *) NULL;
    }
    result = NULL;
    stringValue = Tcl_GetVar(interp, turndialPtr->varName, TCL_GLOBAL_ONLY);
    if (stringValue != NULL) {
	/*printf("string=%s\n",stringValue);*/
	value = strtod(stringValue, &end);
	/*printf("value=%f\n",value);*/
	if ((end == stringValue) || (*end != 0)) {
	    result = "can't assign non-numeric value to turndial variable";
	} else {
	    turndialPtr->value = value;
	}

	/*
	 * This code is a bit tricky because it sets the turndial's value before
	 * calling SetTurndialValue.  This way, SetTurndialValue won't bother to
	 * set the variable again or to invoke the -command.  However, it
	 * also won't redisplay the turndial, so we have to ask for that
	 * explicitly.
	 */

	SetTurndialValue(turndialPtr, turndialPtr->value, 0, 0);
	/*SetTurndialValue(turndialPtr, turndialPtr->value, 1, 0);*/
	EventuallyRedrawTurndial(turndialPtr, REDRAW_KNOB);
    }

    return result;
}
