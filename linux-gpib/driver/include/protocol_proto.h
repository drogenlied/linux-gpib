/***** Public Functions ******/
extern  int dvrsp(gpib_device_t *device, int padsad, uint8_t *result);
extern  int ibAPWait(gpib_device_t *device, int pad);
extern  int ibAPrsp(gpib_device_t *device, int padsad, char *spb);
extern  void ibAPE(gpib_device_t *device, int pad, int v);
extern  int ibcac(gpib_device_t *device, int sync);
extern  ssize_t ibcmd(gpib_device_t *device, uint8_t *buf, size_t length);
extern  int ibgts(gpib_device_t *device);
extern  int ibonl(gpib_device_t *device, int v);
extern  int iblines(gpib_device_t *device, int *buf);
extern  ssize_t ibrd(gpib_device_t *device, uint8_t *buf, size_t length, int *end_flag);
extern  int ibrpp(gpib_device_t *device, uint8_t *buf);
extern  int ibrsv(gpib_device_t *device, uint8_t poll_status);
extern  int ibsic(gpib_device_t *device);
extern  int ibsre(gpib_device_t *device, int enable);
extern  int ibpad(gpib_device_t *device, int v);
extern  int ibsad(gpib_device_t *device, int v);
extern  int ibtmo(gpib_device_t *device, unsigned int v);
extern  int ibeot(gpib_device_t *device, int send_eoi);
extern  int ibeos(gpib_device_t *device, int v);
extern  int ibwait(gpib_device_t *device, unsigned int mask);
extern  ssize_t ibwrt(gpib_device_t *device, uint8_t *buf, size_t cnt, int more);
extern unsigned int ibstatus(gpib_device_t *device);

