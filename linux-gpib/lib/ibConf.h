
/* meaning for flags */     

#define CN_SDCL    (1<<1)             /* Send DCL on init                */
#define CN_SLLO    (1<<2)             /* Send LLO on init                */
#define CN_NETWORK (1<<3)             /* is a network device             */
#define CN_AUTOPOLL (1<<4)            /* Auto serial poll devices        */
#define CN_EXCLUSIVE (1<<5)           /* Exclusive use only */

/*---------------------------------------------------------------------- */

typedef struct ibConfStruct
{
	char name[100];		/* name of the device (for ibfind())     */
	int pad;		// device primary address
	int sad;		// device secodnary address (negative disables)
	char init_string[100];               /* initialization string (optional) */
	int board;                         /* board number                     */
	char eos;                           /* local eos modes                  */
	int eos_flags;
	int flags;                         /* some flags                       */
	unsigned int usec_timeout;
	unsigned int send_eoi : 1;	// assert EOI at end of writes
	unsigned int is_interface : 1;	// is interface board
} ibConf_t;

/*---------------------------------------------------------------------- */

typedef struct ibBoardStruct {
	char board_type[100];	// name (model) of interface board
	int pad;		// device primary address
	int sad;		// device secodnary address (negative disables)
	unsigned long base;                          /* base configuration               */
	unsigned int irq;
	unsigned int dma;
	int fileno;                        /* device file descriptor           */
	char device[100];	// name of device file ( /dev/gpib0, etc.)
	int is_system_controller : 1;	/* board is busmaster or not */
} ibBoard_t;


















