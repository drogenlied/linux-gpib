#include <gpibP.h>

extern uint16      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */

#define CBI_ISA_GPIB         0
#define CBI_ISA_GPIB_LC      1
#define CBI_PCM_GPIB         2
#define CBI_PCI_GPIB         3


extern uint8       board_type;


extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

extern ibregs_t *ib;            /* Local pointer to IB registers */
#define IB ib              

extern uint8       CurHSMode;


#define FIFO_SIZE 1024

#if 0
#if DMAOP

#error board does not support DMA

#endif
#endif






