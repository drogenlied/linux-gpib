#include <ibsys.h>
#include <linux/version.h>
#include <linux/module.h>

// early 2.4.x kernels don't define MODULE_LICENSE
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif
MODULE_LICENSE("GPL");

/* default debugging level */

#if DEBUG
#ifndef DEFAULT_DEBUG
unsigned int   dbgMask  = (DBG_ENTRY | DBG_EXIT | DBG_BRANCH | DBG_DATA | DBG_INTR | DBG_1PPL) & ~DBG_ALL;
#else
unsigned int   dbgMask  = 0;
#endif
#endif


/* static variables for debugging */

int   fidx     = 0;		/* index into function name stack		*/
char *fstk[32] =			/* function name stack (NO OVERFLOW PROTECTION)	*/
{
	"NULL"
};

/* format patterns for different function levels 
 * so debugging output is easier to read
 * this will cost a little memory of course but there is 
 * not much perfomance drop */

char *ffmt[32] = 
{
   "",
   " ",
   "  ",
   "   ",
   "    ",
   "     ",
   "      ",
   "       ",
   "        ",
   "         ",
   "          ",
   "           ",
   "            ",
   "             ",
   "              ",
   "               ",
   "*",
   "* ",
   "*  ",
   "*   ",
   "*    ",
   "*     ",
   "*      ",
   "*       ",
   "*        ",
   "*         ",
   "*          ",
   "*           ",
   "*            ",
   "*             ",
   "*              ",
   "*               "
};


#if SYSTIMO
int       espwdid   = 0;		/* watchdog timer ID for ESP routines */
#endif
#if USEINTS
struct semaphore espsemid;		/* semaphore ID for ESP interrupt support */
int       espintcon = 0;		        /* ESP interrupt routine is "connected" */
#endif

ibboard_t ibboard   = { 0 };	/* driver info, etc */

/*
 * LINUX specific stuff
 * 
 * 
 */

struct wait_queue *ibwait_queue = NULL;



/*
 * Linux initialization functions
 */
IBLCL int osInit(void)
{
	int	s;
extern  struct timer_list ibtimer_list;
extern  void ibintr(int irq, void *d, struct pt_regs *regs);
	int isr_flags = 0;

	DBGin("osInit");

#if SYSTIMO
	s = TICKSPERSEC;
	DBGprint(DBG_DATA, ("ClkRate=%d  ", s));
	if (s != timeTable[0]) {
		DBGprint(DBG_BRANCH, ("adjusting timeTable %d", s));
		timeTable[ 0] = s;			/* (New TMFAC)  */
		timeTable[ 1] = TM(s,10,1000000L);	/*  1: T10us    */
		timeTable[ 2] = TM(s,30,1000000L);	/*  2: T30us    */
		timeTable[ 3] = TM(s,100,1000000L);	/*  3: T100us   */
		timeTable[ 4] = TM(s,300,1000000L);	/*  4: T300us   */
		timeTable[ 5] = TM(s,1,1000);		/*  5: T1ms     */
		timeTable[ 6] = TM(s,3,1000);		/*  6: T3ms     */
		timeTable[ 7] = TM(s,10,1000);		/*  7: T10ms    */
		timeTable[ 8] = TM(s,30,1000);		/*  8: T30ms    */
		timeTable[ 9] = TM(s,100,1000);		/*  9: T100ms   */
		timeTable[10] = TM(s,300,1000);		/* 10: T300ms   */
		timeTable[11] = TM(s,1,1);		/* 11: T1s      */
		timeTable[12] = TM(s,3,1);		/* 12: T3s      */
		timeTable[13] = TM(s,10,1);		/* 13: T10s     */
		timeTable[14] = TM(s,30,1);		/* 14: T30s     */
		timeTable[15] = TM(s,100,1);		/* 15: T100s    */
		timeTable[16] = TM(s,300,1);		/* 16: T300s    */
		timeTable[17] = TM(s,1000,1);		/* 17: T1000s   */
	}
	ibtimer_list.list.next = ibtimer_list.list.prev = NULL;
#endif

#if USEINTS
        sema_init( &espsemid, 0);
	DBGprint(DBG_DATA, ("espsemid=0x%x  ", atomic_read(&espsemid.count)));
#endif
        /* avoid symbols to be exported (TB) */
#if 0
        register_symtab(NULL);
#endif
	/* register IRQ and DMA channel */

#if USEINTS
#if defined(CBI_PCI) || defined(MODBUS_PCI) || 	defined(INES_PCI)  
	isr_flags |= SA_SHIRQ;
#endif
	if( request_irq(ibirq, ibintr, isr_flags, "gpib", NULL)){
	  printk("can't request IRQ %d\n",ibirq);
          DBGout();
	  return(0);
	}
	DBGprint(DBG_DATA, ("IRQ %d  ", ibirq));
#if defined(CBI_PCI) || defined(MODBUS_PCI) || 	defined(INES_PCI)  
	pci_EnableIRQ ();
#endif

#endif
#if DMAOP
	if( request_dma( ibdma, "gpib" ) ){
	  printk("can't request DMA %d\n",ibdma );
#if USEINTS
	free_irq(ibirq, NULL);
#endif
          DBGout();
	  return(0);
	}
#endif
	pgmstat |= PS_SYSRDY;
	DBGout();
	return 1;
}


IBLCL void osReset(void)
{
	DBGin("osReset");

        if( pgmstat & PS_SYSRDY ){

#if USEINTS                /*release ressources */
	free_irq(ibirq, NULL);
#endif
#if DMAOP
	free_dma(ibdma);
#endif
        }
	pgmstat &= ~PS_SYSRDY;
	DBGout();
}



/***************************************************************************************

   Init module functions


****************************************************************************************/

struct file_operations ib_fops = {
  owner: THIS_MODULE,
  llseek: NULL,
  read: ibVFSread,
  write: ibVFSwrite,
  readdir: NULL,
  ioctl: ibioctl,
  mmap: NULL,
  open: ibopen,
  flush: NULL,
  release: ibclose,
};


#ifdef __cplusplus
extern "C" {
#endif

int ibmajor = IBMAJOR;   /* major number for dynamic configuration */


int init_module(void)
{

#if DEBUG
	if(dbgMask != 0 )
         	dbgMask |= DBG_1PPL;
#endif


#ifdef CBI_PCMCIA
   pcmcia_init_module();
#endif
#ifdef INES_PCMCIA
   pcmcia_init_module();
#endif


#ifdef CBI_PCI
   bd_PCIInfo();
#endif
#ifdef MODBUS_PCI
   bd_PCIInfo();
#endif
#ifdef INES_PCI
   bd_PCIInfo();
#endif   

        printk("Linux-GPIB Driver Board=%s -- Major=%d ",BOARD_TYPE,ibmajor);
#if !defined(CBI_PCMCIA) && !defined(INES_PCMCIA)  
        printk("Base=0x%x ",ibbase );
#if USEINTS
	printk("Irq=%d ",ibirq );
#endif
#endif
#if DMAOP
	printk("DMA=%d enabled\n",ibdma);
#else
	printk("DMA disabled\n");
#endif
#if DEBUG
	printk("-- DebugMask = 0x%x\n",dbgMask);
#endif
	printk("-- Kernel Release %s\n", UTS_RELEASE);

  	DBGin("ibinstall");
	
	if( register_chrdev(ibmajor,"gpib",&ib_fops)){
	  printk("can't get Major %d\n",ibmajor);
          DBGout();
	  return(-EIO);
	}
	osMemInit();
	DBGout();
	return 0;

}

void cleanup_module(void)
{
  DBGin("cleanup");

  if (MOD_IN_USE) {
    printk("gpib: device busy, remove delayed\n");
  }
  else {
    osReset();
  }

  osMemRelease();
 
  if ( unregister_chrdev(ibmajor, "gpib") != 0 ) {
    printk("gpib: device busy or other module error \n");
  } else {
    printk("gpib: succesfully removed \n");
  } 
#ifdef CBI_PCMCIA
  pcmcia_cleanup_module();
#endif
#ifdef INES_PCMCIA
  pcmcia_cleanup_module();
#endif    
#ifdef MODBUS_PCI
  bdPCIDetach();
#endif
#ifdef HP82335
	bdDetach();
#endif

  DBGout();
}



#ifdef __cplusplus
}
#endif

