
/*----- Editable Parameters --------------------------------------------*/

#define PAD	 0		/* Primary address of GPIB interface	*/
				/*  Range: 00H - 30H			*/
#define SAD	 0		/* Secondary address of GPIB interface	*/
				/*  Range: 60H - 7EH, or 0 if none	*/
#ifndef IBBASE
#ifdef  NIPCII
#define IBBASE   0x2e1
#else
#define IBBASE	 0x2C0		/* 10-bit I/O address of GPIB interface	*/
#endif
#endif

#ifndef IBIRQ
#define IBIRQ	 11		/* Interrupt request line		*/
#endif

#ifndef IBDMA
#define IBDMA	 5		/* DMA channel				*/
#endif

#define DFLTTIMO T1s		/* Default timeout for I/O operations	*/
#define HZ	 100		/* System clock ticks per second	*/

#ifdef BUILD_DEBUG
#define DEBUG	 1		/* 0 = normal operation, 1 = DEBUG mode */
#else
#define DEBUG	 0		/* 0 = normal operation, 1 = DEBUG mode */
#endif

#if USE_DMA
#define DMAOP    1
#else
#define DMAOP	 0		/* 0 = PIO transfers,    1 = unused	*/
#endif

#if !NO_INTS
#define INTROP	 1		/* 0 = polled operation, 1 = interrupt	*/
#else
#define INTROP	 1		/* 0 = polled operation, 1 = interrupt	*/
#endif

#define SYSTIMO	 1		/* 0 = no system timeouts (use counter)	*/
				/* 1 = system timeouts supported	*/

/*----- END Editable Parameters ----------------------------------------*/

#define IBMAXIO  0xFFFE			/* Maximum cont. I/O transfer	*/





