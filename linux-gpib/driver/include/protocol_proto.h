/***** Public Functions ******/
extern int dvrsp(gpib_board_t *board, unsigned int pad, int sad,
	unsigned int usec_timeout, uint8_t *result );
extern int ibAPWait(gpib_board_t *board, int pad);
extern int ibAPrsp(gpib_board_t *board, int padsad, char *spb);
extern void ibAPE(gpib_board_t *board, int pad, int v);
extern int ibcac(gpib_board_t *board, int sync);
extern ssize_t ibcmd( gpib_board_t *board, uint8_t *buf, size_t length );
extern int ibgts(gpib_board_t *board);
extern int ibonline( gpib_board_t *board, gpib_file_private_t *priv );
extern int iboffline( gpib_board_t *board, gpib_file_private_t *priv );
extern int iblines( const gpib_board_t *board, short *lines );
extern ssize_t ibrd(gpib_board_t *board, uint8_t *buf, size_t length, int *end_flag);
extern int ibrpp( gpib_board_t *board, uint8_t *buf );
extern int ibrsv(gpib_board_t *board, uint8_t poll_status);
extern void ibrsc( gpib_board_t *board, int request_control );
extern int ibsic( gpib_board_t *board, unsigned int usec_duration );
extern int ibsre(gpib_board_t *board, int enable);
extern int ibpad( gpib_board_t *board, unsigned int addr );
extern int ibsad( gpib_board_t *board, int addr );
extern int ibeos( gpib_board_t *board, int eos, int eosflags );
extern int ibwait(gpib_board_t *board, unsigned int mask, unsigned int pad,
	int sad, unsigned long usec_timeout );
extern ssize_t ibwrt(gpib_board_t *board, uint8_t *buf, size_t cnt, int more );
extern unsigned int ibstatus( gpib_board_t *board, unsigned int clear_mask );
extern unsigned int full_ibstatus( gpib_board_t *board, const gpib_device_t *device );
extern int io_timed_out( gpib_board_t *board );
extern int ibppc( gpib_board_t *board, uint8_t configuration );

