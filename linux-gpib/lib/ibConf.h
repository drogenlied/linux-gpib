
/* meaning for flags */     

#define CN_ISCNTL  (1<<0)             /* Device is the bus controller    */
#define CN_SDCL    (1<<1)             /* Send DCL on init                */        
#define CN_SLLO    (1<<2)             /* Send LLO on init                */
#define CN_NETWORK (1<<3)             /* is a network device             */
#define CN_AUTOPOLL (1<<4)            /* Auto serial poll devices        */
#define CN_EXCLUSIVE (1<<5)           /* Exclusive use only */

/*---------------------------------------------------------------------- */

typedef struct ibConfStruct {
  char name[31];                      /* name of the device               */
  int  padsad;                        /* device address                   */

  char init_string[61];               /* initialization string (optional) */
#if 0
  int  dcl;                           /* send clear DCL on init           */
#endif
  int  board;                         /* board number                     */

  char eos;                           /* local eos modes                  */
  int  eosflags;

  int  flags;                         /* some flags                       */

  char *networkdb;                    /* network access database  string  */

  int tmo;

} ibConf_t;

/*---------------------------------------------------------------------- */

typedef struct ibBoardStruct {

  int  padsad;                        /* controller's device address      */
  int  timeout;                       /* timeout                          */

  char eos;                           /* global eos modes                 */
  int  eosflags;

  int  base;                          /* base configuration               */
  int  irq;
  int  dma;

  int  fileno;                        /* device file descriptor           */
  char device[61];  
  char errlog [61];

  int  ifc;                           /* send IFC on init                 */
  int  debug;                         /* debugging mask                   */

                     
  int  dmabuf;                        /* size of DMA buffer               */

} ibBoard_t;

 
















