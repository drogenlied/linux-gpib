
/* IBSTA status bits (returned by all functions) */

#define ERR	 (1 << 15)	/* Function call terminated on error */
#define TIMO	 (1 << 14)	/* Time limit on I/O or wait function exceeded */
#define END	 (1 << 13)	/* EOI terminated the ibrd call */
#define SRQI	 (1 << 12)	/* SRQ is asserted */
#define RQS      (1 << 11)      /* Device requesting Service */
#define CMPL	 (1 <<  8)	/* I/O is complete (should always be set) */
#define CIC	 (1 <<  5)	/* GPIB interface is Controller-in-Charge */
#define ATN	 (1 <<  4)	/* Attention is asserted */
#define TACS	 (1 <<  3)	/* GPIB interface is addressed as Talker */
#define LACS	 (1 <<  2)	/* GPIB interface is addressed as Listener */

#define WAITBITS (TIMO | SRQI | CIC | TACS | LACS)


/* IBERR error codes */

#define EDVR     0		/* system error */
#define ECIC     1		/* not CIC */
#define ENOL     2		/* no listeners */
#define EADR     3		/* CIC and not addressed before I/O */
#define EARG     4		/* bad argument to function call */
#define ESAC     5		/* not SAC */
#define EABO     6		/* I/O operation was aborted */
#define ENEB     7		/* non-existent board (GPIB interface offline) */
#define EDMA     8		/* DMA hardware error detected */
#define EBTO     9		/* DMA hardware uP bus timeout */
#define EOIP    10		/* new I/O attempted with old I/O in progress  */
#define ECAP    11		/* no capability for intended opeation */
#define EFSO    12		/* file system operation error */
#define EOWN    13		/* shareable board exclusively owned */
#define EBUS    14		/* bus error */
#define ESTB    15		/* lost serial poll bytes */
#define ESRQ    16		/* SRQ stuck on */

#define ECFG    17              /* Config file not found */
#define EPAR    18              /* Parse Error in Config */
#define ETAB    19              /* Table Overflow */
#define ENSD    20              /* Device not found */
#define ENWE    21              /* Network Error */
#define ENTF    22              /* Nethandle-table full */
#define EMEM    23              /* Memory allocation Error */

/* Timeout values and meanings */

#define TNONE    0		/* Infinite timeout (disabled)     */
#define T10us    1		/* Timeout of 10 usec (ideal)      */
#define T30us    2		/* Timeout of 30 usec (ideal)      */
#define T100us   3		/* Timeout of 100 usec (ideal)     */
#define T300us   4		/* Timeout of 300 usec (ideal)     */
#define T1ms     5		/* Timeout of 1 msec (ideal)       */
#define T3ms     6		/* Timeout of 3 msec (ideal)       */
#define T10ms    7		/* Timeout of 10 msec (ideal)      */
#define T30ms    8		/* Timeout of 30 msec (ideal)      */
#define T100ms   9		/* Timeout of 100 msec (ideal)     */
#define T300ms  10		/* Timeout of 300 msec (ideal)     */
#define T1s     11		/* Timeout of 1 sec (ideal)        */
#define T3s     12		/* Timeout of 3 sec (ideal)        */
#define T10s    13		/* Timeout of 10 sec (ideal)       */
#define T30s    14		/* Timeout of 30 sec (ideal)       */
#define T100s   15		/* Timeout of 100 sec (ideal)      */
#define T300s   16		/* Timeout of 300 sec (ideal)      */
#define T1000s  17		/* Timeout of 1000 sec (maximum)   */


/* End-of-string (EOS) modes for use with ibeos */

#define REOS     0x04		/* Terminate reads on EOS	*/
#define XEOS     0x08		/* Set EOI with EOS on writes	*/
#define BIN      0x10		/* Do 8-bit compare on EOS	*/

#define EOSM	(REOS | XEOS | BIN)


/* GPIB Bus Control Lines bit vector */

#define BUS_DAV	 0x0100		/* DAV  line status bit */
#define BUS_NDAC 0x0200		/* NDAC line status bit */
#define BUS_NRFD 0x0400		/* NRFD line status bit */
#define BUS_IFC	 0x0800		/* IFC  line status bit */
#define BUS_REN	 0x1000		/* REN  line status bit */
#define BUS_SRQ	 0x2000		/* SRQ  line status bit */
#define BUS_ATN	 0x4000		/* ATN  line status bit */
#define BUS_EOI	 0x8000		/* EOI  line status bit */


/* Possible GPIB command messages */

#define GTL	(unsigned char)0x1	/* go to local			*/
#define SDC	(unsigned char)0x4	/* selected device clear 	*/
#define PPC	(unsigned char)0x5	/* parallel poll configure	*/
#define GET	(unsigned char)0x8	/* group execute trigger 	*/
#define TCT	(unsigned char)0x9	/* take control 		*/
#define LLO	(unsigned char)0x11	/* local lockout		*/
#define DCL	(unsigned char)0x14	/* device clear 		*/
#define PPU	(unsigned char)0x15	/* parallel poll unconfigure 	*/
#define SPE	(unsigned char)0x18	/* serial poll enable 		*/
#define SPD	(unsigned char)0x19	/* serial poll disable 		*/
#define UNL	(unsigned char)0x3F	/* unlisten 			*/
#define UNT	(unsigned char)0x5F	/* untalk 			*/
#define PPE	(unsigned char)0x60	/* parallel poll enable (base)	*/
#define S	(unsigned char)0x08	/* parallel poll sense bit	*/

#define TRUE     1
#define FALSE    0

#define LAD (unsigned char)0x20 /* value to be 'ored' in to obtain listen address */
#define TAD (unsigned char)0x40 /* value to be 'ored' in to obtain talk address   */


extern int  ibsta;			/* status bits returned by last call      */
extern int  ibcnt;			/* actual byte count of last I/O transfer */
extern int  iberr;			/* error code returned when (ibsta & ERR) */


