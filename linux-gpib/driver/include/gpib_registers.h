
#define GPIBin(x)	bdP8in(&IB->x)	/* Board access macros...	*/
#define GPIBout(x,v)	bdP8out(&IB->x, v)

#define GPIBin16(x)	bdP16in(&IB->x)
#define GPIBout16(x,v)	bdP16out(&IB->x, v)

#define GPIBpgin(x)	(GPIBout(auxmr, AUX_PAGE), GPIBin(x))
#define GPIBpgout(x,v)	(GPIBout(auxmr, AUX_PAGE), GPIBout(x,v))


/*
 * I/O operation parameters
 */
#define	IO_READ		(1 << 0)	/* 1: I/O read; 0: I/O write	*/
#define	IO_NOHLDA	(1 << 1)	/* don't do HLDA carry-cycle	*/
#define	IO_NOEOI	(1 << 2)	/* don't send EOI w/last byte	*/
#define	IO_LAST		(1 << 3)	/* last "chunk" in I/O transfer	*/

#ifdef NIAT

/*
 * Register layout for NI AT-GPIB board
 *
 */
typedef struct ibregs {			/*  hardware registers for AT-GPIB      */
	uint8		cdor;		/*  +0 (x00) byte out register          */
	uint8		gpibc;		/*  +1 (x01) GPIB control lines         */
	uint8		imr1;		/*  +2 (x02) interrupt mask register 1  */
	uint8		gpibd;		/*  +3 (x03) GPIB data lines            */
	uint8		imr2;		/*  +4 (x04) interrupt mask register 2  */
	uint8		keyprt;		/*  +5 (x05) port for key and dmaEn bit */
	uint8		spmr;		/*  +6 (x06) serial poll mode register  */
	uint8		intrt;		/*  +7 (x07) interrupt register         */
	uint8		admr;		/*  +8 (x08) address mode register      */
	uint8	pad5,	auxmr;		/* +10 (x0A) auxiliary mode register    */
	uint8	pad6,	adr;		/* +12 (x0C) address register 0/1       */
	uint8	pad7,	eosr;		/* +14 (x0E) end of string register     */
					/* TURBO488...                          */
	uint8	pad8,	cfg;		/* +16 (x10) configuration register     */
	uint8	pad9,	imr3;		/* +18 (x12) interrupt mask register 3  */
	uint8	pad10,	cntl;		/* +20 (x14) byte count low register    */
	uint8	pad11,	cnth;		/* +22 (x16) byte count high register   */
	uint8	pad12;			/*           PADDING                    */
	fifo_t		fifo;		/* +24 (x18) 16-bit FIFO register (B/A) */
	uint8		ccrg;		/* +26 (x1A) carry cycle register       */
	uint8	pad13,	cmdr;		/* +28 (x1C) command register           */
	uint8	pad14,	timer;		/* +30 (x1E) timer register             */

} ibregs_t;

#endif

#ifdef NIPCII 

/*
 *  Register layout for PCII and PCIIa (in PCII mode :-) )
 *
 */

typedef struct ibregs {			/*  hardware registers for PCII      */
	uint8		cdor;		/*  (x00) byte out register          */
	uint8		imr1;		/*  (x01) interrupt mask register 1  */
	uint8		imr2;		/*  (x02) interrupt mask register 2  */
	uint8		spmr;		/*  (x03) serial poll mode register  */
	uint8		admr;		/*  (x04) address mode register      */
	uint8		auxmr;		/*  (x05) auxiliary mode register    */
	uint8		adr;		/*  (x06) address register 0/1       */
	uint8		eosr;		/*  (x07) end of string register     */
#ifdef CBI_4882
        uint8           hs_mode;        /*  (x08) HS_MODE register */
        uint8           hs_int_level;   /*  (x09) HS_INT_LEVEL register */

#endif
} ibregs_t;

#endif


#if defined(NIAT) || defined(NIPCII)


/* Read-only register mnemonics corresponding to write-only registers */

#define dir		cdor		/* Data In Register */
#define isr1		imr1		/* Interrupt Status Register 1 */
#define isr2		imr2		/* Interrupt Status Register 2 */
#define spsr		spmr		/* Serial Poll Status Register */
#define adsr		admr		/* Address Status Register */
#define cptr		auxmr		/* Command Pass Through Register */
#define adr0		adr		/* Address 0 Register */
#define adr1		eosr		/* Address 1 Register */ 

#ifdef CBI_4882
#define hs_status       hs_mode
#endif

#ifdef NIAT

#define sts1		cfg		/* T488 Status Register 1 */
#define sts2		cmdr	        /* T488 Status Register 2 */
#define isr3		ccrg		/* T488 Interrupt Status Register 3 */
#define dmaEn		keyprt		/* DMA Enable Register (Key Port) */

#endif

/* NAT4882 "Paged In" read- and write-only registers (must use GPIBpg__ macros) */

#define csr		spmr		/* Chip Signature Register */
#define keyr		spmr		/* Key Register */
#define sasr		auxmr		/* Source/Acceptor Status Register */
#define isr0		adr		/* Interrupt Status Register 0 */
#define imr0		adr		/* Interrupt Mask Register 0 */
#define bsr		eosr		/* Bus Status Register */ 
#define bcr		eosr		/* Bus Control Register */ 


/*--------------------------------------------------------------*/
/* 7210 bits:                POSITION              7210 reg     */
/*--------------------------------------------------------------*/
#define HR_DI           (unsigned char)(1<<0)	/* ISR1         */
#define HR_DO           (unsigned char)(1<<1)	/*  ,           */
#define HR_ERR          (unsigned char)(1<<2)	/*  ,           */
#define HR_DEC          (unsigned char)(1<<3)	/*  ,           */
#define HR_END          (unsigned char)(1<<4)	/*  ,           */
#define HR_DET          (unsigned char)(1<<5)	/*  ,           */
#define HR_APT          (unsigned char)(1<<6)	/*  ,           */
#define HR_CPT          (unsigned char)(1<<7)	/*  ,           */
#define HR_DIIE         (unsigned char)(1<<0)	/* IMR1         */
#define HR_DOIE         (unsigned char)(1<<1)	/*  ,           */
#define HR_ERRIE        (unsigned char)(1<<2)	/*  ,           */
#define HR_DECIE        (unsigned char)(1<<3)	/*  ,           */
#define HR_ENDIE        (unsigned char)(1<<4)	/*  ,           */
#define HR_DETIE        (unsigned char)(1<<5)	/*  ,           */
#define HR_APTIE        (unsigned char)(1<<6)	/*  ,           */
#define HR_CPTIE        (unsigned char)(1<<7)	/*  ,           */
#define HR_ADSC         (unsigned char)(1<<0)	/* ISR2         */
#define HR_REMC         (unsigned char)(1<<1)	/*  ,           */
#define HR_LOKC         (unsigned char)(1<<2)	/*  ,           */
#define HR_CO           (unsigned char)(1<<3)	/*  ,           */
#define HR_REM          (unsigned char)(1<<4)	/*  ,           */
#define HR_LOK          (unsigned char)(1<<5)	/*  ,           */
#define HR_SRQI         (unsigned char)(1<<6)	/*  ,           */
#define HR_INT          (unsigned char)(1<<7)	/*  ,           */
#define HR_ACIE         (unsigned char)(1<<0)	/* IMR2         */
#define HR_REMIE        (unsigned char)(1<<1)	/*  ,           */
#define HR_LOKIE        (unsigned char)(1<<2)	/*  ,           */
#define HR_COIE         (unsigned char)(1<<3)	/*  ,           */
#define HR_DMAI         (unsigned char)(1<<4)	/*  ,           */
#define HR_DMAO         (unsigned char)(1<<5)	/*  ,           */
#define HR_SRQIE        (unsigned char)(1<<6)	/*  ,           */
#define HR_PEND         (unsigned char)(1<<6)	/* SPSR         */
#define HR_RSV          (unsigned char)(1<<6)	/* SPMR         */
#define HR_MJMN         (unsigned char)(1<<0)	/* ADSR         */
#define HR_TA           (unsigned char)(1<<1)	/*  ,           */
#define HR_LA           (unsigned char)(1<<2)	/*  ,           */
#define HR_TPAS         (unsigned char)(1<<3)	/*  ,           */
#define HR_LPAS         (unsigned char)(1<<4)	/*  ,           */
#define HR_SPMS         (unsigned char)(1<<5)	/*  ,           */
#define HR_NATN         (unsigned char)(1<<6)	/*  ,           */
#define HR_CIC          (unsigned char)(1<<7)	/*  ,           */
#define HR_ADM0         (unsigned char)(1<<0)	/* ADMR         */
#define HR_ADM1         (unsigned char)(1<<1)	/*  ,           */
#define HR_TRM0         (unsigned char)(1<<4)	/*  ,           */
#define HR_TRM1         (unsigned char)(1<<5)	/*  ,           */
#define HR_LON          (unsigned char)(1<<6)	/*  ,           */
#define HR_TON          (unsigned char)(1<<7)	/*  ,           */
#define HR_DL           (unsigned char)(1<<5)	/* ADR          */
#define HR_DT           (unsigned char)(1<<6)	/*  ,           */
#define HR_ARS          (unsigned char)(1<<7)	/*  ,           */
#define HR_EOI          (unsigned char)(1<<7)	/* ADR1         */
#define HR_HLDA         (unsigned char)(1<<0)	/* auxra        */
#define HR_HLDE         (unsigned char)(1<<1)	/*  ,           */
#define HR_REOS         (unsigned char)(1<<2)	/*  ,           */
#define HR_XEOS         (unsigned char)(1<<3)	/*  ,           */
#define HR_BIN          (unsigned char)(1<<4)	/*  ,           */
#define HR_CPTE         (unsigned char)(1<<0)	/* auxrb        */
#define HR_SPEOI        (unsigned char)(1<<1)	/*  ,           */
#define HR_TRI          (unsigned char)(1<<2)	/*  ,           */
#define HR_INV          (unsigned char)(1<<3)	/*  ,           */
#define HR_ISS          (unsigned char)(1<<4)	/*  ,           */
#define HR_PPS          (unsigned char)(1<<3)	/* ppr          */
#define HR_PPU          (unsigned char)(1<<4)	/*  ,           */

#define HR_LCM (unsigned char)(HR_HLDE|HR_HLDA)	/* auxra listen continuous */

#define CR_EOI		(unsigned char)(1<<7)	/* gpibc        */
#define CR_ATN		(unsigned char)(1<<6)	/*  ,           */
#define CR_SRQ		(unsigned char)(1<<5)	/*  ,           */
#define CR_REN		(unsigned char)(1<<4)	/*  ,           */
#define CR_IFC		(unsigned char)(1<<3)	/*  ,           */
#define CR_NRFD		(unsigned char)(1<<2)	/*  ,           */
#define CR_NDAC		(unsigned char)(1<<1)	/*  ,           */
#define CR_DAV		(unsigned char)(1<<0)	/*  ,           */

#define BR_ATN		(unsigned char)(1<<7)	/* bsr and bcr  */
#define BR_DAV		(unsigned char)(1<<6)	/*  ,           */
#define BR_NDAC		(unsigned char)(1<<5)	/*  ,           */
#define BR_NRFD		(unsigned char)(1<<4)	/*  ,           */
#define BR_EOI		(unsigned char)(1<<3)	/*  ,           */
#define BR_SRQ		(unsigned char)(1<<2)	/*  ,           */
#define BR_IFC		(unsigned char)(1<<1)	/*  ,           */
#define BR_REN		(unsigned char)(1<<0)	/*  ,           */

#define HR_nba		(unsigned char)(1<<7)	/* sasr         */
#define HR_AEHS		(unsigned char)(1<<6)	/*  ,           */
#define HR_ANHS1	(unsigned char)(1<<5)	/*  ,           */
#define HR_ANHS2	(unsigned char)(1<<4)	/*  ,           */
#define HR_ADHS		(unsigned char)(1<<3)	/*  ,           */
#define HR_ACRDY	(unsigned char)(1<<2)	/*  ,           */
#define HR_SH1A		(unsigned char)(1<<1)	/*  ,           */
#define HR_SH1B		(unsigned char)(1<<0)	/*  ,           */


#define ICR		(unsigned char)0040	/* AUXMR control masks for hidden regs */
#define PPR		(unsigned char)0140
#define AUXRA		(unsigned char)0200
#define AUXRB		(unsigned char)0240
#define AUXRE		(unsigned char)0300
#define LOMASK		0x1F			/* mask to specify lower 5 bits */

/* 7210 Auxiliary Commands */
#define AUX_PON         (unsigned char)000	/* Immediate Execute pon                  */
#define AUX_CPPF        (unsigned char)001	/* Clear Parallel Poll Flag               */
#define AUX_CR          (unsigned char)002	/* Chip Reset                             */
#define AUX_FH          (unsigned char)003	/* Finish Handshake                       */
#define AUX_TRIG        (unsigned char)004	/* Trigger                                */
#define AUX_RTL         (unsigned char)005	/* Return to local                        */
#define AUX_SEOI        (unsigned char)006	/* Send EOI                               */
#define AUX_NVAL        (unsigned char)007	/* Non-Valid Secondary Command or Address */
#define AUX_SPPF        (unsigned char)011	/* Set Parallel Poll Flag                 */
#define AUX_VAL         (unsigned char)017	/* Valid Secondary Command or Address     */
#define AUX_TCA         (unsigned char)021	/* Take Control Asynchronously            */
#define AUX_GTS         (unsigned char)020	/* Go To Standby                          */
#define AUX_TCS         (unsigned char)022	/* Take Control Synchronously             */
#define AUX_LTN         (unsigned char)023	/* Listen                                 */
#define AUX_DSC         (unsigned char)024	/* Disable System Control                 */
#define AUX_CIFC        (unsigned char)026	/* Clear IFC                              */
#define AUX_CREN        (unsigned char)027	/* Clear REN                              */
#define AUX_TCSE        (unsigned char)032	/* Take Control Synchronously on End      */
#define AUX_LTNC        (unsigned char)033	/* Listen in Continuous Mode              */
#define AUX_LUN         (unsigned char)034	/* Local Unlisten                         */
#define AUX_EPP         (unsigned char)035	/* Execute Parallel Poll                  */
#define AUX_SIFC        (unsigned char)036	/* Set IFC                                */
#define AUX_SREN        (unsigned char)037	/* Set REN                                */

#define AUX_PAGE	(unsigned char)0x50	/* Page-In additional registers           */	

/* CBI-488.2 additional Registers */

/* hidden line status register */

#define LN_ATN          (unsigned char)(1<<0)
#define LN_EOI          (unsigned char)(1<<1)
#define LN_SRQ          (unsigned char)(1<<2)
#define LN_IFC          (unsigned char)(1<<3)
#define LN_REN          (unsigned char)(1<<4)
#define LN_DAV          (unsigned char)(1<<5)
#define LN_NRFD         (unsigned char)(1<<6)
#define LN_NDAC         (unsigned char)(1<<7)

/* CBI 488.2 HS control */

#define HS_TX_ENABLE    (unsigned char)(1<<0)
#define HS_RX_ENABLE    (unsigned char)(1<<1)
#define HS_ODD_BYTE     (unsigned char)(1<<2)
#define HS_HF_INT_EN    (unsigned char)(1<<3)
#define HS_CLR_SRQ_INT  (unsigned char)(1<<4)
#define HS_CLR_EOI_INT  (unsigned char)(1<<5) /* RX enabled */
#define HS_CLR_EMPTY_INT (unsigned char)(1<<5) /* TX enabled */
#define HS_CLR_HF_INT   (unsigned char)(1<<6)
#define HS_SYS_CONTROL  (unsigned char)(1<<7)

/* CBI 488.2 status */

#define HS_FIFO_FULL    (unsigned char)(1<<0)
#define HS_HALF_FULL    (unsigned char)(1<<1)
#define HS_SRQ_INT      (unsigned char)(1<<2)
#define HS_EOI_INT      (unsigned char)(1<<3)
#define HS_TX_MSB_EMPTY (unsigned char)(1<<4)
#define HS_RX_MSB_EMPTY (unsigned char)(1<<5)
#define HS_TX_LSB_EMPTY (unsigned char)(1<<6)
#define HS_RX_LSB_EMPTY (unsigned char)(1<<7)

/* CBI488.2 hs_int_level register */

#define HS_RESET7210    (unsigned char)(1<<7)
#define AUX_HISPEED     0x41
#define AUX_LOSPEED     0x40


/** bus timing */

#define T1_2000_NS 0
#define T1_500NS   1
#define T1_350_NS  2



/*============================================================*/

/* TURBO-488 registers bit definitions */

/* STS1 -- Status Register 1 (read only) */
#define S_DONE          (unsigned char)(0x80)	/* DMA done                           */
#define S_SC            (unsigned char)(0x40)	/* is system contoller                */
#define S_IN            (unsigned char)(0x20)	/* DMA in (to memory)                 */
#define S_DRQ           (unsigned char)(0x10)	/* DRQ line (for diagnostics)         */
#define S_STOP          (unsigned char)(0x08)	/* DMA stopped                        */
#define S_NDAV          (unsigned char)(0x04)	/* inverse of DAV                     */
#define S_HALT          (unsigned char)(0x02)	/* status of transfer machine         */
#define S_GSYNC         (unsigned char)(0x01)	/* indicates if GPIB is in sync w I/O */

/* CFG -- Configuration Register (write only) */
#define	C_CMD	        (unsigned char)(1<<7)	/* FIFO 'bcmd' in progress            */
#define	C_TLCH	        (unsigned char)(1<<6)	/* halt DMA on TLC interrupt          */
#define	C_IN	        (unsigned char)(1<<5)	/* DMA is a GPIB read                 */
#define	C_A_B	        (unsigned char)(1<<4)	/* fifo order 1=motorola, 0=intel     */
#define	C_CCEN	        (unsigned char)(1<<3)	/* enable carry cycle                 */
#define	C_TMOE	        (unsigned char)(1<<2)	/* enable CPU bus time limit          */
#define	C_T_B	        (unsigned char)(1<<1)  	/* tmot reg is: 1=125ns clocks,       */
						/* 0=num bytes                        */
#define	C_B16	        (unsigned char)(1<<0)  	/* 1=FIFO is 16-bit register, 0=8-bit */

/* ISR3 -- Interrupt Status Register (read only) */
#define	HR_INTR	        (unsigned char)(1<<7)	/* 1=board is interrupting	*/
#define	HR_SRQI_CIC	(unsigned char)(1<<5)	/* SRQ asserted and we are CIC	*/
#define	HR_STOP         (unsigned char)(1<<4)	/* fifo empty or STOP command	*/
						/* issued			*/
#define	HR_NFF	        (unsigned char)(1<<3)	/* NOT full fifo		*/
#define	HR_NEF	        (unsigned char)(1<<2)	/* NOT empty fifo		*/
#define	HR_TLCI	        (unsigned char)(1<<1)	/* TLC interrupt asserted	*/
#define	HR_DONE         (unsigned char)(1<<0)	/* DMA done			*/

/* CMDR -- Command Register */
#define	CLRSC		(unsigned char)0x2	/* clear the SC bit 		*/
#define	SETSC		(unsigned char)0x3	/* set the SC bit 		*/
#define	GO		(unsigned char)(1<<2)	/* start DMA 			*/
#define	STOP		(unsigned char)(1<<3)	/* stop DMA 			*/
#define	RSTFIFO		(unsigned char)(1<<4)	/* reset the FIFO 		*/
#define SFTRST		(unsigned char)(1<<5)	/* issue a software reset 	*/
#define	DU_ADD		(unsigned char)(1<<6)	/* Motorola mode dual 	  	*/
#define	DDU_ADD		(unsigned char)(1<<7)	/* Disable dual addressing 	*/

/* STS2 -- Status Register 2 */
#define AFFN		(unsigned char)(1<<3)	/* "A full FIFO NOT"  (0=FIFO full)  */
#define AEFN		(unsigned char)(1<<2)	/* "A empty FIFO NOT" (0=FIFO empty) */
#define BFFN		(unsigned char)(1<<1)	/* "B full FIFO NOT"  (0=FIFO full)  */
#define BEFN		(unsigned char)(1<<0)	/* "B empty FIFO NOT" (0=FIFO empty) */


/* Miscellaneous Register Definitions */

#define HR_INTEN	(unsigned char)(1<<0)	/* INTRT */
#define HR_DMAEN	(unsigned char)(1<<0)	/* DMA   */

#endif

/*-================================================================================

HP 82335 (memory mapped) or plain (TMS9914 i/o) register definitions


================================================================================*/

#if defined(ZIATECH)
#define TMS9914 1
#endif


#if defined(HP82335) || defined(TMS9914)

/* NOTE: The card interrupt clear register is not correctly
 * documented, it refers to ccir[0x37f7]
 */


#if defined(HP82335)

typedef struct ibregs {

  uint8     ccir[0x37f8];     /* first unused register range */


  uint8     csr[0x800];       /* card status register        */
  uint8     isr0;             /* interrupt status 0          */
  uint8     isr1;             /* interrupt status 1          */
  uint8     adsr;             /* adress status               */
  uint8     bsr;              /* bus-status                  */
  uint8     hpibsr;           /* hpib-staus                  */
  uint8     unusedr;          /* unused for reading, writing spmr */
  uint8     cptr;             /* command pass thru           */
  uint8     dir;              /* data in register            */

} ibregs_t;

/* Register Mnemonics for write access */

#define     ccr     csr          /* card control register     */
#define     imr0    isr0         /* interrupt mask 0          */
#define     imr1    isr1         /* interrupt mask 1          */
#define     auxcr   bsr          /* auxiliary command         */
                                 /* adsr unused ?             */
#define     adr     hpibsr       /* adress register           */
#define     spmr    unusedr      /* serial poll mode          */
#define     ppr     cptr         /* parallel poll             */
#define     cdor    dir          /* data out register         */

#endif 

#if defined(TMS9914)

typedef struct ibregs {

  uint8     isr0;             /* interrupt status 0          */
  uint8     isr1;             /* interrupt status 1          */
  uint8     adsr;             /* adress status               */
  uint8     bsr;              /* bus-status                  */
  uint8     adswr;            /* address-switch/address      */
  uint8     unusedr;          /* 1488AIRQstat, writing spmr */
  uint8     cptr;             /* command pass thru           */
  uint8     dir;              /* data in register            */

} ibregs_t;

#if defined(ZIATECH)

#define ctrr adsr             /* Z1488A control register     */
#define istat unusedr         /* Z1488A IRQ status register  */

#endif

/* Register Mnemonics for write access */

#define     imr0    isr0         /* interrupt mask 0          */
#define     imr1    isr1         /* interrupt mask 1          */
#define     auxcr   bsr          /* auxiliary command         */
                                 /* adsr unused ?             */
#define     adr     adswr        /* adress register           */
#define     spmr    unusedr      /* serial poll mode          */
#define     ppr     cptr         /* parallel poll             */
#define     cdor    dir          /* data out register         */

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



