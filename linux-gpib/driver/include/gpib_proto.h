
#ifndef GPIB_PROTO_INCLUDED
#define GPIB_PROTO_INCLUDED

#include <board_proto.h>
#include <sys_proto.h>
#include <protocol_proto.h>
#include <linux/kernel.h>


  /* osutil.c */
#ifndef COMPILING_SYS_OSUTIL_C	/* Dirty trick to remove some warnings */
extern IBLCL void osPrint();
#endif

#if 0

  /* osadjcnt.c */
extern IBLCL void osDMAAdjCnt(ibio_op_t *rwop);

  /* osinterrupt.c */
extern IBLCL void osWaitForInt(int imr3mask);

  /* osutil.c */
extern IBLCL uint8 osP8in(short in_addr);
extern IBLCL void osP8out(short out_addr, uint8 out_value);	
extern IBLCL uint16 osP16in(short in_addr);
extern IBLCL void osP16out(short out_addr, uint16 out_value);
extern IBLCL void osChngBase(int new_base);
extern IBLCL void osChngIRQ(int new_irq);
extern IBLCL void osChngDMA(int new_dma);

  /* osdma.c */
extern IBLCL int osDoDMA(ibio_op_t *rwop);

  /* oslock.c */
extern void osLockMutex( void );
extern void osUnlockMutex( void );

  /* osmem.c */
extern IBLCL void osMemInit(void);
extern void osFreeDMABuffer( char *buf );
extern IBLCL void osMemRelease(void);

  /* ostimer.c */
extern IBLCL void osStartTimer(int v);
extern IBLCL void osRemoveTimer();

  /* osinit.c */
extern IBLCL int osInit();
extern IBLCL void osReset();




extern IBLCL fnInit();
extern IBLCL receive_setup();
extern IBLCL send_setup();
extern IBLCL ibstat();

extern IBLCL int bdSendAuxCmd(int cmd);
extern IBLCL int bdSendAuxACmd(int cmd);
extern int  bdWaitIn(void);
extern bdWaitOut(void);
extern bdWaitATN(void);

 /* board */

extern uint8 bdP8in();
extern void  bdP8out();

#if DMAOP
extern IBLCL bdDMAAdjCnt();
extern IBLCL bdDMAread();
extern IBLCL bdDMAwrt();
extern IBLCL int bdDMAwait();
extern IBLCL bdDMAstart();
extern IBLCL bdDMAstop();
#endif

extern IBLCL bdPIOAdjCnt();
extern IBLCL bdPIOread();
extern IBLCL bdPIOwrt();

extern IBLCL bdcmd();
extern IBLCL bdonl();
extern IBLCL bdlines();

extern IBLCL bdSRQstat();
extern IBLCL bdsc();
extern IBLCL bdGetDataByte(void);
extern IBLCL bdGetCmdByte(void);
extern IBLCL bdGetAdrStat();
extern IBLCL bdCheckEOI();
extern IBLCL bdSetEOS();
extern IBLCL bdGetEOS();
extern IBLCL bdSetSPMode();
extern IBLCL bdSetPAD();
extern IBLCL bdSetSAD();
extern IBLCL bdwait();


/* protocol */

extern IBLCL int dvtrg(int padsad);
extern IBLCL int dvclr(int padsad);
extern IBLCL int dvrsp(int padsad,char *spb);
extern IBLCL int dvrd(int padsad,faddr_t buf,unsigned int cnt);
extern IBLCL int dvwrt(int padsad,faddr_t buf,unsigned int cnt);
extern IBLCL int receive_setup(int padsad,int spoll);
extern IBLCL int send_setup(int padsad);
extern IBLCL int fnInit(int reqd_adsbit);
extern IBLCL int ibcac(int v);
extern IBLCL int ibcmd(faddr_t buf, unsigned int cnt);
extern IBLCL int ibgts();
extern IBLCL int ibonl(int v);
extern IBLCL int iblines(int *buf);
extern IBLCL int ibrd(faddr_t buf, unsigned int cnt);
extern IBLCL int ibrpp(faddr_t buf);
extern IBLCL int ibrsv(int v);
extern IBLCL int ibsic(void);
extern IBLCL int ibsre(int v);
extern IBLCL int ibpad(int v);
extern IBLCL int ibsad(int v);
extern IBLCL int ibtmo(int v);
extern IBLCL int ibeot(int v);
extern IBLCL int ibeos(int v);
extern IBLCL int ibstat(void);
extern IBLCL int ibwait(unsigned int mask);
extern IBLCL int ibwrt(faddr_t buf, unsigned int cnt);


#endif






#endif /* GPIB_PROTO_INCLUDED */
