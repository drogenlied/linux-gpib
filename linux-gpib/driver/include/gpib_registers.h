
#ifndef _GPIB_REGISTERS_H
#define _GPIB_REGISTERS_H


#if defined(NIPCII) || defined(NIAT)

/*
 *  Register layout for nec-7210 compatible chips
 *
 */

#if defined(MODBUS_PCI)
#define NEC7210_REG_OFFSET 0x2
#endif

#define CR_EOI	(1<<7)	/* gpibc        */
#define CR_ATN	(1<<6)	/*  ,           */
#define CR_SRQ	(1<<5)	/*  ,           */
#define CR_REN	(1<<4)	/*  ,           */
#define CR_IFC	(1<<3)	/*  ,           */
#define CR_NRFD	(1<<2)	/*  ,           */
#define CR_NDAC	(1<<1)	/*  ,           */
#define CR_DAV	(1<<0)	/*  ,           */

#define BR_ATN	(1<<7)	/* bsr and bcr  */
#define BR_DAV	(1<<6)	/*  ,           */
#define BR_NDAC	(1<<5)	/*  ,           */
#define BR_NRFD	(1<<4)	/*  ,           */
#define BR_EOI	(1<<3)	/*  ,           */
#define BR_SRQ	(1<<2)	/*  ,           */
#define BR_IFC	(1<<1)	/*  ,           */
#define BR_REN	(1<<0)	/*  ,           */

#define HR_nba	(1<<7)	/* sasr         */
#define HR_AEHS	(1<<6)	/*  ,           */
#define HR_ANHS1	(1<<5)	/*  ,           */
#define HR_ANHS2	(1<<4)	/*  ,           */
#define HR_ADHS	(1<<3)	/*  ,           */
#define HR_ACRDY	(1<<2)	/*  ,           */
#define HR_SH1A	(1<<1)	/*  ,           */
#define HR_SH1B	(1<<0)	/*  ,           */

#define AUX_PAGE	0x50	/* Page-In additional registers           */

/* CBI-488.2 additional Registers */

/* hidden line status register */

#define LN_ATN          (1<<0)
#define LN_EOI          (1<<1)
#define LN_SRQ          (1<<2)
#define LN_IFC          (1<<3)
#define LN_REN          (1<<4)
#define LN_DAV          (1<<5)
#define LN_NRFD         (1<<6)
#define LN_NDAC         (1<<7)


/** bus timing */

#define T1_2000_NS 0
#define T1_500NS   1
#define T1_350_NS  2

#endif

/*-================================================================================

HP 82335 (memory mapped) or plain (TMS9914 i/o) register definitions

================================================================================*/

#if defined(HP82335) || defined(TMS9914)

/* NOTE: The card interrupt clear register is not correctly
 * documented, it refers to offset 0x37f7
 */


#if defined(HP82335)

#define CCIR 0x37f7
#define CSR	0x37f8
#define BSR	0x3ffb
#define HPIBSR	0x3ffc

/* Register Mnemonics for write access */

#define CCR	CSR          /* card control register     */

#endif

#if defined(TMS9914)

#define BSR	0x3	/* bus-status                  */
#define ADSWR	0x4	/* address-switch/address      */

#if defined(ZIATECH)


#define CTRR 0x2             /* Z1488A control register     */
#define ISTAT 0x5         /* Z1488A IRQ status register  */

#endif

#endif

/* CCR - Register bits  */
#define     HR_DMAEN   (unsigned char) (1<<0)   /* DMA enable                  */
#define     HR_DMACH   (unsigned char) (1<<1)   /* DMA channel select  O=3,1=2 */
#define     HR_INTEN   (unsigned char) (1<<2)   /* interrupt enable            */
#define     HR_SYSDA   (unsigned char) (1<<3)   /* system controller disable   */

/* CSR - Register bits  */
#define     HR_SWT6    (unsigned char) (1<<0)   /* switch 6 position           */
#define     HR_SWT5    (unsigned char) (1<<1)   /* switch 5 position           */
#define     HR_SYSCTL  (unsigned char) (1<<2)   /* system controller bit       */

#define     HR_DMAen   (unsigned char) (1<<4)   /* DMA enabled                 */
#define     HR_DMAch   (unsigned char) (1<<5)   /* DMA channel   0=3,1=2       */
#define     HR_INTen   (unsigned char) (1<<6)   /* Interrupt enable            */
#define     HR_IP      (unsigned char) (1<<7)   /* Interrupt Pending           */

/* HPIBSR - Register bits */

#define     HR_INTSTS  (unsigned char) (1<<0)   /* interupt status bit         */
#define     HR_AOTCTL  (unsigned char) (1<<1)   /* active controller bit       */

#endif /*HPIB*/

#endif	//_GPIB_REGISTERS_H

