

#define IBRD		0		/* Standard functions. */
#define IBWRT		1
#define IBCMD		2
#define IBWAIT		3
#define IBRPP		4
#define IBONL		5
#define IBSIC		6
#define IBSRE		7
#define IBGTS		8
#define IBCAC		9

#define IBLINES		11
#define IBPAD		12
#define IBSAD		13
#define IBTMO		14
#define IBEOT		15
#define IBEOS		16
#define IBRSV		17

#define DEVFCN		100

#define DVTRG		(DEVFCN+0)	/* Device-level functions...*/
#define DVCLR		(DEVFCN+1)
#define DVRSP		(DEVFCN+2)
#define DVRD		(DEVFCN+3)
#define DVWRT		(DEVFCN+4)

#define CFCIO           200             /* Special Functions.. */

#define CFCBASE         (CFCIO+0)
#define CFCIRQ          (CFCIO+1)
#define CFCDMA          (CFCIO+2)
#define CFCDMABUFFER	(CFCIO+3)

#define IBSDBG          (CFCIO+4)

#define APIO            300
#define IBAPWAIT        (APIO+0)
#define IBAPRSP         (APIO+1)
#define IBAPE           (APIO+2)





