
#ifdef __cplusplus
extern "C" {
#endif
/***** Public Functions ******/
extern  int ibape(int ud, int v);
extern  int ibAPwait(int ud, int mask);
extern  int ibcac(int ud, int v);
extern  int ibclr(int ud);
extern  int ibcmd(int ud,char *cmd,unsigned long cnt);
extern  int ibconfig( int ud, int option, int value );
extern int ibdev(int minor, int pad, int sad, int timo, int eot, int eos);
extern  int ibdma( int ud, int v );
extern  int ibeot(int ud, int v);
extern  int ibevent(int ud, ibEventHandler handler );
extern  int ibfind(char *dev);
extern  void FindLstn( int boardID, Addr4882_t *padlist, Addr4882_t *resultlist, int limit );
extern  int ibIsMaster(int ud);
extern  int ibgts(int ud, int v);
extern  int iblines(int ud, unsigned short *buf);
extern  int ibloc(int ud);
extern  int ibonl(int ud, int onl);
extern  int ibchbase(int ud, int base);
extern  int ibchirq(int ud, int base);
extern  int ibchdma(int ud, int base);
extern  int ibpad(int ud, int v);
extern  int ibrd(int ud, uint8_t *rd, unsigned long cnt);
extern  int ibrpp(int ud, char *ppr);
extern  int ibrsp(int ud, char *spr);
extern  int ibrsv(int ud, int v);
extern  int ibsad(int ud, int v);
extern  int ibSdbg(int ud,int mode);
extern  int ibsic(int ud);
extern  int ibsre(int ud, int v);
extern  int ibtmo(int ud,int v);
extern  int ibtrg(int ud);
extern  int ibwait(int ud, int mask);
extern  int ibwrt(int ud, char *rd, unsigned long cnt);
/***** Private Functions ******/
extern int ibCheckDescriptor(int ud);
extern  int ibBdChrConfig(int ud, int base, int irq, int dma);
extern  void ibBoardDefaultValues(void);
extern  int ibBoardOpen(int bd,int flags);
extern  int ibBoardClose(int bd);
extern  int ibBoardFunc (int bd, int code, ...);
extern  int ibGetNrBoards(void);
extern  void yyerror(char *s);
extern  int ibeos(int ud, int v);
extern  int iblcleos(int ud);
extern  char *ibVerbCode(int code);
extern  void ibPutMsg (char *format,...); 
extern  void ibPutErrlog(int ud,char *routine);
extern  int ibParseConfigFile(char *filename);
extern  int ibGetDescriptor(ibConf_t *p);
extern  int ibFindDevIndex(char *name);
#ifdef __cplusplus
}
#endif
