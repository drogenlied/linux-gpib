
#ifndef GPIB_PROTO_INCLUDED
#define GPIB_PROTO_INCLUDED

#include <linux/fs.h>

int ibopen( struct inode *inode, struct file *filep );
int ibclose( struct inode *inode, struct file *file );
int ibioctl( struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg );
int osInit( void );
void osReset( void );
void watchdog_timeout( unsigned long arg );
void osStartTimer( gpib_board_t *board, unsigned int usec_timeout );
void osRemoveTimer( gpib_board_t *board );
void osSendEOI( void );
void osSendEOI( void );
void init_gpib_board( gpib_board_t *board );
unsigned int usec_to_jiffies( unsigned int usec );
int serial_poll_all( gpib_board_t *board, unsigned int usec_timeout );
void init_gpib_descriptor( gpib_descriptor_t *desc );
int dvrsp(gpib_board_t *board, unsigned int pad, int sad,
	unsigned int usec_timeout, uint8_t *result );
int ibAPWait(gpib_board_t *board, int pad);
int ibAPrsp(gpib_board_t *board, int padsad, char *spb);
void ibAPE(gpib_board_t *board, int pad, int v);
int ibcac(gpib_board_t *board, int sync);
ssize_t ibcmd( gpib_board_t *board, uint8_t *buf, size_t length );
int ibgts(gpib_board_t *board);
int ibonline( gpib_board_t *board, gpib_file_private_t *priv );
int iboffline( gpib_board_t *board, gpib_file_private_t *priv );
int iblines( const gpib_board_t *board, short *lines );
ssize_t ibrd(gpib_board_t *board, uint8_t *buf, size_t length, int *end_flag);
int ibrpp( gpib_board_t *board, uint8_t *buf );
int ibrsv(gpib_board_t *board, uint8_t poll_status);
void ibrsc( gpib_board_t *board, int request_control );
int ibsic( gpib_board_t *board, unsigned int usec_duration );
int ibsre(gpib_board_t *board, int enable);
int ibpad( gpib_board_t *board, unsigned int addr );
int ibsad( gpib_board_t *board, int addr );
int ibeos( gpib_board_t *board, int eos, int eosflags );
int ibwait(gpib_board_t *board, int wait_mask, int clear_mask, int *status,
	unsigned long usec_timeout, gpib_descriptor_t *desc );
ssize_t ibwrt(gpib_board_t *board, uint8_t *buf, size_t cnt, int more );
int ibstatus( gpib_board_t *board );
int general_ibstatus( gpib_board_t *board, const gpib_status_queue_t *device,
	int clear_mask, const gpib_descriptor_t *desc );
int io_timed_out( gpib_board_t *board );
int ibppc( gpib_board_t *board, uint8_t configuration );

#endif /* GPIB_PROTO_INCLUDED */
