
/* meaning for flags */     

#define CN_ISCNTL  (1<<0)             /* Device is the bus controller    */
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
	int eosflags;
	int flags;                         /* some flags                       */
	int tmo;
	unsigned int send_eoi : 1;	// assert EOI at end of writes
	unsigned int is_interface : 1;	// is interface board
} ibConf_t;

/*---------------------------------------------------------------------- */

typedef struct ibBoardStruct {
	char name[100];	// name (model) of interface board
	int pad;		// device primary address
	int sad;		// device secodnary address (negative disables)
	int timeout;                       /* timeout                          */
	char eos;                           /* global eos modes                 */
	int eosflags;
	int base;                          /* base configuration               */
	int irq;
	int dma;
	int fileno;                        /* device file descriptor           */
	char device[100];
	int ifc;                           /* send IFC on init                 */
} ibBoard_t;


















