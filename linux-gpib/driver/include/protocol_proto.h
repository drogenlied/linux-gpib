/***** Public Functions ******/
extern  int dvtrg(int padsad);
extern  int dvclr(int padsad);
extern  int dvrsp(int padsad, uint8_t *result);
extern  ssize_t dvwrt(int padsad, uint8_t *buf, unsigned int cnt);
extern  int receive_setup(int padsad);
extern  int send_setup(int padsad);
extern  int ibAPWait(int pad);
extern  int ibAPrsp(int padsad, char *spb);
extern  void ibAPE(int pad, int v);
extern  int ibcac(int sync);
extern  ssize_t ibcmd(uint8_t *buf, size_t length);
extern  int ibgts(void);
extern  int ibonl(int v);
extern  int iblines(int *buf);
extern  ssize_t ibrd(uint8_t *buf, size_t length, int *end_flag);
extern  int ibrpp(uint8_t *buf);
extern  int ibrsv(uint8_t poll_status);
extern  int ibsic(void);
extern  int ibsre(int enable);
extern  int ibpad(int v);
extern  int ibsad(int v);
extern  int ibtmo(int v);
extern  int ibeot(int send_eoi);
extern  int ibeos(int v);
extern  int ibwait(unsigned int mask);
extern  ssize_t ibwrt(uint8_t *buf, size_t cnt, int more);

