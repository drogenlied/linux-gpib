/***************************************************************************
                                   tms9914.h
                             -------------------
    begin                : Feb 2002
    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TMS9914_H
#define _TMS9914_H

#include <linux/types.h>
#include <gpib_types.h>

/* struct used to provide variables local to a tms9914 chip */
typedef struct tms9914_private_struct tms9914_private_t;
struct tms9914_private_struct
{
	unsigned long iobase;
	unsigned int offset;	// offset between successive tms9914 io addresses
	unsigned int dma_channel;
	// software copy of bits written to interrupt mask registers
	volatile uint8_t imr0_bits, imr1_bits;
	// bits written to address mode register
	volatile uint8_t admr_bits;
	volatile uint8_t auxa_bits;	// bits written to auxilliary register A
	// used to keep track of board's state, bit definitions given below
	volatile int state;
	// wrappers for outb, inb, readb, or writeb
	uint8_t (*read_byte)(tms9914_private_t *priv, unsigned int register_number);
	void (*write_byte)(tms9914_private_t *priv, uint8_t byte, unsigned int
 register_number);
};

// slightly shorter way to access read_byte and write_byte
extern inline uint8_t read_byte(tms9914_private_t *priv, unsigned int register_number)
{
	return priv->read_byte(priv, register_number);
}
extern inline void write_byte(tms9914_private_t *priv, uint8_t byte, unsigned int register_number)
{
	priv->write_byte(priv, byte, register_number);
}

// tms9914_private_t.state bit numbers
enum
{
	PIO_IN_PROGRESS_BN,	// pio transfer in progress
	DMA_IN_PROGRESS_BN,	// dma transfer in progress
	READ_READY_BN,	// board has data byte available to read
	WRITE_READY_BN,	// board is ready to send a data byte
	COMMAND_READY_BN,	// board is ready to send a command byte
	RECEIVED_END_BN,	// received END
};

// interface functions
extern ssize_t tms9914_read(gpib_board_t *board, tms9914_private_t *priv,
	uint8_t *buffer, size_t length, int *end);
extern ssize_t tms9914_write(gpib_board_t *board, tms9914_private_t *priv,
	uint8_t *buffer, size_t length, int send_eoi);
extern ssize_t tms9914_command(gpib_board_t *board, tms9914_private_t *priv,
	uint8_t *buffer, size_t length);
extern int tms9914_take_control(gpib_board_t *board, tms9914_private_t *priv,
	int syncronous);
extern int tms9914_go_to_standby(gpib_board_t *board, tms9914_private_t *priv);
extern void tms9914_interface_clear(gpib_board_t *board, tms9914_private_t *priv, int assert);
extern void tms9914_remote_enable(gpib_board_t *board, tms9914_private_t *priv, int enable);
extern void tms9914_enable_eos(gpib_board_t *board, tms9914_private_t *priv,
	uint8_t eos_bytes, int compare_8_bits);
extern void tms9914_disable_eos(gpib_board_t *board, tms9914_private_t *priv);
extern unsigned int tms9914_update_status(gpib_board_t *board, tms9914_private_t *priv);
extern void tms9914_primary_address(gpib_board_t *board,
	tms9914_private_t *priv, unsigned int address);
extern void tms9914_secondary_address(gpib_board_t *board, tms9914_private_t *priv,
	unsigned int address, int enable);
extern int tms9914_parallel_poll(gpib_board_t *board, tms9914_private_t *priv, uint8_t *result);
extern void tms9914_parallel_poll_response( gpib_board_t *board,
	tms9914_private_t *priv, uint8_t config );
extern void tms9914_serial_poll_response(gpib_board_t *board, tms9914_private_t *priv, uint8_t status);
extern uint8_t tms9914_serial_poll_status( gpib_board_t *board, tms9914_private_t *priv );

// utility functions
extern void tms9914_board_reset(tms9914_private_t *priv);

// wrappers for io functions
extern uint8_t tms9914_ioport_read_byte(tms9914_private_t *priv, unsigned int register_num);
extern void tms9914_ioport_write_byte(tms9914_private_t *priv, uint8_t data, unsigned int register_num);
extern uint8_t tms9914_iomem_read_byte(tms9914_private_t *priv, unsigned int register_num);
extern void tms9914_iomem_write_byte(tms9914_private_t *priv, uint8_t data, unsigned int register_num);

// interrupt service routine
void tms9914_interrupt(gpib_board_t *board, tms9914_private_t *priv);

// tms9914 has 8 registers
static const int tms9914_num_registers = 8;

/* tms9914 register numbers (might need to be multiplied by
 * a board-dependent offset to get actually io address offset)
 */
// write registers
enum
{
	IMR0 = 0,	/* interrupt mask 0          */
	IMR1 = 1,	/* interrupt mask 1          */
	AUXCR = 3,	/* auxiliary command         */
	ADR = 4,	// address register
	SPMR = 5,	// serial poll mode register
	PPR = 6,	/* parallel poll             */
	CDOR = 7,	/* data out register         */
};
// read registers
enum
{
	ISR0 = 0,	/* interrupt status 0          */
	ISR1 = 1,	/* interrupt status 1          */
	ADSR = 2,	/* address status               */
	BSR = 3,	/* bus status */
	CPTR = 6,	/* command pass thru           */
	DIR = 7,	/* data in register            */
};

//bit definitions common to tms9914 compatible registers

/* ISR0   - Register bits */
#define     HR_MAC     (unsigned char) (1<<0)   /* My Address Change           */
#define     HR_RLC     (unsigned char) (1<<1)   /* Remote/Local change         */
#define     HR_SPAS    (unsigned char) (1<<2)   /* Serial Poll active State    */
#define     HR_END     (unsigned char) (1<<3)   /* END (EOI or EOS)            */
#define     HR_BO      (unsigned char) (1<<4)   /* Byte Out                    */
#define     HR_BI      (unsigned char) (1<<5)   /* Byte In                     */

/* IMR0   - Register bits */
#define     HR_MACIE   (unsigned char) (1<<0)   /*        */
#define     HR_RLCIE   (unsigned char) (1<<1)   /*        */
#define     HR_SPASIE  (unsigned char) (1<<2)   /*        */
#define     HR_ENDIE   (unsigned char) (1<<3)   /*        */
#define     HR_BOIE    (unsigned char) (1<<4)   /*        */
#define     HR_BIIE    (unsigned char) (1<<5)   /*        */

/* ISR1   - Register bits */
#define     HR_IFC     (unsigned char) (1<<0)   /* IFC asserted                */
#define     HR_SRQ     (unsigned char) (1<<1)   /* SRQ asserted                */
#define     HR_MA      (unsigned char) (1<<2)   /* My Adress                   */
#define     HR_DCAS    (unsigned char) (1<<3)   /* Device Clear active State   */
#define     HR_APT     (unsigned char) (1<<4)   /* Adress pass Through         */
#define     HR_UNC     (unsigned char) (1<<5)   /* Unrecognized Command        */
#define     HR_ERR     (unsigned char) (1<<6)   /* Data Transmission Error     */
#define     HR_GET     (unsigned char) (1<<7)   /* Group execute Trigger       */

/* IMR1   - Register bits */
#define     HR_IFCIE   (unsigned char) (1<<0)   /*        */
#define     HR_SRQIE   (unsigned char) (1<<1)   /*        */
#define     HR_MAIE    (unsigned char) (1<<2)   /*        */
#define     HR_DCASIE  (unsigned char) (1<<3)   /*        */
#define     HR_APTIE   (unsigned char) (1<<4)   /*        */
#define     HR_UNCIE   (unsigned char) (1<<5)   /*        */
#define     HR_ERRIE   (unsigned char) (1<<6)   /*        */
#define     HR_GETIE   (unsigned char) (1<<7)   /*        */

/* ADSR   - Register bits */
#define     HR_ULPA    (unsigned char) (1<<0)   /* Store last address LSB      */
#define     HR_TA      (unsigned char) (1<<1)   /* Talker Adressed             */
#define     HR_LA      (unsigned char) (1<<2)   /* Listener adressed           */
#define     HR_TPAS    (unsigned char) (1<<3)   /* talker primary adress state */
#define     HR_LPAS    (unsigned char) (1<<4)   /* listener    "               */
#define     HR_ATN     (unsigned char) (1<<5)   /* ATN active                  */
#define     HR_LLO     (unsigned char) (1<<6)   /* LLO active                  */
#define     HR_REM     (unsigned char) (1<<7)   /* REM active                  */

/* ADR   - Register bits */
#define	ADDRESS_MASK		0x1f			/* mask to specify lower 5 bits for ADR */
#define     HR_DAT     (unsigned char) (1<<5)   /*        */
#define     HR_DAL     (unsigned char) (1<<6)   /*        */
#define     HR_EDPA    (unsigned char) (1<<7)   /*        */

/*---------------------------------------------------------*/
/* TMS 9914 Auxiliary Commands                             */
/*---------------------------------------------------------*/

#define AUX_CR	0x0     /* d Chip reset                   */
#define AUX_DHDF 0x1	// release dac holdoff (nonvalid)
#define AUX_VAL	(AUX_DHDF | AUX_CS)	// release dac holdoff, valid
#define AUX_RHDF	0x2     /* X Release RFD holdoff          */
#define AUX_HLDA	0x3     /* d holdoff on all data          */
#define AUX_HLDE	0x4     /* d holdoff on EOI only          */
#define AUX_NBAF	0x5     /* X Set ne byte availiable false */
#define AUX_FGET	0x6     /* d force GET                    */
#define AUX_RTL	0x7     /* d return to local              */
#define AUX_SEOI	0x8     /* X send EOI with next byte      */
#define AUX_LON	0x9     /* d Listen only                  */
#define AUX_TON	0xa    /* d Talk only                    */
#define AUX_GTS	0xb    /* X goto standby                 */
#define AUX_TCA	0xc    /* X take control asynchronously  */
#define AUX_TCS	0xd    /* X take    "     synchronously  */
#define AUX_RPP	0xe    /* d Request parallel poll        */
#define AUX_SIC	0xf    /* d send interface clear         */
#define AUX_SRE	0x10    /* d send remote enable           */
#define AUX_RQC	0x11    /* X request control              */
#define AUX_RLC	0x12    /* X release control              */
#define AUX_DAI	0x13    /* d disable all interrupts       */
#define AUX_PTS	0x14    /* X pass through next secondary  */
#define AUX_STDL	0x15    /* d short T1 delay                 */
#define AUX_SHDW	0x16    /* d shadow handshake             */
#define AUX_VSTDL	0x17    /* d very short T1 delay              */
#define AUX_RSV2	0x18	// d request service bit 2

#define AUX_CS	0x80 /* set bit instead of clearing it     */

#endif	//_TMS9914_H
