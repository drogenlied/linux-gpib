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
extern  void osStartTimer(gpib_device_t *device, int v);
extern  void osRemoveTimer(gpib_device_t *device);
extern  void osSendEOI(void);
extern  void osSendEOI(void);
extern  void osChngBase(gpib_device_t *device, unsigned long new_base);
extern  void osChngIRQ(gpib_device_t *device, int new_irq);
extern  void osChngDMA(gpib_device_t *device, int new_dma);

