//bit definitions common to nec-7210 compatible registers

#ifndef _NEC7210_H
#define _NEC7210_H

// ISR1: interrupt status register 1
#define HR_DI           (1<<0)
#define HR_DO           (1<<1)
#define HR_ERR          (1<<2)
#define HR_DEC          (1<<3)
#define HR_END          (1<<4)
#define HR_DET          (1<<5)
#define HR_APT          (1<<6)
#define HR_CPT          (1<<7)

// IMR1: interrupt mask register 1
#define IMR1_ENABLE_INTR_MASK	0xff	// all the bits in this register that enable interrupts
#define HR_DIIE         (1<<0)
#define HR_DOIE         (1<<1)
#define HR_ERRIE        (1<<2)
#define HR_DECIE        (1<<3)
#define HR_ENDIE        (1<<4)
#define HR_DETIE        (1<<5)
#define HR_APTIE        (1<<6)
#define HR_CPTIE        (1<<7)

// ISR2, interrupt status register 2
#define HR_ADSC         (1 << 0)
#define HR_REMC         (1 << 1)
#define HR_LOKC         (1 << 2)
#define HR_CO           (1 << 3)
#define HR_REM          (1 << 4)
#define HR_LOK          (1 << 5)
#define HR_SRQI         (1 << 6)
#define HR_INT          (1 << 7)

// IMR2, interrupt mask register 2
#define IMR2_ENABLE_INTR_MASK	0x4f	// all the bits in this register that enable interrupts
#define HR_ACIE         (1<<0)
#define HR_REMIE        (1<<1)
#define HR_LOKIE        (1<<2)
#define HR_COIE         (1<<3)
#define HR_DMAI         (1<<4)
#define HR_DMAO         (1<<5)
#define HR_SRQIE        (1<<6)

// SPSR, serial poll status register
#define HR_PEND         (1<<6)

// SPMR, serial poll mode register
#define HR_RSV          (1<<6)

// ADSR, address status register
#define HR_MJMN         (1<<0)
#define HR_TA           (1<<1)
#define HR_LA           (1<<2)
#define HR_TPAS         (1<<3)
#define HR_LPAS         (1<<4)
#define HR_SPMS         (1<<5)
#define HR_NATN         (1<<6)
#define HR_CIC          (1<<7)

// ADMR, address mode register
#define HR_ADM0         (1<<0)
#define HR_ADM1         (1<<1)
#define HR_TRM0         (1<<4)
#define HR_TRM1         (1<<5)
#define HR_LON          (1<<6)
#define HR_TON          (1<<7)

// ADR, bits used in address0, address1 and address0/1 registers
#define HR_DL           (1<<5)
#define HR_DT           (1<<6)
#define HR_ARS          (1<<7)

// ADR1, address1 register
#define HR_EOI          (1<<7)

// AUXMR, auxiliary mode register
#define ICR		0x20
#define PPR		0x60
#define AUXRA		0x80
#define AUXRB		0xa0
#define AUXRE		0xc0
#define LOMASK		0x1F			/* mask to specify lower 5 bits */

// auxra, auxiliary register A
#define HR_HANDSHAKE_MASK	0x3
#define HR_HLDA         0x1
#define HR_HLDE         0x2
#define HR_LCM          0x3	/* auxra listen continuous */
#define HR_REOS         (1<<2)
#define HR_XEOS         (1<<3)
#define HR_BIN          (1<<4)

// auxrb, auxiliary register B
#define HR_CPTE         (1<<0)
#define HR_SPEOI        (1<<1)
#define HR_TRI          (1<<2)
#define HR_INV          (1<<3)
#define HR_ISS          (1<<4)

// parallel poll register
#define HR_PPS          (1<<3)
#define HR_PPU          (1<<4)

/* 7210 Auxiliary Commands */
#define AUX_PON         0x0	/* Immediate Execute pon                  */
#define AUX_CPPF        0x1	/* Clear Parallel Poll Flag               */
#define AUX_CR          0x2	/* Chip Reset                             */
#define AUX_FH          0x3	/* Finish Handshake                       */
#define AUX_TRIG        0x4	/* Trigger                                */
#define AUX_RTL         0x5	/* Return to local                        */
#define AUX_SEOI        0x6	/* Send EOI                               */
#define AUX_NVAL        0x7	/* Non-Valid Secondary Command or Address */
#define AUX_SPPF        0x9	/* Set Parallel Poll Flag                 */
#define AUX_VAL         0xf	/* Valid Secondary Command or Address     */
#define AUX_GTS         0x10	/* Go To Standby                          */
#define AUX_TCA         0x11	/* Take Control Asynchronously            */
#define AUX_TCS         0x12	/* Take Control Synchronously             */
#define AUX_LTN         0x13	/* Listen                                 */
#define AUX_DSC         0x14	/* Disable System Control                 */
#define AUX_CIFC        0x16	/* Clear IFC                              */
#define AUX_CREN        0x17	/* Clear REN                              */
#define AUX_TCSE        0x1a	/* Take Control Synchronously on End      */
#define AUX_LTNC        0x1b	/* Listen in Continuous Mode              */
#define AUX_LUN         0x1c	/* Local Unlisten                         */
#define AUX_EPP         0x1d	/* Execute Parallel Poll                  */
#define AUX_SIFC        0x1e	/* Set IFC                                */
#define AUX_SREN        0x1f	/* Set REN                                */

#endif	//_NEC7210_H
