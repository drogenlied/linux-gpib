
#ifndef _GPIB_TYPES_H
#define _GPIB_TYPES_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

typedef unsigned char	*faddr_t;	

typedef struct ibio_op {
	faddr_t		io_vbuf;	/* virtual buffer address	*/
	uint32_t		io_pbuf;	/* physical buffer address	*/
	unsigned	io_cnt;		/* transfer count		*/
	int		io_flags;	/* direction flags, etc.	*/
	uint8_t		io_ccfunc;	/* carry-cycle function		*/
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

#endif	// _GPIB_TYPES_H
