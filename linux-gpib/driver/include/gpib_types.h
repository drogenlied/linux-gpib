


typedef char		int8;		
typedef unsigned char	uint8;
typedef short		int16;
typedef unsigned short	uint16;
typedef long		int32;
typedef unsigned long	uint32;

typedef unsigned char	*faddr_t;	


typedef union {                         /* FIFO access type             */
    struct {
        uint8   b;                      /*  +0 8-bit FIFO "B" register  */
        uint8   a;                      /*  +1 8-bit FIFO "A" register  */

    }           f8;                     /* two 8-bit FIFO registers A/B */
    uint16      f16;                    /* single 16-bit FIFO register  */

} fifo_t;

typedef struct ibio_op {
	faddr_t		io_vbuf;	/* virtual buffer address	*/
	uint32		io_pbuf;	/* physical buffer address	*/
	unsigned	io_cnt;		/* transfer count		*/
	int		io_flags;	/* direction flags, etc.	*/
	uint8		io_ccfunc;	/* carry-cycle function		*/
} ibio_op_t;


typedef struct {
	int		ib_ioport;	/* I/O address of GPIB board	*/
	int		ib_irqline;	/* Interrupt request line	*/
	int		ib_dmachan;	/* DMA channel			*/
} ibinfo_t;


typedef struct {
	int		ib_opened;	/* GPIB board is opened if > 0	*/
	ibinfo_t      * ib_infoptr;	/* Pointer to ibinfo structure	*/

        int             dmaflag;        /* Switches DMA on/off */

        
         
} ibboard_t;



typedef struct {
	int		ib_cnt;		/* I/O count  (rd, wrt, etc)	*/
	int		ib_arg;		/* other argument value 	*/
	int		ib_ret;		/* general purpose return value	*/
	int		ib_ibsta;	/* returned status vector	*/
	int		ib_iberr;	/* returned error code (if ERR)	*/
	int		ib_ibcnt;	/* returned I/O count		*/

	char            *ib_buf;

} ibarg_t;



