/***** Public Functions ******/
extern  int ibopen( struct inode *inode, struct file *filep );
extern  int ibclose( struct inode *inode, struct file *file );
extern  int ibioctl( struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg );
extern  int osInit( void );
extern  void osReset( void );
extern  void watchdog_timeout( unsigned long arg );
extern  void osStartTimer( gpib_board_t *board, unsigned int usec_timeout );
extern  void osRemoveTimer( gpib_board_t *board );
extern  void osSendEOI( void );
extern  void osSendEOI( void );
extern  void osChngBase( gpib_board_t *board, unsigned long new_base );
extern  void osChngIRQ( gpib_board_t *board, int new_irq );
extern  void osChngDMA( gpib_board_t *board, int new_dma );

