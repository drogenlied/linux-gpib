
#define __NO_VERSION__
#include <gpibP.h>

extern ibregs_t *ib;            /* Local pointer to IB registers */
#define   IB		ib		/* interface board base ptr */


extern volatile int       ibsta;
extern volatile int       ibcnt;
extern volatile int       iberr;


extern volatile int pgmstat;
extern volatile int noTimo;

extern int timeidx;
extern int pollTimeidx;
extern int myPAD;
extern int mySAD;
extern int auxrabits;
extern int ifcDelay;


extern uint32 timeTable[];
