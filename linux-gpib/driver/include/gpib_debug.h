
#if DEBUG

#define DBG_ALL      (1 << 0)
#define DBG_ENTRY   ((1 << 1) | DBG_ALL)
#define DBG_EXIT    ((1 << 2) | DBG_ALL)
#define DBG_BRANCH  ((1 << 3) | DBG_ALL)
#define DBG_DATA    ((1 << 4) | DBG_ALL)
#define DBG_INTR    ((1 << 5) | DBG_ALL)
#define DBG_REG     ((1 << 6) | DBG_ALL)
#define DBG_SPEC    ((1 << 7) | DBG_ALL)
#define DBG_1PPL     (1 << 8)		/* one DBG print statement/line */



extern unsigned int dbgMask;


/* class of debug statements allowed		*/

extern int   fidx;     /*defined in ../sys/osinit.c */
extern char *fstk[];
extern char *ffmt[];

#define DBGprint(ms,ar)	{ if (dbgMask && (dbgMask & ms)) { osPrint("gpib - %s%s:", ffmt[fidx], fstk[fidx]); osPrint ar; osPrint(0); } }
#define DBGin(id)	{ fstk[++fidx] = id; DBGprint(DBG_ENTRY, ("in  ")); }
#define DBGout()	{ DBGprint(DBG_EXIT, ("out  ")); fidx--; }

#define IBLCL				/* IB "local" symbols -- leave global for debugging */

#else	/*DEBUG*/
#define DBGprint(ms,ar)	{ }
#define DBGin(id)	{ }
#define DBGout()	{ }

#define IBLCL  

#endif /*DEBUG*/
	
