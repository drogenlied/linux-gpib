
#include <gpibP.h>

extern uint16      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */


extern ibregs_t *ib;            /* Local pointer to IB registers */


extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

#define IB ib              

/*#define MARKS_BUGFIX*/




