
#ifndef _GPIB_REGISTERS_H
#define _GPIB_REGISTERS_H

#define GPIBpgin(x)	(GPIBout(AUXMR, AUX_PAGE), GPIBin(x))
#define GPIBpgout(x,v)	(GPIBout(AUXMR, AUX_PAGE), GPIBout(x,v))


/*
 * I/O operation parameters
 */
#define	IO_READ		(1 << 0)	/* 1: I/O read; 0: I/O write	*/
#define	IO_NOHLDA	(1 << 1)	/* don't do HLDA carry-cycle	*/
#define	IO_NOEOI	(1 << 2)	/* don't send EOI w/last byte	*/
#define	IO_LAST		(1 << 3)	/* last "chunk" in I/O transfer	*/

#if defined(NIPCII) || defined(NIAT)

/*
 *  Register layout for nec-7210 compatible chips
 *
 */

#if defined(MODBUS_PCI)
#define NEC7210_REG_OFFSET 0x2
#endif

/* NAT4882 "Paged In" read- and write-only registers (must use GPIBpg__ macros) */

#define CSR		SPMR		/* Chip Signature Register */
#define KEYR		SPMR		/* Key Register */
#define SASR		AUXMR		/* Source/Acceptor Status Register */
#define ISR0		ADR		/* Interrupt Status Register 0 */
#define IMR0		ADR		/* Interrupt Mask Register 0 */
#define BSR		EOSR		/* Bus Status Register */
#define BCR		EOSR		/* Bus Control Register */

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



/*============================================================*/

/* TURBO-488 registers bit definitions */

/* STS1 -- Status Register 1 (read only) */
#define S_DONE          (0x80)	/* DMA done                           */
#define S_SC            (0x40)	/* is system contoller                */
#define S_IN            (0x20)	/* DMA in (to memory)                 */
#define S_DRQ           (0x10)	/* DRQ line (for diagnostics)         */
#define S_STOP          (0x08)	/* DMA stopped                        */
#define S_NDAV          (0x04)	/* inverse of DAV                     */
#define S_HALT          (0x02)	/* status of transfer machine         */
#define S_GSYNC         (0x01)	/* indicates if GPIB is in sync w I/O */

/* CFG -- Configuration Register (write only) */
#define	C_CMD	        (1<<7)	/* FIFO 'bcmd' in progress            */
#define	C_TLCH	     (1<<6)	/* halt DMA on TLC interrupt          */
#define	C_IN	        (1<<5)	/* DMA is a GPIB read                 */
#define	C_A_B	        (1<<4)	/* fifo order 1=motorola, 0=intel     */
#define	C_CCEN	     (1<<3)	/* enable carry cycle                 */
#define	C_TMOE	     (1<<2)	/* enable CPU bus time limit          */
#define	C_T_B	        (1<<1)  	/* tmot reg is: 1=125ns clocks,       */
						/* 0=num bytes                        */
#define	C_B16	        (1<<0)  	/* 1=FIFO is 16-bit register, 0=8-bit */

/* ISR3 -- Interrupt Status Register (read only) */
#define	HR_INTR	        (1<<7)	/* 1=board is interrupting	*/
#define	HR_SRQI_CIC      (1<<5)	/* SRQ asserted and we are CIC	*/
#define	HR_STOP          (1<<4)	/* fifo empty or STOP command	*/
						/* issued			*/
#define	HR_NFF	        (1<<3)	/* NOT full fifo		*/
#define	HR_NEF	        (1<<2)	/* NOT empty fifo		*/
#define	HR_TLCI	        (1<<1)	/* TLC interrupt asserted	*/
#define	HR_DONE          (1<<0)	/* DMA done			*/

/* CMDR -- Command Register */
#define	GO		(1<<2)	/* start DMA 			*/
#define	STOP		(1<<3)	/* stop DMA 			*/
#define	RSTFIFO		(1<<4)	/* reset the FIFO 		*/
#define SFTRST		0x22	/* issue a software reset 	*/

/* STS2 -- Status Register 2 */
#define AFFN		(1<<3)	/* "A full FIFO NOT"  (0=FIFO full)  */
#define AEFN		(1<<2)	/* "A empty FIFO NOT" (0=FIFO empty) */
#define BFFN		(1<<1)	/* "B full FIFO NOT"  (0=FIFO full)  */
#define BEFN		(1<<0)	/* "B empty FIFO NOT" (0=FIFO empty) */


/* Miscellaneous Register Definitions */

#define HR_INTEN	(1<<0)	/* INTRT */
#define HR_DMAEN	(1<<0)	/* DMA   */

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
#define ISR0	0x3ff8
#define ISR1	0x3ff9
#define ADSR	0x3ffa
#define BSR	0x3ffb
#define HPIBSR	0x3ffc
#define CPTR	0x3ffe
#define DIR	0x3fff

/* Register Mnemonics for write access */

#define CCR	CSR          /* card control register     */
#define IMR0	ISR0         /* interrupt mask 0          */
#define IMR1	ISR1         /* interrupt mask 1          */
#define AUXCR	BSR          /* auxiliary command         */
#define ADR	HPIBSR       /* adress register           */
#define SPMR	0x3ffd      /* serial poll mode          */
#define PPR	CPTR         /* parallel poll             */
#define CDOR	DIR          /* data out register         */

#endif

#if defined(TMS9914)

#define ISR0	0x0	/* interrupt status 0          */
#define ISR1	0x1	/* interrupt status 1          */
#define ADSR	0x2	/* adress status               */
#define BSR	0x3	/* bus-status                  */
#define ADSWR	0x4	/* address-switch/address      */
#define SPMR	0x5	/* serial poll mode register */
#define CPTR	0x6	/* command pass thru           */
#define DIR	0x7	/* data in register            */


#if defined(ZIATECH)


#define CTRR 0x2             /* Z1488A control register     */
#define ISTAT 0x5         /* Z1488A IRQ status register  */

#endif

/* Register Mnemonics for write access */

#define IMR0	0x0         /* interrupt mask 0          */
#define IMR1	0x1         /* interrupt mask 1          */
#define AUXCR	0x3         /* auxiliary command         */
#define ADR	0x4        /* adress register           */
#define PPR	0x6         /* parallel poll             */
#define CDOR	0x7          /* data out register         */

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

#define     HR_CIC     0   /*ugly trick to get CIC quiet*/

/* ADR   - Register bits */
#define     HR_DAT     (unsigned char) (1<<5)   /*        */
#define     HR_DAL     (unsigned char) (1<<6)   /*        */
#define     HR_EDPA    (unsigned char) (1<<7)   /*        */

#define LOMASK		0x1F			/* mask to specify lower 5 bits for ADR */

/*---------------------------------------------------------*/
/* TMS 9914 Auxiliary Commands                             */
/*---------------------------------------------------------*/

#define     AUX_CR       (unsigned char) 0     /* d Chip reset                   */
#define     AUX_DACR     (unsigned char) 1     /* d Release ACDS holdoff         */
#define     AUX_RHDF     (unsigned char) 2     /* X Release RFD holdoff          */
#define     AUX_HLDA     (unsigned char) 3     /* d holdoff on all data          */
#define     AUX_HLDE     (unsigned char) 4     /* d holdoff on EOI only          */
#define     AUX_NBAF     (unsigned char) 5     /* X Set ne byte availiable false */
#define     AUX_FGET     (unsigned char) 6     /* d force GET                    */
#define     AUX_RTL      (unsigned char) 7     /* d return to local              */
#define     AUX_SEOI     (unsigned char) 8     /* X send EOI with next byte      */
#define     AUX_LON      (unsigned char) 9     /* d Listen only                  */
#define     AUX_TON      (unsigned char) 10    /* d Talk only                    */
#define     AUX_GTS      (unsigned char) 11    /* X goto standby                 */
#define     AUX_TCA      (unsigned char) 12    /* X take control asynchronously  */
#define     AUX_TCS      (unsigned char) 13    /* X take    "     synchronously  */
#define     AUX_RPP      (unsigned char) 14    /* d Request parallel poll        */
#define     AUX_SIC      (unsigned char) 15    /* d send interface clear         */
#define     AUX_SRE      (unsigned char) 16    /* d send remote enable           */
#define     AUX_RQC      (unsigned char) 17    /* d request control              */
#define     AUX_RLC      (unsigned char) 18    /* d release control              */
#define     AUX_DAI      (unsigned char) 19    /* d disable all interrupts       */
#define     AUX_PTS      (unsigned char) 20    /* X pass through next secondary  */
#define     AUX_STDL     (unsigned char) 21    /* d set T1 delay                 */
#define     AUX_SHDW     (unsigned char) 22    /* d shadow handshake             */

#define     AUX_CS       (unsigned char)(1<<7) /* C/S bit                      */

/** additional 'AUX' commands combined with the C/S bit **/

#define     AUX_SIFC      (AUX_SIC | AUX_CS)
#define     AUX_CIFC      (AUX_SIC )
#define     AUX_SREN      (AUX_SRE | AUX_CS)
#define     AUX_CREN      (AUX_SRE )
#define     AUX_FH        (AUX_RHDF)




#endif /*HPIB*/

/* PROGRAM STATUS codes (see int pgmstat) */

#define PS_ONLINE	(1 << 0)		/* GPIB interface is online */
#define PS_SYSRDY	(1 << 1)		/* System support functions are initialized */
#define PS_TIMINST	(1 << 2)		/* Watch dog timer is installed and running */
#define PS_HELD		(1 << 3)		/* Handshake holdoff in effect */
#define PS_NOINTS	(1 << 4)		/* Do NOT use interrupts for I/O and waits */
#define PS_NOEOI	(1 << 5)		/* Do NOT send EOI at the end of writes */
#define PS_SAC		(1 << 6)		/* GPIB interface is System Controller */
#define PS_NOEOSEND	(1 << 7)		/* Do NOT set END in ibsta with EOS */
#define PS_SILENT	(1 << 8)		/* Do NOT print any messages from ibpoke */

#define PS_STATIC	(PS_SYSRDY | PS_NOINTS | PS_NOEOSEND | PS_SILENT)


/* used for interrupts */

#define USEINTS		(INTROP && SYSTIMO)	/* interrupts require SYSTIMO */

#if USEINTS
#define WaitingFor(i)	{if (!(pgmstat & PS_NOINTS)) osWaitForInt(i);}
#else
#define WaitingFor(i)
#endif


/* Timeout support macros */

#if SYSTIMO
#define TimedOut()	(!noTimo)		/* for testing within loops... */
#define NotTimedOut()	(noTimo)

#define TMFAC		HZ			/* timing factor for TM() macro */

#else
#define TimedOut()	((noTimo > 0) && !(--noTimo))
#define NotTimedOut()	((noTimo < 0) || ((noTimo > 0) && --noTimo))

#define TMFAC		(HZ * 1000)		/* timing factor for TM() macro */
#endif						/* -- approximate and system dependent! */

#define INITTIMO	(-1)			/* initial timeout setting */
#define TM(f,n,d)	((((f)*(n))/(d))+1)	/* clock ticks or counts per timo value */




/* driver states */


#define DRV_ONLINE   (1<<0)
#define DRV_IFC      (1<<1)
#define DRV_REN      (1<<2)


#endif	//_GPIB_REGISTERS_H

