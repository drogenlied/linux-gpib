/*
 * tkDial.c --
 *
 *      This module implements a dial widget for the Tk
 *      toolkit.
 *
 *  This code was written by Francis Gardner, Linton Miller and
 *  Richard Dearden.
 *
 * Copyright 1992 Regents of the University of Victoria, Wellington, NZ.
 * This code includes portions of code from the University of Illinois dial
 * widget so the code is copyright by that institution too.
 * (Find and paste Illinois copyright notice :-)
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
 *  need to pad out border, needle and text as they overlap on occasions
 *     (the required padding is radius dependent!!)
 *
 *  TO DO:
 *  choose a better colour scheme
 *  choose a better alternative colour scheme
 *
 *  EXTENSIONS:
 *  add an "activebackground" configuration option
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <tcl.h>
#include <tk.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/* Dial default values */

        /* for colour displays */
#define DEF_DIAL_BG_COLOR               "gray77"
#define DEF_TICK_COLOR                  "blue2"
#define DEF_MAJOR_TICK_COLOR            "blue4"
#define DEF_NEEDLE_COLOR                "red"
#define DEF_TEXT_COLOR                  "black"
#define DEF_DIAL_CURSOR                 ((char *) NULL)

#define ALT_DIAL_BG_COLOR               "red2"
#define ALT_TICK_COLOR                  "white"
#define ALT_MAJOR_TICK_COLOR            "gray70"
#define ALT_NEEDLE_COLOR                "green"
#define ALT_TEXT_COLOR                  "white"

        /* for monochrome displays */
#define DEF_DIAL_BG_MONO                "white"
#define DEF_TICK_MONO                   "black"
#define DEF_MAJOR_TICK_MONO             "black"
#define DEF_NEEDLE_MONO                 "black"
#define DEF_TEXT_MONO                   "black"

#define ALT_DIAL_BG_MONO                "black"
#define ALT_TICK_MONO                   "white"
#define ALT_MAJOR_TICK_MONO             "white"
#define ALT_NEEDLE_MONO                 "white"
#define ALT_TEXT_MONO                   "white"

        /* various other defaults */
#define DIAL_CLASS_NAME                 "Dial"
#define DEF_DIAL_WINDOW_WIDTH           "125"
#define DEF_DIAL_BORDER_WIDTH           "3"
#define DEF_DIAL_FONT                   "*-Helvetica-Bold-R-Normal-*-120-*"
#define DEF_DIAL_TEXT                   (char *)NULL
#define DEF_DIAL_RELIEF                 "raised"
#define DEF_DIAL_MIN_VALUE              "0"
#define DEF_DIAL_MAX_VALUE              "1000"
#define DEF_DIAL_BEGIN_DEGREE           "300"
#define DEF_DIAL_END_DEGREE             "60"
#define DEF_SHOW_VALUE_FLAG             "true"
#define DEF_DIAL_NUM_TICKS              "30"
#define DEF_DIAL_MAJOR_TICK             "5"
#define DEF_DIAL_CALLBACK_INTERVAL      "500"   /* 500 milliseconds */
#define DEF_DIAL_RADIUS                 "60"

#define  PADDING                 2
#ifndef PI
#define  PI                      3.14159265358979
#endif
#define  PIHALF                  (PI / 2)
#define  TWOPI                   (PI * 2)
#define  NEEDLE_FRACTION         0.85
#define  NEEDLE_WIDTH_FRACTION   0.05
#define  NEEDLE_POINTS           4
#define  MAXTICKS                360
#define  MAJOR_TICK              10
#define  FONTPAD                 2
#define  min(a, b)               ((a) < (b) ? (a) : (b))
#define  max(a, b)               ((a) > (b) ? (a) : (b))
#define  round(x)                (x >= 0.0 ? (int)(x + .5) : (int)(x - .5))
#define  fontheight(f)           (f->max_bounds.ascent+f->max_bounds.descent+2*FONTPAD)
#define  norm(w, v)              ((v) - (w->min_value)) /      \
                                 ((w->max_value) - (w->min_value))
#define  hasatitle(d)            (((d)->title != NULL) && \
                                  ((d)->title[0] != '\0'))
#define  degrees2radians(d)      ( TWOPI * ((double)(d) / 360) )
#define  angle_between(d1, d2)   ( (d2)>(d1) ? (d2)-(d1) : (360-(d1)+(d2)) )

/*
 * A data structure of the following type is kept for each
 * widget managed by this file:
 */

typedef struct Dial_struct Dial;
struct Dial_struct {

    /*
     * General stuff
     */
    Tk_Window tkwin;            /* Window that embodies the dial.  NULL
                                 * means that the window has been destroyed. */
    Display *disp;
    Tcl_Interp *interp;         /* Interpreter associated with button.       */
    int displaybits;            /* specifies how much of dial to display     */
    Cursor cursor;		/* Current cursor for window, or None.  -jules- */
    long userbits;              /* a place for user data                     */
    char *userdata;

    /*
     * Border stuff
     */

    Tk_3DBorder border;         /* Structure used to draw 3-D
                                 * border                             */
    int borderWidth;            /* Width of border.                   */
    int relief;                 /* 3-d effect: TK_RELIEF_RAISED, etc. */

    /*
     * Needle stuff
     */

    double min_value;               /* minimum dial value            */
    double max_value;               /* maximum dial value            */
    double value;                   /* dial value                    */
    double oldvalue;                /* previous dial value           */
    unsigned int radius;            /* radius factor                 */
    unsigned int needle_length;     /* needle length - calculated    */
    unsigned int needle_width;      /* needle width - calculated     */
    Tk_3DBorder needleBorder;       /* structure used to draw needle */
    unsigned int centerX;           /* dial center x position        */
    unsigned int centerY;           /* dial center y position        */
    XPoint needle[NEEDLE_POINTS];   /* the points that make up
                                     * the needle polygon            */
    /*
     * Tick stuff
     */

    GC tickGC;                      /* ticks GC                      */
    GC majortickGC;                 /* major ticks GC                */
    XColor *tickColorPtr;           /* color for dial tick marks     */
    XColor *majortickColorPtr;      /* color for major tick marks    */
    unsigned int numTicks;          /* the number of ticks to draw   */
    unsigned int majorTick;         /* major tick interval           */
    XSegment ticks[MAXTICKS];       /* line segments for ticks       */
    XSegment majorticks[MAXTICKS];  /* line segments for maror ticks */

    unsigned int begin_degree;
    unsigned int end_degree;        /* ticks and the needle are drawn
                                     * in between the angles of
                                     * begin_degree and end_degree   */
    /*
     * Text stuff
     */

    XFontStruct *fontPtr;       /* Information about text font, or NULL. */
    XColor *textColorPtr;       /* Color for drawing text.               */
    GC textGC;                  /* GC for drawing text.                  */
    int showvalue;              /* boolean flag for whether or not to
                                 * display textual value                 */
    char *title;                /* title for dial - malloc()ed           */

    /*
     *  Callback stuff
     */

    unsigned long interval;   /* interval (mS) between callbacks             */
    char *command;            /* command involked during the callback        */
    int continue_callback;    /* boolean flag used to terminate the callback */
    int (*callback_func)(Tcl_Interp *,
			 Dial *); /* Function to be invoked on callback. */

    /*
     *  Alternative colour scheme
     */

    Tk_3DBorder altborder;        /* Structure used to draw border  */
    Tk_3DBorder altneedleBorder;  /* structure used to draw needle  */
    XColor *a_textColor;          /* alternative text color         */
    XColor *a_tickColor;          /* alternative tick color         */
    XColor *a_majortickColor;     /* alternative major tick color   */

};

/*
 *  displaybits:
 *  DISPLAY_TICKS - draw the tick points
 *  DISPLAY_NEEDLE - draw the needle
 *  DISPLAY_BORDER - draw the border
 */

#define   DISPLAY_TICKS           1
#define   DISPLAY_NEEDLE          2
#define   DISPLAY_BORDER          4
#define   DISPLAY_VALUE           8
#define   DISPLAY_TITLE          16
#define   REDRAW_PENDING         32
#define   CLEAR_NEEDED           64

#define   DISPLAY_ALL_BUT_TEXT   (DISPLAY_TICKS | DISPLAY_NEEDLE |         \
                                  DISPLAY_BORDER)

#define   DISPLAY_ALL            (DISPLAY_TICKS | DISPLAY_NEEDLE |        \
                                  DISPLAY_BORDER | DISPLAY_VALUE |        \
                                  DISPLAY_TITLE)

/*
 * Information used for parsing configuration specs:
 */

static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_BORDER, "-altbackground", "background", "Background",
        ALT_DIAL_BG_COLOR, Tk_Offset(Dial, altborder),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altbackground", "background", "Background",
        ALT_DIAL_BG_MONO, Tk_Offset(Dial, altborder),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altbg", "altbackground", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-altmajortickcolor", "majortickcolor", "Foreground",
        ALT_MAJOR_TICK_MONO, Tk_Offset(Dial, a_majortickColor),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_COLOR, "-altmajortickcolor", "majortickcolor", "Foreground",
        ALT_MAJOR_TICK_COLOR, Tk_Offset(Dial, a_majortickColor),
        TK_CONFIG_COLOR_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altmajortickcolour", "altmajortickcolor",
        (char *)NULL, (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altmc", "altmajortickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-altnc", "altneedlecolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_BORDER, "-altneedlecolor", "altneedlecolor", "Foreground",
       ALT_NEEDLE_COLOR, Tk_Offset(Dial, altneedleBorder),
       TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-altneedlecolor", "altneedlecolor", "Foreground",
       ALT_NEEDLE_MONO, Tk_Offset(Dial, altneedleBorder),
       TK_CONFIG_MONO_ONLY},

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-altneedlecolour", "altneedlecolor", (char *) NULL,
        (char *) NULL, 0, 0},
    {TK_CONFIG_SYNONYM, "-alttc", "alttickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif

    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
        ALT_TEXT_COLOR, Tk_Offset(Dial, a_textColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttextcolor", "textcolor", "Foreground",
       ALT_TEXT_MONO, Tk_Offset(Dial, a_textColor),
       TK_CONFIG_MONO_ONLY},

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttextcolour", "alttextcolor", (char *)NULL,
       (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_TICK_COLOR, Tk_Offset(Dial, a_tickColor),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-alttickcolor", "tickcolor", "Foreground",
        ALT_TICK_MONO, Tk_Offset(Dial, a_tickColor),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-alttickcolour", "alttickcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif

    {TK_CONFIG_BORDER, "-background", "background", "Background",
        DEF_DIAL_BG_COLOR, Tk_Offset(Dial, border),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-background", "background", "Background",
        DEF_DIAL_BG_MONO, Tk_Offset(Dial, border),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_SYNONYM, "-bd", "borderwidth", (char *)NULL,
        (char *) NULL, 0, 0},
    {TK_CONFIG_INT, "-begindegree", "begindegree", "Begindegree",
        DEF_DIAL_BEGIN_DEGREE, Tk_Offset(Dial, begin_degree), 0 },
    {TK_CONFIG_SYNONYM, "-bg", "background", (char *)NULL,
        (char *)NULL, 0, 0},
    {TK_CONFIG_INT, "-borderwidth", "borderwidth", "Borderwidth",
        DEF_DIAL_BORDER_WIDTH, Tk_Offset(Dial, borderWidth), 0},
    {TK_CONFIG_STRING, "-command", "command", "Command",
        DEF_DIAL_TEXT, Tk_Offset(Dial, command), 0},

    {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
       DEF_DIAL_CURSOR, Tk_Offset(Dial, cursor),
       TK_CONFIG_NULL_OK}, 

    {TK_CONFIG_STRING, "-data", "data", "Data",
        (char *) NULL, Tk_Offset(Dial, userdata), 0},
    {TK_CONFIG_INT, "-enddegree", "enddegree", "Enddegree",
        DEF_DIAL_END_DEGREE, Tk_Offset(Dial, end_degree), 0 },
    {TK_CONFIG_FONT, "-font", "font", "Font",
        DEF_DIAL_FONT, Tk_Offset(Dial, fontPtr), 0},
    {TK_CONFIG_INT, "-interval", "interval", "Interval",
        DEF_DIAL_CALLBACK_INTERVAL, Tk_Offset(Dial, interval), 0},
    {TK_CONFIG_COLOR, "-majortickcolor", "majortickcolor", "Foreground",
        DEF_MAJOR_TICK_MONO, Tk_Offset(Dial, majortickColorPtr),
        TK_CONFIG_MONO_ONLY},
    {TK_CONFIG_COLOR, "-majortickcolor", "majortickcolor", "Foreground",
        DEF_MAJOR_TICK_COLOR, Tk_Offset(Dial, majortickColorPtr),
        TK_CONFIG_COLOR_ONLY},

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-majortickcolour", "majortickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif

    {TK_CONFIG_INT, "-majorticks", "majorticks", "Majorticks",
       DEF_DIAL_MAJOR_TICK, Tk_Offset(Dial, majorTick), 0},
    {TK_CONFIG_DOUBLE, "-maxvalue", "maxvalue", "Maxvalue",
       DEF_DIAL_MAX_VALUE, Tk_Offset(Dial, max_value), 0 },

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-mc", "majortickcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif

    {TK_CONFIG_DOUBLE, "-minvalue", "minvalue", "Minvalue",
       DEF_DIAL_MIN_VALUE, Tk_Offset(Dial, min_value), 0 },

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-nc", "needlecolor", (char *)NULL,
       (char *)NULL, 0, 0},
#endif
    
    {TK_CONFIG_BORDER, "-needlecolor", "needlecolor", "Foreground",
       DEF_NEEDLE_COLOR, Tk_Offset(Dial, needleBorder),
       TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_BORDER, "-needlecolor", "needlecolor", "Foreground",
       DEF_NEEDLE_MONO, Tk_Offset(Dial, needleBorder),
       TK_CONFIG_MONO_ONLY},

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-needlecolour", "needlecolor", (char *) NULL,
       (char *) NULL, 0, 0},
#endif

    {TK_CONFIG_INT, "-numticks", "numticks", "Numticks",
       DEF_DIAL_NUM_TICKS, Tk_Offset(Dial, numTicks), 0},
    {TK_CONFIG_INT, "-radius", "radius", "Radius", DEF_DIAL_RADIUS,
       Tk_Offset(Dial, radius), 0},
    {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
       DEF_DIAL_RELIEF, Tk_Offset(Dial, relief), 0},
    {TK_CONFIG_BOOLEAN, "-showvalue", "showvalue", "Showvalue",
       DEF_SHOW_VALUE_FLAG, Tk_Offset(Dial, showvalue), 0},

#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-tc", "tickcolor", (char *)NULL,
       (char *)NULL, 0, 0},
#endif

    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Foreground",
        DEF_TEXT_COLOR, Tk_Offset(Dial, textColorPtr),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Foreground",
        DEF_TEXT_MONO, Tk_Offset(Dial, textColorPtr),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-textcolour", "textcolor", (char *)NULL,
        (char *)NULL, 0, 0},
#endif
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Foreground",
        DEF_TICK_COLOR, Tk_Offset(Dial, tickColorPtr),
        TK_CONFIG_COLOR_ONLY},
    {TK_CONFIG_COLOR, "-tickcolor", "tickcolor", "Foreground",
        DEF_TICK_MONO, Tk_Offset(Dial, tickColorPtr),
        TK_CONFIG_MONO_ONLY},
#ifdef SYNONYM
    {TK_CONFIG_SYNONYM, "-tickcolour", "tickcolor", (char *) NULL,
        (char *) NULL, 0, 0},
#endif
    {TK_CONFIG_STRING, "-title", "title", "Title",
        DEF_DIAL_TEXT, Tk_Offset(Dial, title), 0},
    {TK_CONFIG_INT, "-userbits", "userbits", "Userbits",
        "0", Tk_Offset(Dial, userbits), 0 },
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
        (char *) NULL, 0, 0}
};

/*
 * Forward declarations for procedures defined later in this file:
 */

static void             Callback _ANSI_ARGS_((Dial *DialPtr));
static void             ComputeDialGeometry _ANSI_ARGS_((Dial *DialPtr));
static void             ComputeNeedlePoints _ANSI_ARGS_((Dial *w));
static void             ComputeTickPoints _ANSI_ARGS_((Dial *w,
                            unsigned int length, double fraction,
                            XSegment *s));
static int              ConfigureDial _ANSI_ARGS_((Tcl_Interp *interp,
                            Dial *DialPtr, int argc, char **argv,
                            int flags));
static void             DestroyDial _ANSI_ARGS_((ClientData clientData));
static void             DialEventProc _ANSI_ARGS_((ClientData clientData,
                            XEvent *eventPtr));
static int              DialWidgetCmd _ANSI_ARGS_((ClientData clientData,
                            Tcl_Interp *interp, int argc, char **argv));
static void             DisplayDial _ANSI_ARGS_((ClientData clientData));
static void             DrawText _ANSI_ARGS_((Dial *DialPtr, int displaybits));
static void             DrawTickPoints _ANSI_ARGS_((Dial *DialPtr));
static void             EventuallyRedrawDial _ANSI_ARGS_((Dial *DialPtr,
                            int displaybits));
static void             ReplaceColours _ANSI_ARGS_((Dial *DialPtr, int argc,
			    char **argv));
static void             SetDialValue _ANSI_ARGS_((Dial *DialPtr, double value));
static void             SwapColours _ANSI_ARGS_((Dial *DialPtr));

/*
 *--------------------------------------------------------------
 *
 * Tk_DialCmd --
 *
 *      This procedure is invoked to process the "Dial" Tcl
 *      command.  See the user documentation for details on what
 *      it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *--------------------------------------------------------------
 */

int Tk_DialCmd(clientData, interp, argc, argv)
    ClientData clientData;      /* Main window associated with
                                 * interpreter. */
    Tcl_Interp *interp;         /* Current interpreter. */
    int argc;                   /* Number of arguments. */
    char **argv;                /* Argument strings. */
{
    Tk_Window tkwin = (Tk_Window) clientData;
    register Dial *DialPtr;
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
     * Initialize fields that won't be initialized by ConfigureDial,
     * or which ConfigureDial expects to have reasonable values
     * (e.g. resource pointers).
     */

    DialPtr = (Dial *) malloc(sizeof(Dial));

    /*
     * I don't know how safe it is to return TCL_ERROR should the memory
     * allocation fail.  Maybe "goto error" is more appropriate.
     */
    if (DialPtr == NULL) {
      fprintf( stderr, "Failed to allocate %d bytes for the Dial structure\n",
              sizeof(Dial) );
      return TCL_ERROR;
    }
    DialPtr->tkwin = new;
    DialPtr->disp  = Tk_Display(new);
    DialPtr->interp = interp;
    DialPtr->border = NULL;                /* these *must* be set to NULL */
    DialPtr->needleBorder = NULL;          /* to be configured by Tk      */
    DialPtr->relief = TK_RELIEF_RAISED;
    DialPtr->cursor = None;	           /* -jules- */
    DialPtr->value = 0;
    DialPtr->fontPtr = NULL;
    DialPtr->title = NULL;
    DialPtr->command = NULL;
    DialPtr->userdata = NULL;
    DialPtr->tickGC = None;
    DialPtr->textGC = None;
    DialPtr->majortickGC = None;
    DialPtr->textColorPtr = NULL;
    DialPtr->tickColorPtr = NULL;
    DialPtr->majortickColorPtr = NULL;

    DialPtr->altborder = NULL;
    DialPtr->altneedleBorder = NULL;
    DialPtr->a_textColor = NULL;
    DialPtr->a_tickColor = NULL;
    DialPtr->a_majortickColor = NULL;

    DialPtr->continue_callback = FALSE;
    DialPtr->displaybits = 0;
    DialPtr->callback_func = NULL;
    
    Tk_SetClass(DialPtr->tkwin, DIAL_CLASS_NAME);
    Tk_CreateEventHandler(DialPtr->tkwin, ExposureMask|StructureNotifyMask,
            DialEventProc, (ClientData) DialPtr);
    Tcl_CreateCommand(interp, Tk_PathName(DialPtr->tkwin), DialWidgetCmd,
            (ClientData) DialPtr, (void (*)()) NULL);
    if (ConfigureDial(interp, DialPtr, argc-2, argv+2, 0) != TCL_OK) {
        goto error;
    }

    interp->result = Tk_PathName(DialPtr->tkwin);
    return TCL_OK;

    error:
    Tk_DestroyWindow(DialPtr->tkwin);
    return TCL_ERROR;
}

/*
 *--------------------------------------------------------------
 *
 * DialWidgetCmd --
 *
 *      This procedure is invoked to process the Tcl command
 *      that corresponds to a widget managed by this module.
 *      See the user documentation for details on what it does.
 *
 * Results:
 *      A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int DialWidgetCmd(clientData, interp, argc, argv)
    ClientData clientData;              /* Information about Dial
                                         * widget. */
    Tcl_Interp *interp;                 /* Current interpreter. */
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Argument strings. */
{
    register Dial *DialPtr = (Dial *) clientData;
    int result = TCL_OK;
    int length;
    char c;


    if (argc < 2) {
        Tcl_AppendResult(interp, "wrong # args: should be \"",
                argv[0], " option ?arg arg ...?\"", (char *) NULL);
        return TCL_ERROR;
    }
    Tk_Preserve((ClientData) DialPtr);
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)) {
        if (argc == 2) {
            result = Tk_ConfigureInfo(interp, DialPtr->tkwin, configSpecs,
                    (char *) DialPtr, (char *) NULL, 0);
        } else if (argc == 3) {
            result = Tk_ConfigureInfo(interp, DialPtr->tkwin, configSpecs,
                    (char *) DialPtr, argv[2], 0);
        } else {
            result = ConfigureDial(interp, DialPtr, argc-2, argv+2,
                    TK_CONFIG_ARGV_ONLY);
        }
    } else if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
        if (argc != 2) {
            Tcl_AppendResult(interp, "wrong # args: should be \"",
                    argv[0], " get\"", (char *) NULL);
            goto error;
        }
        sprintf(interp->result, "%g", DialPtr->value);
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
        SetDialValue(DialPtr, value);
    } else if ((c == 's') && (strncmp(argv[1], "start", length) == 0)) {
      if (! DialPtr->continue_callback ) {
        DialPtr->continue_callback = TRUE;
        Callback(DialPtr);
      }
    } else if ((c == 's') && (strncmp(argv[1], "stop", length) == 0)) {
        DialPtr->continue_callback = FALSE;
    } else if ((c == 's') && (strncmp(argv[1], "swapcolours", length) == 0)) {
      SwapColours(DialPtr);
    } else if ((c == 'r') && (strncmp(argv[1],"replacecolours",length) == 0)) {
      ReplaceColours(DialPtr, argc-2, argv+2);
    } else {
        sprintf(interp->result,
                "bad option \"%.50s\":  must be configure, get, set, start,"
                " stop, swapcolours or replacecolours", argv[1]);
        goto error;
    }
    Tk_Release((ClientData) DialPtr);
    return result;

    error:
    Tk_Release((ClientData) DialPtr);
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


static void Callback(DialPtr)
     Dial *DialPtr;
{
  int no_errors = TRUE;
  int result;
  int nval;

  if (DialPtr->callback_func != NULL) 
    SetDialValue(DialPtr,
		 (*(DialPtr->callback_func))(DialPtr->interp,DialPtr));
    
  if ( DialPtr->command != NULL && DialPtr->command[0] != '\0' ) {
    result = Tcl_Eval( DialPtr->interp, DialPtr->command );
    if ( result != TCL_OK ) {
      /*TkBindError(DialPtr->interp);*/
      no_errors = FALSE;
    }
  }

  if ( DialPtr->continue_callback && no_errors )
    Tk_CreateTimerHandler( DialPtr->interval, (void *)Callback,
                          (ClientData) DialPtr );

  return;
}

/*
 *----------------------------------------------------------------------
 *
 * DestroyDial --
 *
 *      This procedure is invoked by Tk_EventuallyFree or Tk_Release
 *      to clean up the internal structure of a dial at a safe time
 *      (when no-one is using it anymore).
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Everything associated with the dial is freed up.
 *
 *----------------------------------------------------------------------
 */

  static void DestroyDial(clientData)
    ClientData clientData;      /* Info about Dial widget. */
{
    register Dial *DialPtr = (Dial *) clientData;


    if (DialPtr->border != NULL)
      Tk_Free3DBorder(DialPtr->border);

    if (DialPtr->altborder != NULL)
      Tk_Free3DBorder(DialPtr->altborder);

    if (DialPtr->needleBorder != NULL)
      Tk_Free3DBorder(DialPtr->needleBorder);

    if (DialPtr->altneedleBorder != NULL)
      Tk_Free3DBorder(DialPtr->altneedleBorder);

    if (DialPtr->fontPtr != NULL)
      Tk_FreeFontStruct(DialPtr->fontPtr);

    if (DialPtr->textColorPtr != NULL)
      Tk_FreeColor(DialPtr->textColorPtr);

    if (DialPtr->tickColorPtr != NULL)
      Tk_FreeColor(DialPtr->tickColorPtr);

    if (DialPtr->majortickColorPtr != NULL)
      Tk_FreeColor(DialPtr->majortickColorPtr);

    if (DialPtr->a_textColor != NULL)
      Tk_FreeColor(DialPtr->a_textColor);

    if (DialPtr->a_tickColor != NULL)
      Tk_FreeColor(DialPtr->a_tickColor);

    if (DialPtr->a_majortickColor != NULL)
      Tk_FreeColor(DialPtr->a_majortickColor);

    if (DialPtr->textGC != None)
      Tk_FreeGC(DialPtr->disp,DialPtr->textGC);

    if (DialPtr->tickGC != None)
      Tk_FreeGC(DialPtr->disp,DialPtr->tickGC);

    if (DialPtr->majortickGC != None)
      Tk_FreeGC(DialPtr->disp,DialPtr->majortickGC);

    if (DialPtr->title != NULL)
      ckfree(DialPtr->title);

    if (DialPtr->userdata != NULL)
      ckfree(DialPtr->userdata);

    if (DialPtr->command != NULL)
      ckfree(DialPtr->command);

    if (DialPtr->cursor != None) {
	Tk_FreeCursor(DialPtr->disp,DialPtr->cursor);	/* -jules- */
    }

    free((char *) DialPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ConfigureDial --
 *
 *      This procedure is called to process an argv/argc list, plus
 *      the Tk option database, in order to configure (or
 *      reconfigure) a Dial widget.
 *
 * Results:
 *      The return value is a standard Tcl result.  If TCL_ERROR is
 *      returned, then interp->result contains an error message.
 *
 * Side effects:
 *      Configuration information, such as colors, border width,
 *      etc. get set for DialPtr;  old resources get freed,
 *      if there were any.
 *
 *----------------------------------------------------------------------
 */

static int ConfigureDial(interp, DialPtr, argc, argv, flags)
    Tcl_Interp *interp;         /* Used for error reporting. */
    register Dial *DialPtr;     /* Information about widget;  may or may
                                 * not already have values for some fields. */
    int argc;                   /* Number of valid entries in argv. */
    char **argv;                /* Arguments. */
    int flags;                  /* Flags to pass to Tk_ConfigureWidget. */
{
    XGCValues gcValues;
    GC newGC;

    if (Tk_ConfigureWidget(interp, DialPtr->tkwin, configSpecs,
            argc, argv, (char *) DialPtr, flags) != TCL_OK) {
        return TCL_ERROR;
    }

    Tk_SetBackgroundFromBorder(DialPtr->tkwin, DialPtr->border);

    gcValues.font = DialPtr->fontPtr->fid;
    gcValues.foreground = DialPtr->textColorPtr->pixel;
    newGC = Tk_GetGC(DialPtr->tkwin, GCForeground|GCFont, &gcValues);
    if (DialPtr->textGC != None) {
      Tk_FreeGC(DialPtr->disp,DialPtr->textGC);
    }
    DialPtr->textGC = newGC;

    gcValues.foreground = DialPtr->tickColorPtr->pixel;
    newGC = Tk_GetGC(DialPtr->tkwin, GCForeground, &gcValues);
    if (DialPtr->tickGC != None) {
      Tk_FreeGC(DialPtr->disp,DialPtr->tickGC);
    }
    DialPtr->tickGC = newGC;

    gcValues.foreground = DialPtr->majortickColorPtr->pixel;
    newGC = Tk_GetGC(DialPtr->tkwin, GCForeground, &gcValues);
    if (DialPtr->majortickGC != None) {
      Tk_FreeGC(DialPtr->disp,DialPtr->majortickGC);
    }
    DialPtr->majortickGC = newGC;

    /*
     * Recompute display-related information, and let the geometry
     * manager know how much space is needed now.
     */

    ComputeDialGeometry(DialPtr);

    EventuallyRedrawDial(DialPtr, DISPLAY_ALL | CLEAR_NEEDED);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ComputeDialGeometry --
 *
 *      This procedure is called to compute various geometrical
 *      information for a Dial, such as where various things get
 *      displayed.  It's called when the window is reconfigured.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The
 *      geometry manager gets told about the window's preferred size.
 *
 *----------------------------------------------------------------------
 */

/*
 *  returns TRUE if angle a lies between begin degree d1 and end degree d2
 */
#define incl(d1,d2,a) ( (d1) < (d2) ? ((d1) <= (a) && (a) <= (d2))   \
                                    : ((d1) <= (a) || (a) <= (d2)) )

static void ComputeDialGeometry(DialPtr)
    register Dial *DialPtr;             /* Information about widget. */
{
  int width;
  int height;
  struct { double x; double y; } p1, p2;
  XPoint minp, maxp;
  int fh = fontheight( DialPtr->fontPtr );
  int tt=0, sv=0;
  int radius = DialPtr->radius;
  int bd = DialPtr->begin_degree, ed = DialPtr->end_degree;

/*
 *  Calculate some other stuff that is used later
 */
  DialPtr->needle_length = NEEDLE_FRACTION * DialPtr->radius;
  DialPtr->needle_width = NEEDLE_WIDTH_FRACTION * DialPtr->radius;

/*
 *  p1 and p2 are the cartesian coordinates derived from the polar
 *  coordinates using radius, begindegree and enddegree.
 */
  p1.x = (double)radius * sin( degrees2radians( bd ) );
  p1.y = (double)radius * cos( degrees2radians( bd ) );
  p2.x = (double)radius * sin( degrees2radians( ed ) );
  p2.y = (double)radius * cos( degrees2radians( ed ) );

/*
 *  minp and maxp define a rectangle that encloses the circle segment
 *  defined by p1 and p2.
 */
  maxp.y = incl(bd, ed, 0)   ?  radius : (int)(max(0, max(p1.y, p2.y))+0.5);
  maxp.x = incl(bd, ed, 90)  ?  radius : (int)(max(0, max(p1.x, p2.x))+0.5);
  minp.y = incl(bd, ed, 180) ? -radius : (int)(min(0, min(p1.y, p2.y))-0.5);
  minp.x = incl(bd, ed, 270) ? -radius : (int)(min(0, min(p1.x, p2.x))-0.5);

/*
 *  Calculate where the center of dial is in window coordinates.
 *  Compensate for the title and textual value.
 */
  DialPtr->centerX = -minp.x + PADDING + DialPtr->borderWidth +
                     DialPtr->needle_width;
  DialPtr->centerY = maxp.y + PADDING + DialPtr->borderWidth +
                     DialPtr->needle_width;

  if ( hasatitle(DialPtr) ) {
    tt = 1;
    DialPtr->centerY += fh;
  }
  if ( DialPtr->showvalue ) {
    sv = 1;
  }

/*
 *  Calculate the width and height of the window.
 */
  width = maxp.x - minp.x + 2*(DialPtr->borderWidth+PADDING) +
          2*DialPtr->needle_width;
  height = maxp.y - minp.y + (tt+sv)*fh + 2*(DialPtr->borderWidth+PADDING) +
           2*DialPtr->needle_width;

/*
 *  Make a request for a window with these dimensions
 */
  Tk_GeometryRequest(DialPtr->tkwin, width, height );
  Tk_SetInternalBorder(DialPtr->tkwin, DialPtr->borderWidth);
}

/*
 *--------------------------------------------------------------
 *
 * DisplayDial --
 *
 *      This procedure redraws the contents of a Dial window.  It
 *      is invoked as a do-when-idle handler, so it only
 *      runs when there's nothing else for the application to do.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Information appears on the screen.
 *
 *--------------------------------------------------------------
 */

static void DisplayDial(clientData)
    ClientData clientData;      /* Information about widget. */
{
  register Dial *DialPtr = (Dial *) clientData;
  register Tk_Window tkwin = DialPtr->tkwin;
/*  int relief;*/
  int displaybits = DialPtr->displaybits;
/*  int fh = fontheight(DialPtr->fontPtr);*/


  DialPtr->displaybits &= ~REDRAW_PENDING;
  if ( (tkwin == NULL) || !Tk_IsMapped(tkwin) ) {
    return;           /* nothing to do */
  }

  /*
   *  Render the dial
   */
  if ( displaybits & CLEAR_NEEDED )
    XClearWindow( Tk_Display(tkwin), Tk_WindowId(tkwin) );

  /* Draw tick points */
  if ( displaybits & DISPLAY_TICKS )
    DrawTickPoints(DialPtr);

  /* Draw needle */
  if ( displaybits & DISPLAY_NEEDLE ) {

    /* Erase needle */
#if TK_MAJOR_VERSION > 3
    if ( ! (displaybits & CLEAR_NEEDED ) )
      Tk_Fill3DPolygon( tkwin, Tk_WindowId(tkwin),
                       DialPtr->border, DialPtr->needle,
                       NEEDLE_POINTS, 0, TK_RELIEF_FLAT );

    ComputeNeedlePoints( DialPtr );
    Tk_Fill3DPolygon( tkwin, Tk_WindowId(tkwin),
                     DialPtr->needleBorder, DialPtr->needle,
                     NEEDLE_POINTS, 0, TK_RELIEF_FLAT );
#else
    if ( ! (displaybits & CLEAR_NEEDED ) )
      Tk_Fill3DPolygon( Tk_Display(tkwin), Tk_WindowId(tkwin),
                       DialPtr->border, DialPtr->needle,
                       NEEDLE_POINTS, 0, TK_RELIEF_FLAT );

    ComputeNeedlePoints( DialPtr );
    Tk_Fill3DPolygon( Tk_Display(tkwin), Tk_WindowId(tkwin),
                     DialPtr->needleBorder, DialPtr->needle,
                     NEEDLE_POINTS, 0, TK_RELIEF_FLAT );
#endif
  }

  /* Draw text */
  DrawText(DialPtr, displaybits);

  /* Draw border */
  if ( displaybits & DISPLAY_BORDER )
#if TK_MAJOR_VERSION > 3
    Tk_Draw3DRectangle( tkwin, Tk_WindowId(tkwin),
                       DialPtr->border, 0, 0, Tk_Width(tkwin),
                       Tk_Height(tkwin), DialPtr->borderWidth,
                       DialPtr->relief );
#else
    Tk_Draw3DRectangle( Tk_Display(tkwin), Tk_WindowId(tkwin),
                       DialPtr->border, 0, 0, Tk_Width(tkwin),
                       Tk_Height(tkwin), DialPtr->borderWidth,
                       DialPtr->relief );
#endif

    DialPtr->displaybits &= ~CLEAR_NEEDED;
}

/*
 *-----------------------------------------------------------------------
 *
 * DrawText --
 *
 *      Draws a title if the user has specified one, and draw some text
 *      corresponding to the value, if the user so desires.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Draws into DialPtr->tkwin
 *
 *----------------------------------------------------------------------
 */

static void DrawText(DialPtr, displaybits)
     Dial *DialPtr;
     int   displaybits;
{
    Tk_Window tkwin = DialPtr->tkwin;
    register XFontStruct *fp = DialPtr->fontPtr;
    register char *title = DialPtr->title;
    int win_width = Tk_Width(tkwin);
    int bd = DialPtr->borderWidth;
    int x, y, dummy, tlength, vlength;
    int twidth, vwidth;     /* width of value and title in pixels */
    int tf = 0, vf = 0;     /* flags for displaying text and values */
    char valueString[30];
    XCharStruct bbox;


    if ( hasatitle(DialPtr) && (displaybits & DISPLAY_TITLE) ) {
      tf = 1;
      tlength = strlen(title);
      twidth = XTextWidth( fp, title, tlength );
    }
    if ( DialPtr->showvalue && (displaybits & DISPLAY_VALUE) ) {
      vf = 1;
      sprintf( valueString, "%g", DialPtr->value );
      vlength = strlen(valueString);
      vwidth = XTextWidth( fp, valueString, vlength );
    }

    if ( hasatitle(DialPtr) && (displaybits & DISPLAY_TITLE) ) {
      y = fp->max_bounds.ascent + FONTPAD + bd;
      XTextExtents(fp, title, tlength, &dummy, &dummy, &dummy, &bbox);
      if (twidth < win_width - 2*bd)
        x = win_width / 2 - (bbox.lbearing + bbox.rbearing)/2;
      else          /* text wider than window - (have X) clip it */
        x = bd + FONTPAD;

      XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), bd, bd,
                 win_width - 2*bd, fontheight(fp), False);
      XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
                  DialPtr->textGC, x, y, title, tlength);
    }

    if (DialPtr->showvalue && (displaybits & DISPLAY_VALUE) ) {
      y = Tk_Height(tkwin) - (fp->max_bounds.descent + FONTPAD) - bd;
      XTextExtents(fp, valueString, vlength, &dummy, &dummy, &dummy, &bbox);
      if (vwidth < win_width - 2*bd)
        x = win_width/2 - (bbox.lbearing + bbox.rbearing)/2;
      else
        x = bd + FONTPAD;

      XClearArea(Tk_Display(tkwin), Tk_WindowId(tkwin), bd,
                 Tk_Height(tkwin) - fontheight(fp) - bd,
                 win_width-2*bd, fontheight(fp), False);
      XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
                  DialPtr->textGC, x, y, valueString, vlength);
  }
}

/*
 *-----------------------------------------------------------------------
 *
 * ComputeNeedlePoints --
 *
 *      Calculates the location of 4 points that form a triangle (the needle).
 *      The needle will point at an angle corresponding to the dial value.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      Draws a polygon in DialPtr->tkwin.
 *
 *-----------------------------------------------------------------------
 */

static void ComputeNeedlePoints(DialPtr)
    Dial *DialPtr;
{
  unsigned int      length = DialPtr->needle_length;
  unsigned int      width = DialPtr->needle_width;
  register double   angle, cosangle, sinangle;
  register double   ws, wc;
  XPoint            *needle = DialPtr->needle;
  double max_val = DialPtr->max_value, min_val = DialPtr->min_value;
  double value = DialPtr->value;
  int bd = DialPtr->begin_degree;
  int ed = DialPtr->end_degree;

/*
 *  This set of tests is done because DialPtr->value is not constrained
 *  by DialPtr->max_value and DialPtr->min_value (as it used to be).
 *  DialPtr->min_value and DialPtr->max_value now represent the *needle*
 *  constraints.  The value can be set to anything, but the needle shows a
 *  value between the constraints.  The textual value, if it is showing
 *  will display the real value.  (i.e the "needle value" and "textual value"
 *  can say show different values, but the textual value always shows the
 *  set value)
 */

  if ( max_val > min_val ) {
    if ( value > max_val )
      value = max_val;
    else
      if ( value < min_val )
        value = min_val;
  } else {      /* max_val <= min_val */
    if ( value < max_val )
      value = max_val;
    else
      if ( value > min_val )
        value = min_val;
  }

/*
 * A full circle is 2 PI radians.  Angles are measured from 12
 * o'dial, dialwise increasing.
 */

  if ( DialPtr->max_value == DialPtr->min_value )
    angle = PIHALF;
  else
    angle = PIHALF - degrees2radians(bd) -
      degrees2radians(angle_between(bd,ed)) * (double) norm(DialPtr, value);

  cosangle = cos(angle);
  sinangle = sin(angle);

/*
 * Order of points of the needle.
 *
 *            0,3
 *            / \
 *           /   \
 *          /     \
 *         1-------2
 */

  wc = round(width * cosangle);
  ws = round(width * sinangle);

  /* 0 */
  needle[0].x = DialPtr->centerX + round(length * cosangle);
  needle[0].y = DialPtr->centerY - round(length * sinangle);

  /* 1 */
  needle[1].x = DialPtr->centerX - round(wc - ws);
  needle[1].y = DialPtr->centerY + round(ws + wc);

  /* 2 */
  needle[2].x = DialPtr->centerX - round(wc + ws);
  needle[2].y = DialPtr->centerY + round(ws - wc);

  /* 3 */
  needle[3].x = DialPtr->needle[0].x;
  needle[3].y = DialPtr->needle[0].y;

}

/*
 *-------------------------------------------------------------------------
 *
 * ComputeTickPoints --
 *
 *      Computes where the ends of a line segment are within the Tk_Window
 *      for a given tick value (fraction).
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      ComputeTickPoints is passed a pointer to an XSegment which is
 *      where the segment points are places.
 *
 *-------------------------------------------------------------------------
 */

static void ComputeTickPoints(DialPtr, length, fraction, s)
    Dial            *DialPtr;
    unsigned int       length;
    double          fraction;
    XSegment        *s;
{
  int           cx = DialPtr->centerX;
  int           cy = DialPtr->centerY;
  double        angle, cosangle, sinangle;
  int bd = DialPtr->begin_degree;
  int ed = DialPtr->end_degree;

/*
 * A full circle is 2 PI radians.  Angles are measured from 12
 * o'clock, dialwise increasing.
 */

  angle = /*PIHALF -*/ degrees2radians(bd) +
                       degrees2radians(angle_between(bd,ed)) * fraction;


  cosangle = cos(angle);
  sinangle = sin(angle);

  s->x1 = cx + (int)(length * sinangle);
  s->y1 = cy - (int)(length * cosangle);
  s->x2 = cx + (int)(DialPtr->radius * sinangle);
  s->y2 = cy - (int)(DialPtr->radius * cosangle);

}

/*
 *-------------------------------------------------------------------------
 *
 * DrawTicksPoints --
 *
 *      Calculates where the major and "minor" tick points are then draws
 *      them.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The DialPtr->ticks and DialPtr->majorticks arrays are modified.
 *
 *-------------------------------------------------------------------------
 */

static void DrawTickPoints(DialPtr)
     Dial *DialPtr;
{
  int i, s, t;
  unsigned int length = DialPtr->needle_length;
  unsigned int delta = (DialPtr->radius - length) / 2;
  register Tk_Window tkwin = DialPtr->tkwin;
  int numTicks, modifier;


  if (DialPtr->numTicks > MAXTICKS) DialPtr->numTicks = MAXTICKS;
  numTicks = DialPtr->numTicks;

/*
 *  The modifier is used to adjust the angle between the ticks.
 *  When the begin and end angles aren't 0 and 360 respectively, the correct
 *  number of ticks are drawn but they don't correspond to the dial value.
 */
  if (DialPtr->begin_degree != 0 || DialPtr->end_degree != 360)
    modifier = -1;
  else
    modifier = 0;


  /* Compute tick points */
  s = t = 0;

  if ( DialPtr->majorTick == 0 )   /* there are no major ticks */
    for (i=0; i<numTicks; i++, s++)
      ComputeTickPoints(DialPtr, length+delta,
                        ((double) i)/(double) (numTicks+modifier),
                        &DialPtr->ticks[s] );
  else                             /* there are some major tick points */
    for (i=0; i<numTicks; i++)
      if ((i % DialPtr->majorTick) == 0) {
        ComputeTickPoints(DialPtr, length,
                          ((double) i)/(double) (numTicks+modifier),
                          &DialPtr->majorticks[t]);
        t++;
      } else {
        ComputeTickPoints(DialPtr, length+delta,
                          ((double) i)/(double) (numTicks+modifier),
                          &DialPtr->ticks[s] );
        s++;
      }

  /* Draw the ticks */
  XDrawSegments(Tk_Display(tkwin), Tk_WindowId(tkwin),
                DialPtr->tickGC, DialPtr->ticks, s);

  /* Draw the major ticks */
  XDrawSegments(Tk_Display(tkwin), Tk_WindowId(tkwin),
                DialPtr->majortickGC, DialPtr->majorticks, t);
}

/*
 *--------------------------------------------------------------
 *
 * DialEventProc --
 *
 *      This procedure is invoked by the Tk dispatcher for various
 *      events on Dials.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      When the window gets deleted, internal structures get
 *      cleaned up.  When it gets exposed, it is redisplayed.
 *
 *--------------------------------------------------------------
 */

static void DialEventProc(clientData, eventPtr)
    ClientData clientData;      /* Information about window. */
    XEvent *eventPtr;           /* Information about event. */
{
    Dial *DialPtr = (Dial *) clientData;

  if ((eventPtr->type == Expose) && (eventPtr->xexpose.count == 0))
    EventuallyRedrawDial(DialPtr, DISPLAY_ALL);
  else if (eventPtr->type == DestroyNotify) {
    Tcl_DeleteCommand(DialPtr->interp, Tk_PathName(DialPtr->tkwin));
    DialPtr->tkwin = NULL;
    Tk_CancelIdleCall(DisplayDial, (ClientData) DialPtr);
    Tk_EventuallyFree((ClientData) DialPtr, DestroyDial);
  } else if (eventPtr->type == ConfigureNotify) {
    ComputeDialGeometry(DialPtr);
  }

}

/*
 *--------------------------------------------------------------
 *
 * SetDialValue --
 *
 *      This procedure changes the value of a Dial
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void SetDialValue(DialPtr, val)
    register Dial *DialPtr;     /* Info about widget  */
    double val;                 /* New value for Dial */
{
    double old_val = DialPtr->value;
    double min_val = DialPtr->min_value;
    double max_val = DialPtr->max_value;

    DialPtr->oldvalue = old_val;
    DialPtr->value = val;

    /*
     * If any of the following cases hold the needle position won't change
     * so just update the textual value.
     */

    if (val == old_val)
      return;             /* don't do anything, nothing has changed */
    else
    if ( (min_val < max_val) && (val < min_val) && (old_val < min_val) )
      EventuallyRedrawDial(DialPtr, DISPLAY_VALUE);
    else
    if ( (min_val < max_val) && (val > max_val) && (old_val > max_val) )
      EventuallyRedrawDial(DialPtr, DISPLAY_VALUE);
    else
    if ( (min_val > max_val) && (val < max_val) && (old_val < max_val) )
      EventuallyRedrawDial(DialPtr, DISPLAY_VALUE);
    else
    if ( (min_val > max_val) && (val > min_val) && (old_val > min_val))
      EventuallyRedrawDial(DialPtr, DISPLAY_VALUE);
    else
    if ( min_val == max_val )
      EventuallyRedrawDial(DialPtr, DISPLAY_VALUE);
    else
      EventuallyRedrawDial(DialPtr, DISPLAY_NEEDLE | DISPLAY_VALUE);

}

/*
 *--------------------------------------------------------------
 *
 * EventuallyRedrawDial --
 *
 *      Arrange for part or all of a Dial widget to redrawn at
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

static void EventuallyRedrawDial(DialPtr, displaybits)
    register Dial *DialPtr;     /* Information about widget. */
    int displaybits;            /* Which parts of the dial are to be drawn */
{
    if ( (DialPtr->tkwin == NULL) || !Tk_IsMapped(DialPtr->tkwin) ||
        (DialPtr->displaybits & REDRAW_PENDING) )
      return;

    DialPtr->displaybits = displaybits | REDRAW_PENDING;
    Tk_DoWhenIdle(DisplayDial, (ClientData) DialPtr);
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
 *     Colour scheme is swapped and dial is redisplayed with
 *     the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void SwapColours(Dial *DialPtr)
{
  Tk_3DBorder tempb;
  XColor     *tempc;

/*
 *  Swap a and b using c as the temporary variable.
 *  NOTE:  This only works because the reference counts to the XColors are
 *         unchanged because the pointers are swapped consistently.
 */
#define SWAP(a,b,c) { (c) = (a); (a) = (b); (b) = (c); }

  SWAP(DialPtr->altborder, DialPtr->border, tempb);
  SWAP(DialPtr->altneedleBorder, DialPtr->needleBorder, tempb);

  SWAP(DialPtr->a_textColor, DialPtr->textColorPtr, tempc);
  SWAP(DialPtr->a_tickColor, DialPtr->tickColorPtr, tempc);
  SWAP(DialPtr->a_majortickColor, DialPtr->majortickColorPtr, tempc);

#undef SWAP

  ConfigureDial(DialPtr->interp, DialPtr, 0, NULL, TK_CONFIG_ARGV_ONLY);
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
 *     Dial is displayed with the new colour scheme.
 *
 *--------------------------------------------------------------
 */

static void ReplaceColours(Dial *DialPtr, int argc, char *argv[])
{
#if TK_MAJOR_VERSION > 3
  DialPtr->altborder = Tk_Get3DBorder(DialPtr->interp,
				      DialPtr->tkwin,
				      Tk_NameOf3DBorder(DialPtr->border));

  DialPtr->altneedleBorder = Tk_Get3DBorder(DialPtr->interp,
					    DialPtr->tkwin,
					    Tk_NameOf3DBorder(DialPtr->needleBorder));

  DialPtr->a_textColor = Tk_GetColorByValue(DialPtr->tkwin,
					    DialPtr->textColorPtr);

  DialPtr->a_tickColor = Tk_GetColorByValue(DialPtr->tkwin,
					    DialPtr->tickColorPtr);

  DialPtr->a_majortickColor = Tk_GetColorByValue(DialPtr->tkwin,
						 DialPtr->majortickColorPtr);
#else
  DialPtr->altborder = Tk_Get3DBorder(DialPtr->interp,
				      DialPtr->tkwin,
				      (Colormap) None,
				      Tk_NameOf3DBorder(DialPtr->border));

  DialPtr->altneedleBorder = Tk_Get3DBorder(DialPtr->interp,
					    DialPtr->tkwin,
					    (Colormap) None,
					    Tk_NameOf3DBorder(DialPtr->needleBorder));

  DialPtr->a_textColor = Tk_GetColorByValue(DialPtr->interp,
					    DialPtr->tkwin,
					    (Colormap) None,
					    DialPtr->textColorPtr);

  DialPtr->a_tickColor = Tk_GetColorByValue(DialPtr->interp,
					    DialPtr->tkwin,
					    (Colormap) None,
					    DialPtr->tickColorPtr);

  DialPtr->a_majortickColor = Tk_GetColorByValue(DialPtr->interp,
						 DialPtr->tkwin,
						 (Colormap) None,
						 DialPtr->majortickColorPtr);
#endif

  ConfigureDial(DialPtr->interp, DialPtr, argc, argv, TK_CONFIG_ARGV_ONLY);
}
