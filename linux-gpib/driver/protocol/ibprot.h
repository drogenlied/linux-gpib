
#define __NO_VERSION__
#include <gpibP.h>

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

extern uint32_t timeTable[];
