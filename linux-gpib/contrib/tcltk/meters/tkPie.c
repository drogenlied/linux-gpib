/*
 * tkPie.c --
 *
 *      This module implements a piegraph widget for the Tk
 *      toolkit.
 *
 *       Heavily mangled by Julian Anderson 
 *  This code is based on the Dial widget written by Francis Gardner 
 *  and Richard Dearden.
 *
 *
 * Copyright 1992 Regents of the University of Victoria, Wellington, NZ.
 * This code includes portions of code from the University of Illinois dial
 * widget so the code is copyright by that institution too.
 * [Find and paste Illinois copyright notice]
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
#include <tcl.h>
#include <tk.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PIE_ELTS                  20
#define PIE_CLASS_NAME               "Pie"
#define PADDING                       2
#define FONTPAD                       2
#define fontheight(f)            (f->max_bounds.ascent+f->max_bounds.descent+2*FONTPAD)
#define piewidth(pie)            (((pie)->radius + (pie)->borderWidth + (pie)->explodelength)*2)
#define pieoffset(pie)           ((pie)->radius + (pie)->borderWidth + (pie)->explodelength)
#define REDRAW_PENDING 1
#define CLEAR_NEEDED 2
#define KEY_CHANGED 4
#define SHOW_VALUE 4
#define M_DEGREE                23040
#ifndef PI
#define PI                      3.14159265358979
#endif
#define PIHALF                  (PI / 2)
#define TWOPI                   (PI * 2)
#define deg64_2_radians(d)      ( TWOPI * ((double)(d) / M_DEGREE) )
#define angle_between(d1, d2)   ( (d2)>(d1) ? (d2)-(d1) : (360-(d1)+(d2)) )
#define MAX_WEDGES        16


/*
 * On mono monitors we require some way to distinguish pieces. Alter at will.
 */

char bitmap[MAX_WEDGES][8] = {
  { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 }, /* Grey 1/2 */
  { 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00 }, /* Grey 1/4 */
  { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 }, /* Grey 4/25 */
  { 0xff, 0x88, 0x88, 0x88, 0xff, 0x88, 0x88, 0x88 }, /* Squares */
  { 0xff, 0x80, 0x80, 0x80, 0xff, 0x08, 0x08, 0x08 }, /* Brickwork */
  { 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F, 0xF0, 0x0F }, /* Something */
  { 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11 }, /* ne-sw slope */
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, /* white */
  { 0x08, 0x14, 0x22, 0x41, 0x80, 0x40, 0x20, 0x10 }, /* Diag brick */
  { 0x91, 0x62, 0x02, 0x01, 0x80, 0x40, 0x46, 0x89 }, /* funny square */
  { 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18 }, /* diag squares */
  { 0x84, 0x48, 0x00, 0x00, 0x48, 0x84, 0x00, 0x00 }, /* diag dashes */
  { 0x80, 0x80, 0x41, 0x37, 0x08, 0x08, 0x14, 0x72 }, /* scales */
  { 0xCc, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33, 0x99 }, /* fat diags */
  { 0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33 }, /* Fat grey 1/2 */
  { 0xCC, 0x66, 0xCC, 0x66, 0xCC, 0x66, 0xCC, 0x66 }
};

/*
 * Default colors for wedges. I really should think of an elegant way to
 * customise these, but this is reasonable to start off with.
 */

char *colors[MAX_WEDGES] = {
  "green","lightblue","moccasin","red","yellow","SkyBlue","grey75","blue",
  "chocolate", "cyan", "sea green", "gold", "salmon", "hot pink", 
  "lawn green", "black"
};


struct PieElement {
  char *label;
  char *legend;
  double value;
  GC gc;
};

typedef struct PieElement PieElt;

/*
 * A data structure of the following type is kept for each
 * widget managed by this file:
 */

typedef struct {

    /*
     * General stuff
     */
    Tk_Window tkwin;			/* Window that embodies the pie.  NULL
					 * means that the window has been destroyed. */
    Display *display;
    Tcl_Interp *interp;			/* Interpreter associated with button.       */
    int displaybits;			/* specifies how much of pie to display     */
    long userbits;			/* a place for user data                     */
    char *userdata;
    int display_depth;			/* Depth of display. */
    Cursor cursor;			/* Current cursor for window, or None.  -jules- */
    
    /*
     * Border stuff
     */

    Tk_3DBorder border;			/* Structure used to draw 3-D
					 * border                             */ 
    int borderWidth;			/* Width of border.                   */
    int relief;				/* 3-d effect: TK_RELIEF_RAISED, etc. */
    XColor *backgroundColor;		/* Background color                   */

    /*
     * Pie stuff
     */

    XColor *pieColor;
    int radius;
    double total_value;			/* Sum of elements. */
    int element_count;			/* How many elements. */
    PieElt elements[MAX_WEDGES];	/* The wedges themselves. */
    char *exploded;			/* Which element is exploded. */
    int explodelength;			/* How far it is exploded. */
    unsigned int centerX;		/* pie center x position        */
    unsigned int centerY;		/* pie center y position        */

    /*
     * misc configuration options.
     */

    int valuewidth;
    int numdigits;
    int textwidth;
    int percentwidth;
    char *keyorder;			/* permutations of K,L,V,P */
					/* for display order.*/
    int keysize;
    int origin;				/* Origin for pie elements. */
    


    /*
     * Text stuff
     */

    XFontStruct *fontPtr;		/* Information about text font, or NULL. */
    XColor *textColorPtr;		/* Color for drawing text.               */
    GC textGC;				/* GC for drawing text.                  */
    int showvalue;			/* boolean flag for whether or not to
					 * display textual value                 */
    char *title;			/* title for pie - malloc()ed           */
    int titleheight;			/* Height of title for geometry computations */

    /*
     *  Callback stuff
     */

    unsigned long interval;		/* interval (mS) between callbacks             */
    char *command;			/* command involked during the callback        */
    int continue_callback;		/* boolean flag used to terminate the callback */
    
  } Pie;

#define DEF_PIE_CURSOR                 ((char *) NULL)
#define DEF_PIE_BG_COLOR           "white"
#define DEF_PIE_BG_MONO            "white"
#define DEF_TEXT_COLOR             "black"
#define DEF_TEXT_MONO              "black"
#define DEF_PIE_BORDER_WIDTH       "3"
#define DEF_PIE_RADIUS             "40"
#define DEF_PIE_FONT               "*-Helvetica-Bold-R-Normal-*-120-*"
#define DEF_PIE_RELIEF             "raised"
#define DEF_PIE_VALUEWIDTH         "50"
#define DEF_PIE_PERCENTWIDTH       "60"
#define DEF_PIE_TEXTWIDTH          "50"
#define DEF_PIE_ORDER              "KVPL"
					/* <K>ey,<V>alue,<P>ercent,<L>egend*/
#define DEF_PIE_KEYBOX             "100"
#define DEF_PIE_ORIGIN             "0" 
					/* Vertical */
#define DEF_PIE_EXPLODE            "10"
#define DEF_PIE_CALLBACK_INTERVAL  "500"
#define DEF_PIE_NUMDIGITS          "2"


static Tk_ConfigSpec configSpecs[] = {
  {TK_CONFIG_BORDER, "-background", "background", "Background",
     DEF_PIE_BG_COLOR, Tk_Offset(Pie, border),
     TK_CONFIG_COLOR_ONLY},
  {TK_CONFIG_BORDER, "-background", "background", "Background",
     DEF_PIE_BG_MONO, Tk_Offset(Pie, border),
     TK_CONFIG_MONO_ONLY},
/*  {TK_CONFIG_COLOR, "-background", "background", "Background",
     DEF_PIE_BG_MONO, Tk_Offset(Pie, backgroundColor),
     TK_CONFIG_MONO_ONLY},
     {TK_CONFIG_COLOR, "-background", "background", "Background",
     DEF_PIE_BG_COLOR, Tk_Offset(Pie, backgroundColor),
     TK_CONFIG_COLOR_ONLY},
*/
  {TK_CONFIG_INT, "-borderwidth", "borderwidth", "Borderwidth",
     DEF_PIE_BORDER_WIDTH, Tk_Offset(Pie, borderWidth), 0},
  {TK_CONFIG_STRING, "-command", "command", "Command",
     (char *)NULL, Tk_Offset(Pie, command), 0},
  {TK_CONFIG_ACTIVE_CURSOR, "-cursor", "cursor", "Cursor",
     DEF_PIE_CURSOR, Tk_Offset(Pie, cursor), TK_CONFIG_NULL_OK}, 

  {TK_CONFIG_STRING, "-data", "data", "Data",
     (char *) NULL, Tk_Offset(Pie, userdata), 0},
  {TK_CONFIG_STRING, "-explode", "explode", "Explode",
     "", Tk_Offset(Pie,exploded), 0},
  {TK_CONFIG_INT, "-explodewidth", "explodewidth", "Explodewidth",
     DEF_PIE_EXPLODE, Tk_Offset(Pie,explodelength), 0},
  {TK_CONFIG_FONT, "-font", "font", "Font",
     DEF_PIE_FONT, Tk_Offset(Pie, fontPtr), 0},
  {TK_CONFIG_INT, "-interval", "interval", "Interval",
     DEF_PIE_CALLBACK_INTERVAL, Tk_Offset(Pie, interval), 0},
  {TK_CONFIG_INT, "-keyboxsize", "keyboxsize", "Keyboxsize",
     DEF_PIE_KEYBOX, Tk_Offset(Pie,keysize), 0},
  {TK_CONFIG_STRING, "-keyorder", "keyorder", "Keyorder",
     DEF_PIE_ORDER, Tk_Offset(Pie, keyorder), 0},
  {TK_CONFIG_INT, "-numdigits", "numdigits", "Numdigits",
     DEF_PIE_NUMDIGITS, Tk_Offset(Pie,numdigits), 0},
  {TK_CONFIG_INT, "-origin", "origin", "Origin",
     DEF_PIE_ORIGIN, Tk_Offset(Pie, origin), 0},
  {TK_CONFIG_INT, "-percentwidth", "percentwidth", "Percentwidth",
     DEF_PIE_PERCENTWIDTH, Tk_Offset(Pie, percentwidth), 0},
  {TK_CONFIG_INT, "-radius", "radius", "Radius", DEF_PIE_RADIUS,
     Tk_Offset(Pie, radius), 0},
  {TK_CONFIG_RELIEF, "-relief", "relief", "Relief",
     DEF_PIE_RELIEF, Tk_Offset(Pie, relief), 0},
  {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Foreground",
     DEF_TEXT_COLOR, Tk_Offset(Pie, textColorPtr),
     TK_CONFIG_COLOR_ONLY},
  {TK_CONFIG_COLOR, "-textcolor", "textcolor", "Foreground",
     DEF_TEXT_MONO, Tk_Offset(Pie, textColorPtr),
     TK_CONFIG_MONO_ONLY},
  {TK_CONFIG_INT, "-textwidth", "textwidth", "Textwidth",
     DEF_PIE_TEXTWIDTH, Tk_Offset(Pie, textwidth), 0},
  {TK_CONFIG_STRING, "-title", "title", "Title",
     (char *)NULL, Tk_Offset(Pie, title), 0},
  {TK_CONFIG_INT, "-valuewidth", "valuewidth", "Valuewidth",
     DEF_PIE_VALUEWIDTH, Tk_Offset(Pie, valuewidth), 0},
  
  /*
     */

  {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, 0}
};


void DisplayPie(clientData)
     ClientData clientData;
{
  register Pie *piePtr = (Pie *) clientData;
  register Tk_Window tkwin = piePtr->tkwin;
  double foo;
  int fh = fontheight(piePtr->fontPtr);
  double perpt,start;
  double exp_start = 0,exp_diff = 0;
  char *options;
  int i;
  int y = PADDING + piePtr->titleheight;
  Display *disp = Tk_Display(tkwin);
  Drawable draw = Tk_WindowId(tkwin);
  

  if (( piePtr->exploded && piePtr->exploded[0]) 
      || (piePtr->displaybits & CLEAR_NEEDED))
    XClearWindow( Tk_Display(tkwin), Tk_WindowId(tkwin) );
  if (piePtr->displaybits & KEY_CHANGED )
    XClearArea(disp,draw, 
	       piewidth(piePtr),0,
	       Tk_Width(piePtr->tkwin) - piewidth(piePtr),
	       Tk_Height(piePtr->tkwin),0);

  if (piePtr->title && piePtr->title[0]) 
    {
      int tlength = strlen(piePtr->title);
      int twidth = XTextWidth(piePtr->fontPtr,
			      piePtr->title,
			      tlength);
      int win_width = Tk_Width(tkwin);
      int x,dummy;
      XCharStruct bbox;
      
      if (twidth < win_width - 2*piePtr->borderWidth)
	x = win_width / 2 - (twidth/2);
      else          /* text wider than window - (have X) clip it */
	x = piePtr->borderWidth + PADDING;
      XDrawString(Tk_Display(tkwin), Tk_WindowId(tkwin),
		  piePtr->textGC, x, fh , piePtr->title, tlength);
    }
  

  piePtr->displaybits = 0;
/**/
  if( piePtr->total_value == 0 ) perpt = (double) M_DEGREE;
  else
      perpt = (double)M_DEGREE / (double)piePtr->total_value;
  piePtr->origin = piePtr->origin % 360;
  start = (double)(M_DEGREE / 4)-(double)(piePtr->origin*64);
  
  if ((piePtr->element_count == 0) || (piePtr->total_value == 0)) {
    XDrawArc(disp,draw,
	     piePtr->textGC,
	     piePtr->borderWidth + piePtr->explodelength,
	     piePtr->borderWidth + piePtr->explodelength + piePtr->titleheight,
	     piePtr->radius*2,
	     piePtr->radius*2,
	     0,
	     M_DEGREE);
    return;
  }
  for (i = 0; i <piePtr->element_count; i++) {
    start += (start<0.0)?M_DEGREE:((start>M_DEGREE)?-M_DEGREE:0);
    if (strcmp(piePtr->exploded,piePtr->elements[i].label)==0) {
      int xoff,yoff;
      
      exp_start = start;
      exp_diff = -(double)piePtr->elements[i].value * perpt;
      xoff = piePtr->explodelength * 
	cos(deg64_2_radians((double)(exp_start + exp_diff/2)));
      yoff = piePtr->explodelength * 
	sin(deg64_2_radians((double)(exp_start + exp_diff/2)));
      XFillArc(disp,
	       draw,
	       piePtr->elements[i].gc,
	       piePtr->borderWidth + piePtr->explodelength +xoff,
	       piePtr->borderWidth + piePtr->explodelength -yoff
	       + piePtr->titleheight,
	       piePtr->radius*2,
	       piePtr->radius*2,
	       exp_start,
	       exp_diff);
      XDrawArc(disp,
	       draw,
	       piePtr->textGC,
	       piePtr->borderWidth + piePtr->explodelength +xoff,
	       piePtr->borderWidth + piePtr->explodelength -yoff
	       + piePtr->titleheight,
	       piePtr->radius*2,
	       piePtr->radius*2,
	       exp_start,
	       exp_diff);
      start += exp_diff;
    }else {
      XFillArc(disp,	/* display */
	       draw,	/* drawable */
	       piePtr->elements[i].gc,
	       piePtr->borderWidth + piePtr->explodelength,
	       piePtr->borderWidth + piePtr->explodelength
	       + piePtr->titleheight,
	       (piePtr->radius*2),		/* width */
	       (piePtr->radius*2),		/* height */
	       start,			/* angle1 */
	       foo = -(double)piePtr->elements[i].value * perpt); /* angle2 */
      if (piePtr->element_count > 1)
	XDrawLine(disp,
		  draw,
		  piePtr->textGC,
		  pieoffset(piePtr),
		  pieoffset(piePtr) + piePtr->titleheight,
		  pieoffset(piePtr)+
		  (int)(piePtr->radius * cos(deg64_2_radians(start))),
		  pieoffset(piePtr) -
		  (int)(piePtr->radius * sin(deg64_2_radians(start))) +
		  piePtr->titleheight);
      start += foo;
    }
  }
  XDrawArc(disp,
	   draw,
	   piePtr->textGC,
	   piePtr->borderWidth + piePtr->explodelength,
	   piePtr->borderWidth + piePtr->explodelength
	   + piePtr->titleheight,
	   piePtr->radius*2,
	   piePtr->radius*2,
	   exp_start,
	   M_DEGREE + exp_diff);
  
    
  for (i =0 ; i <piePtr->element_count; i++) {
    int height = fontheight(piePtr->fontPtr);
    char valuestr[30];
    int rheight = (height * piePtr->keysize) / 100;
    int x = piewidth(piePtr);
    
    options = piePtr->keyorder;
    while (*options) {
      x += PADDING*2;
      switch (*options) {
      case 'k': case 'K' : {			/* Key element. */
	XFillRectangle(disp,draw,
		       piePtr->elements[i].gc, 
		       x,
		       y,
		       rheight,rheight);
	XDrawRectangle(disp,draw,
		       piePtr->textGC, 
		       x,
		       y,
		       rheight,rheight);
	x += rheight;
	break;
      }
      case 'p': case 'P': {
	int tlength;
	sprintf(valuestr, "%.2f%%",
		100.0 * piePtr->elements[i].value / piePtr->total_value);
	tlength = strlen(valuestr);
	
	XDrawString(disp,draw,
		    piePtr->textGC, 
		    x,
		    y+rheight - PADDING*2,
		    valuestr,
		    tlength);
	x += piePtr->percentwidth;
	break;
      }
      case 'v': case 'V': {
	int tlength;
	char formatstr[10];

	sprintf(formatstr,"%%.%df",piePtr->numdigits);
	sprintf(valuestr, formatstr ,piePtr->elements[i].value);
	tlength = strlen(valuestr);
	
	XDrawString(disp,draw,
		    piePtr->textGC, 
		    x,
		    y+rheight - PADDING*2, valuestr, tlength);
	x += piePtr->valuewidth;
	break;
      }
      case 'l': case 'L' : {
	if (piePtr->elements[i].label) {	  /* TEMPORARY FIX: label <-> legend */
	  int tlength;
	  
	  tlength = strlen(piePtr->elements[i].label);
	  
	  XDrawString(disp,draw,
		      piePtr->textGC, 
		      x,
		      y+rheight - PADDING*2,
		      piePtr->elements[i].label,
		      tlength);
	}
	x += piePtr->textwidth;
	break;
      }
	default : {} /* ignore spurious values */
      }
      options++;
    }
    y += height;
  }	
}

void EventuallyRedrawPie(piePtr, displaybits)
    register Pie *piePtr;	/* Information about widget. */
    int displaybits;            /* Which parts of the pie are to be drawn */
{

    if ( (piePtr->tkwin == NULL) || !Tk_IsMapped(piePtr->tkwin) ||
	(piePtr->displaybits & REDRAW_PENDING) )
      return;

    piePtr->displaybits = displaybits | REDRAW_PENDING;
    Tk_DoWhenIdle(DisplayPie, (ClientData)piePtr);

}

void RedrawPie(piePtr)
     Pie *piePtr;
{
  EventuallyRedrawPie(piePtr,piePtr->displaybits);
}

void ComputePieGeometry(piePtr)
     Pie *piePtr;  
{
  int width;
  int height,height2;
  int font = fontheight(piePtr->fontPtr);
  char *options;

  height = piewidth(piePtr);
  width = height + PADDING;
  height2 = (font * piePtr->element_count)+
    (piePtr->borderWidth + piePtr->explodelength)*2+PADDING*2;
  if (height < height2)
    height = height2;
  options = piePtr->keyorder;
  while (*options) {
    width += PADDING*2;
    switch (*options) {
    case 'k': case 'K': {width += (font * piePtr->keysize) / 100; break; }
    case 'p': case 'P': {width += piePtr->percentwidth; break; }
    case 'l': case 'L': {width += piePtr->textwidth; break; }
    case 'v': case 'V': {width += piePtr->valuewidth; break; }
      default : {}
    }
    options++;
  }
  
  piePtr->titleheight = (piePtr->title && piePtr->title[0])?font:0;
  height += piePtr->titleheight;
  Tk_GeometryRequest(piePtr->tkwin, width, height);
  Tk_SetInternalBorder(piePtr->tkwin,piePtr->borderWidth);
}

int pieInsert(piePtr, label,value)
     Pie *piePtr;
     char *label;
     double value;
{
  
  /* do that voodoo what you do. */
  XGCValues gcValues;
  int i = piePtr->element_count;
  
  piePtr->total_value += value;

  piePtr->element_count += 1;
  piePtr->elements[i].value = value;
  piePtr->elements[i].label = 
    (char *)malloc(strlen(label)+1);

  strcpy(piePtr->elements[i].label,label);

  piePtr->elements[i].legend = NULL;	/* For the time being. */

  piePtr->displaybits |= KEY_CHANGED;
  
  /* Now a fill pattern. */
  
  if (piePtr->display_depth > 1) {
    XColor *foo;
#if TK_MAJOR_VERSION > 3
    foo = Tk_GetColor(piePtr->interp,
		  piePtr->tkwin,
		  colors[piePtr->element_count - 1]);
#else
    foo = Tk_GetColor(piePtr->interp,
		  piePtr->tkwin,
		  (Colormap)NULL,
		  colors[piePtr->element_count - 1]);
#endif
    if (foo == NULL) 
      return TCL_ERROR;
    gcValues.foreground = foo->pixel;
    piePtr->elements[i].gc = Tk_GetGC(piePtr->tkwin,
			GCForeground,
			&gcValues);
  } else {
    gcValues.fill_style = FillTiled;
    gcValues.tile = 
      Tk_GetBitmapFromData(piePtr->interp,
			   piePtr->tkwin,
			   bitmap[piePtr->element_count-1],
			   8,8);
    piePtr->elements[i].gc = Tk_GetGC(piePtr->tkwin,
			GCTile | GCFillStyle,
			&gcValues);
    
  }
  return TCL_OK;
}

void getAllValues(piePtr)
     Pie *piePtr;
{
  int i;

  for (i = 0; i<piePtr->element_count; i++) {
    char valuestr[40];
    sprintf(valuestr, "%f", piePtr->elements[i].value);
    Tcl_AppendResult(piePtr->interp,
		     piePtr->elements[i].label, 
		     " ", valuestr, " ", (char *)NULL);
  }
}

void getPieElements(piePtr,label)
     Pie *piePtr;
     char *label;
{
  int i;
  for (i = 0; i < piePtr->element_count; i++) 
    if (strcmp(piePtr->elements[i].label, label)==0) {
      char valuestr[20];
      sprintf(valuestr, "%f", piePtr->elements[i].value);
      Tcl_AppendResult(piePtr->interp,
		       piePtr->elements[i].label, " ", 
		       valuestr, " ", (char *)NULL);
      return;
    }      
} 

void alterPieIndex(piePtr,i,value)
     Pie *piePtr;
     int i;
     double value;
{
  piePtr->total_value += (value - piePtr->elements[i].value);
  piePtr->elements[i].value = value;
  piePtr->displaybits |= KEY_CHANGED;
}

int alterPieValue(piePtr,label,value)
     Pie *piePtr;
     char *label;
     double value;
{
  int i;
  for (i = 0; i < piePtr->element_count; i++)
    if (strcmp(piePtr->elements[i].label, label)==0) {
      alterPieIndex(piePtr,i,value);
      return TCL_OK;
    }
  return pieInsert(piePtr,label,value);
}

int alterPieStr(piePtr,label,valuestr)
     Pie *piePtr;
     char *label;
     char *valuestr;
{
  double value;

  
  if (Tcl_GetDouble(piePtr->interp, valuestr, &value) != TCL_OK) {
    return TCL_ERROR;    
  }
  return alterPieValue(piePtr,label,value);
}

void deleteElementIndex(piePtr,index)
     Pie *piePtr;
     int index;
{
  GC gc;
  int i;
  
  if (piePtr->elements[index].legend)
    free(piePtr->elements[index].legend);
  if (piePtr->elements[index].label)
    free(piePtr->elements[index].label);
  piePtr->total_value -= piePtr->elements[index].value;
  for (i = index; i < piePtr->element_count; i++) {
    piePtr->elements[i].label = piePtr->elements[i+1].label;
    piePtr->elements[i].legend = piePtr->elements[i+1].legend;
    piePtr->elements[i].value = piePtr->elements[i+1].value;
  }
  Tk_FreeGC(piePtr->display,piePtr->elements[piePtr->element_count-1].gc);
}

void deleteElement(piePtr, label)
     Pie *piePtr;
     char *label;
{

  int i;
  
  for (i=0; i< piePtr->element_count; i++) 
    if (strcmp(piePtr->elements[i].label,label) == 0) {
      deleteElementIndex(piePtr,i);
      piePtr->element_count--;
      piePtr->displaybits |= KEY_CHANGED;
      if (piePtr->element_count == 0)
	piePtr->displaybits != CLEAR_NEEDED;
      return;
    }
}

void Callback(piePtr)
     Pie *piePtr;
{
  int no_errors = TRUE;
  int result;
  int i, n, w;
  char c;
  double value;
  char *datastr;
  char mmstr[32];
  char machinestr[32];
  char metricstr[32];

  if ( piePtr->command != NULL && piePtr->command[0] != '\0' ) {
    result = Tcl_Eval( piePtr->interp, piePtr->command );
    if ( result != TCL_OK ) {
      /*TkBindError(piePtr->interp);*/
      no_errors = FALSE;
    }
  }

  if ( piePtr->continue_callback && no_errors )
    Tk_CreateTimerHandler( piePtr->interval, (void *)Callback,
			  (ClientData) piePtr );

}

int pieWidgetCmd(clientData, interp,argc,argv)
     ClientData clientData;
     Tcl_Interp *interp;
     int argc; char **argv;
{
  /* What to do when the widget is called, ie .foo configure, etc. */
  register Pie *piePtr = (Pie *) clientData;
  int result = TCL_OK;
  int length;
  char c;


  if (argc < 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"",
		     argv[0], " option ?arg arg ...?\"", (char *) NULL);
    return TCL_ERROR;
  }
  Tk_Preserve((ClientData) piePtr);
  c = argv[1][0];
  length = strlen(argv[1]);
  if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)) {
    if (argc == 2)
      /* return info about all configuration options. */
      result = Tk_ConfigureInfo(interp, piePtr->tkwin, configSpecs,
				(char *) piePtr, (char *) NULL, 0);
    else if (argc == 3) 
      result = Tk_ConfigureInfo(interp, piePtr->tkwin, configSpecs,
				(char *) piePtr, argv[2], 0);
    else
      result = ConfigurePie(interp, piePtr, argc-2, argv+2,
			    TK_CONFIG_ARGV_ONLY);
    piePtr->displaybits |= CLEAR_NEEDED;
  }
  else
    if ((c == 'd') && (strncmp(argv[1], "delete", length) == 0)) {
      if (argc == 2) {
	int i;
	for (i=0; i<piePtr->element_count; i++) {
	  if (piePtr->elements[i].legend)
	    free(piePtr->elements[i].legend);
	  if (piePtr->elements[i].label)
	    free(piePtr->elements[i].label);
	  Tk_FreeGC(piePtr->display,piePtr->elements[i].gc);
	}
	piePtr->total_value = 0;
	piePtr->element_count = 0;
	result = TCL_OK;
      } else
	if (argc > 2) {
	  argv++;
	  while (argc > 2) {
	    argv++;argc--;
	    deleteElement(piePtr,*argv);
	  }
	  result = TCL_OK;
	} else {
	  Tcl_AppendResult(interp, "wrong # args: should be \"",
			   argv[0], " delete ?label ...?\"",(char *)NULL);
	  result =  TCL_ERROR;
	}
    }
    else
      if ((c == 's') && (strncmp(argv[1], "set", length) == 0)) {
	if ((argc >= 4) && (argc%2 == 0)) 
	  while (argc > 2) {
	    argv += 2; argc -=2;
	    result = alterPieStr(piePtr, argv[0],argv[1]);
	    if (result != TCL_OK) argc = 0;
	  }
	else {
	  Tcl_AppendResult(interp, "wrong # args: should be \"",
			   argv[0], " set ?label value? ...\"",(char *)NULL);
	  result =  TCL_ERROR;
	}
      }
      else
	if ((c == 'g') && (strncmp(argv[1], "get", length) == 0)) {
	  if (argc == 2) {
	    result = TCL_OK;
	    getAllValues(piePtr);
	  } else 
	    if (argc>2) {
	      argv++;
	      result = TCL_OK;
	      while (argc > 2) {
		argv++;argc--;
		getPieElements(piePtr,*argv);
	      }
	    } else {
	      Tcl_AppendResult(interp, "wrong # args: should be \"",
			   argv[0], " get ?label ...?\"",(char *)NULL);
	      result =  TCL_ERROR;
	    }
	} else 
	  if ((c == 's') && (strncmp(argv[1], "start", length) == 0)) {
	    if (! piePtr->continue_callback ) {
	      piePtr->continue_callback = TRUE;
	      Callback(piePtr);
	    }
	  } 
	  else 
	    if ((c == 's') && (strncmp(argv[1], "stop", length) == 0)) {
	      piePtr->continue_callback = FALSE;
	    }
	    else
	      {
		/* BASE CASE: Not understood. */
		sprintf(interp->result,
			"bad option \"%.50s\": must be configure, delete, get, set, start or stop",
			argv[1]);
		result = TCL_ERROR;
	      }
  
  if (result == TCL_OK) {
    ComputePieGeometry(piePtr);
    RedrawPie(piePtr);
  }
  Tk_Release((ClientData) piePtr);
  return result;
}

void DestroyPie(clientData)
     ClientData clientData;
{
  register Pie *piePtr = (Pie *) clientData;

  int i;

  if (piePtr->border != NULL)
    Tk_Free3DBorder(piePtr->border);
  
  if (piePtr->fontPtr != NULL)
    Tk_FreeFontStruct(piePtr->fontPtr);
  
  if (piePtr->textColorPtr != NULL)
    Tk_FreeColor(piePtr->textColorPtr);
  
  if (piePtr->backgroundColor != NULL)
    Tk_FreeColor(piePtr->backgroundColor);
  
  if (piePtr->textGC != None)
    Tk_FreeGC(piePtr->display,piePtr->textGC);

  if (piePtr->cursor != None) {
    Tk_FreeCursor(piePtr->display,piePtr->cursor);	/* -jules- */
  }

  /* and so on...  ********************************************/

  for (i = 0; i < piePtr->element_count; i++) {
    if (piePtr->elements[i].legend)
      free(piePtr->elements[i].legend);
    if (piePtr->elements[i].label)
      free(piePtr->elements[i].label);
    Tk_FreeGC(piePtr->display,piePtr->elements[i].gc);
  }

  free((char *) piePtr);
}


void pieEventProc(clientData,eventPtr)
     ClientData clientData;
     XEvent *eventPtr;
{
  Pie *piePtr = (Pie *) clientData;
  
  if ((eventPtr->type == Expose) &&(eventPtr->xexpose.count == 0))
    RedrawPie(piePtr);
  else if (eventPtr->type == DestroyNotify) {
    Tcl_DeleteCommand(piePtr->interp, Tk_PathName(piePtr->tkwin));
    piePtr->tkwin = NULL;
    Tk_CancelIdleCall(DisplayPie, (ClientData) piePtr);
    Tk_EventuallyFree((ClientData) piePtr, DestroyPie);
  } else if (eventPtr->type == ConfigureNotify) {
    ComputePieGeometry(piePtr);
  }
}  



 /* 
  *---------------------------------------------------------------
  *
  *
  * Tk_PieCmd --
  *
  *
  *---------------------------------------------------------------
  */




int Tk_PieCmd(clientData, interp, argc, argv)
     ClientData clientData;
     Tcl_Interp *interp;
     int argc;
     char **argv;
{
  Tk_Window tkwin = (Tk_Window) clientData;
  register Pie *piePtr;
  Tk_Window new;
  int w;
  
  if (argc < 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"",
		     argv[0], " pathName ?options?\"", (char *) NULL);
    return TCL_ERROR;
  }
  
  piePtr = (Pie *)malloc(sizeof(Pie));
  
  if (!piePtr) {
    fprintf( stderr, "Failed to allocate %d bytes for the Pie structure\n",
	    sizeof(Pie) );
    return TCL_ERROR;
    }

  new = Tk_CreateWindowFromPath(interp, tkwin, argv[1], (char *)NULL);
  if (new==NULL)
    return TCL_ERROR;

  piePtr->tkwin = new;
  piePtr->display = Tk_Display(new);

  piePtr->interp = interp;
  piePtr->border = NULL;
  piePtr->relief = TK_RELIEF_RAISED;
  piePtr->displaybits = 0;
  piePtr->cursor = None;	           /* -jules- */
  piePtr->backgroundColor = NULL;
  piePtr->textColorPtr = NULL;
  piePtr->pieColor = NULL;
  piePtr->fontPtr = NULL;
  piePtr->title = NULL;
  piePtr->command = NULL;
  piePtr->exploded = NULL;
  piePtr->total_value = 0;
  piePtr->element_count = 0;
  piePtr->userdata = NULL;
  piePtr->continue_callback = FALSE;
  piePtr->textGC = None;
  
  Tk_MakeWindowExist(piePtr->tkwin);

  piePtr->display_depth = DefaultDepthOfScreen(Tk_Screen(tkwin));

  Tk_SetClass(piePtr->tkwin, PIE_CLASS_NAME);
  Tk_CreateEventHandler(piePtr->tkwin, 
			ExposureMask|StructureNotifyMask,
			pieEventProc, (ClientData)piePtr);
  Tcl_CreateCommand(interp,
		    Tk_PathName(piePtr->tkwin),
		    pieWidgetCmd,
		    (ClientData) piePtr,
		    (void (*)()) NULL);

  if (ConfigurePie(interp, piePtr, argc-2, argv+2,0) == TCL_OK) {
    interp->result = Tk_PathName(piePtr->tkwin);
    return TCL_OK;
  }
  Tk_DestroyWindow(piePtr->tkwin);
  return TCL_ERROR;
}

int ConfigurePie(interp,piePtr,argc,argv,flags)
     Tcl_Interp *interp;
     Pie *piePtr;
     int argc;
     char **argv;
     int flags;
{
  XGCValues gcValues;
  GC newGC;
  
  if (Tk_ConfigureWidget(interp,
			 piePtr->tkwin,
			 configSpecs,
			 argc,
			 argv,
			 (char *) piePtr,
			 flags) != TCL_OK)
    return TCL_ERROR;
  
  Tk_SetBackgroundFromBorder(piePtr->tkwin, piePtr->border);
  gcValues.font = piePtr->fontPtr->fid;
  gcValues.foreground = piePtr->textColorPtr->pixel;
  newGC = Tk_GetGC(piePtr->tkwin, GCForeground|GCFont, &gcValues);
  if (piePtr->textGC != None) 
    Tk_FreeGC(piePtr->display,piePtr->textGC);
  piePtr->textGC = newGC;

  ComputePieGeometry(piePtr);
  RedrawPie(piePtr);
  return TCL_OK;
}

/*
 * Local Variables:
 * comment-column:40
 * End:
 */
