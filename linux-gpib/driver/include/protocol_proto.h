/* Automatically created by mkproto.sh. DONT EDIT */
/***** Public Functions ******/
extern  int dvtrg(int padsad);
extern  int dvclr(int padsad);
extern  int dvrsp(int padsad,char *spb);
extern  int dvrd(int padsad,faddr_t buf,unsigned int cnt);
extern  int dvwrt(int padsad,faddr_t buf,unsigned int cnt);
extern  int receive_setup(int padsad,int spoll);
extern  int send_setup(int padsad);
extern  int fnInit(int reqd_adsbit);
extern  int ibcac(int v);
extern  int ibcmd(faddr_t buf, unsigned int cnt);
extern  int ibgts(void);
extern  int ibonl(int v);
extern  int iblines(int *buf);
extern  int ibrd(faddr_t buf, unsigned int cnt);
extern  int ibrpp(faddr_t buf);
extern  int ibrsv(int v);
extern  int ibsic(void);
extern  int ibsre(int v);
extern  int ibpad(int v);
extern  int ibsad(int v);
extern  int ibtmo(int v);
extern  int ibeot(int v);
extern  int ibeos(int v);
extern  int ibstat(void);			
extern  int ibwait(unsigned int mask);
extern  int ibwrt(faddr_t buf, unsigned int cnt);
