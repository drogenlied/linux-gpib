/***** Public Functions ******/
extern  int ibopen(struct inode *inode, struct file *filep);
extern  int ibclose(struct inode *inode, struct file *file);
extern  int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg);
extern  int osInit(void);
extern  void osReset(void);
extern  void osMemInit(void); 
extern  void osMemRelease(void); 
extern  char *osGetDMABuffer( int *size );
extern  void osFreeDMABuffer( char *buf );
extern  void watchdog_timeout(unsigned long arg);
extern  void osStartTimer(int v);
extern  void osRemoveTimer(void);
extern  void osSendEOI(void);
extern  void osSendEOI(void);
extern  void osChngBase(unsigned long new_base);
extern  void osChngIRQ(int new_irq);
extern  void osChngDMA(int new_dma);

