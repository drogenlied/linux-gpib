
/* IBSTA status bits (returned by all functions) */

#define ERR_NUM	15
#define ERR	 (1 << ERR_NUM)	/* Function call terminated on error */
#define TIMO_NUM	14
#define TIMO	 (1 << TIMO_NUM)	/* Time limit on I/O or wait function exceeded */
#define END_NUM	13
#define END	 (1 << END_NUM)	/* EOI or EOS encountered */
#define SRQI_NUM	12
#define SRQI	 (1 << SRQI_NUM)	/* SRQ is asserted */
#define RQS_NUM	11
#define RQS      (1 << RQS_NUM)      /* Device requesting Service */
#define CMPL_NUM	8
#define CMPL	 (1 <<  CMPL_NUM)	/* I/O is complete  */
#define LOK_NUM	7
#define LOK	(1 << LOK_NUM)	// lockout state
#define REM_NUM	6
#define REM	(1 << REM_NUM)	// remote state
#define CIC_NUM	5
#define CIC	 (1 <<  CIC_NUM)	/* GPIB interface is Controller-in-Charge */
#define ATN_NUM	4
#define ATN	 (1 <<  ATN_NUM)	/* Attention is asserted */
#define TACS_NUM	3
#define TACS	 (1 <<  TACS_NUM)	/* GPIB interface is addressed as Talker */
#define LACS_NUM	2
#define LACS	 (1 <<  LACS_NUM)	/* GPIB interface is addressed as Listener */
#define DTAS_NUM 1
#define DTAS (1 << DTAS_NUM)	// device trigger state
#define DCAS_NUM 0
#define DCAS	(1 << DCAS_NUM)	// device clear state

#define WAITBITS (TIMO | SRQI | CIC | TACS | LACS)
// status bits that drivers are responsible for
static const int DRIVERBITS = (SRQI | LOK | REM | CIC | ATN | TACS | LACS | DTAS | DCAS);

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

enum gpib_timeout
{
	TNONE,		/* Infinite timeout (disabled)     */
	T10us,		/* Timeout of 10 usec (ideal)      */
	T30us,		/* Timeout of 30 usec (ideal)      */
	T100us,		/* Timeout of 100 usec (ideal)     */
	T300us,		/* Timeout of 300 usec (ideal)     */
	T1ms,		/* Timeout of 1 msec (ideal)       */
	T3ms,		/* Timeout of 3 msec (ideal)       */
	T10ms,		/* Timeout of 10 msec (ideal)      */
	T30ms,		/* Timeout of 30 msec (ideal)      */
	T100ms,		/* Timeout of 100 msec (ideal)     */
	T300ms,		/* Timeout of 300 msec (ideal)     */
	T1s,		/* Timeout of 1 sec (ideal)        */
	T3s,		/* Timeout of 3 sec (ideal)        */
	T10s,		/* Timeout of 10 sec (ideal)       */
	T30s,		/* Timeout of 30 sec (ideal)       */
	T100s,		/* Timeout of 100 sec (ideal)      */
	T300s,		/* Timeout of 300 sec (ideal)      */
	T1000s,		/* Timeout of 1000 sec (maximum)   */
};

/* End-of-string (EOS) modes for use with ibeos */

#define REOS     0x04		/* Terminate reads on EOS	*/
#define BIN      0x10		/* Do 8-bit compare on EOS	*/

#define EOSM	(REOS | BIN)


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

#define GTL	0x1	/* go to local			*/
#define SDC	0x4	/* selected device clear 	*/
#define PPC	0x5	/* parallel poll configure	*/
#define GET	0x8	/* group execute trigger 	*/
#define TCT	0x9	/* take control 		*/
#define LLO	0x11	/* local lockout		*/
#define DCL	0x14	/* device clear 		*/
#define PPU	0x15	/* parallel poll unconfigure 	*/
#define SPE	0x18	/* serial poll enable 		*/
#define SPD	0x19	/* serial poll disable 		*/
#define UNL	0x3F	/* unlisten 			*/
#define UNT	0x5F	/* untalk 			*/
#define PPE	0x60	/* parallel poll enable (base)	*/
#define S	0x08	/* parallel poll sense bit	*/

#define TRUE     1
#define FALSE    0

#define LAD 0x20 /* value to be 'ored' in to obtain listen address */
#define TAD 0x40 /* value to be 'ored' in to obtain talk address   */

extern inline uint8_t MLA( unsigned int addr )
{
	return addr | 0x20;
};

extern inline uint8_t MTA( unsigned int addr )
{
	return addr | 0x40;
};

extern inline uint8_t MSA( unsigned int addr )
{
	return addr | 0x60;
};

