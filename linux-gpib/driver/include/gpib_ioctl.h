
#include <asm/ioctl.h>

#define GPIB_CODE 160

typedef struct
{
	char name[100];
} board_type_ioctl_t;

// argument for read/write/command ioctls
typedef struct
{
	uint8_t *buffer;
	unsigned long count;
	int end;
} read_write_ioctl_t;

typedef struct
{
	unsigned int pad;
	int sad;
} open_close_dev_ioctl_t;

typedef struct
{
	unsigned int pad;
	int sad;
	uint8_t status_byte;
} serial_poll_ioctl_t;

typedef struct
{
	int eos;
	int eos_flags;
} eos_ioctl_t;

/* Standard functions. */
#define IBRD _IOWR( GPIB_CODE, 0, read_write_ioctl_t )
#define IBWRT _IOWR( GPIB_CODE, 1, read_write_ioctl_t )
#define IBCMD _IOWR( GPIB_CODE, 2, read_write_ioctl_t )
#define IBOPENDEV _IOW( GPIB_CODE, 3, open_close_dev_ioctl_t )
#define IBCLOSEDEV _IOW( GPIB_CODE, 4, open_close_dev_ioctl_t )
#define IBWAIT _IOWR( GPIB_CODE, 5, unsigned int )
#define IBRPP _IOWR( GPIB_CODE, 6, uint8_t )
#define IBAPE _IOW( GPIB_CODE, 7, int )
#define IBONL _IOW( GPIB_CODE, 8, int)
#define IBSIC _IO( GPIB_CODE, 9 )
#define IBSRE _IOW( GPIB_CODE, 10, int )
#define IBGTS _IO( GPIB_CODE, 11 )
#define IBCAC _IOW( GPIB_CODE, 12, int )
#define IBSTATUS _IOR( GPIB_CODE, 13, int )
#define IBLINES _IOR( GPIB_CODE, 14, short )
#define IBPAD _IOW( GPIB_CODE, 15, unsigned int )
#define IBSAD _IOW( GPIB_CODE, 16, int )
#define IBTMO _IOW( GPIB_CODE, 17, unsigned int )
#define IBRSP		_IOWR( GPIB_CODE, 18, serial_poll_ioctl_t )
#define IBEOS _IOW( GPIB_CODE, 19, eos_ioctl_t )
#define IBRSV _IOW( GPIB_CODE, 20, uint8_t )
#define CFCBASE _IOW( GPIB_CODE, 21, unsigned long )
#define CFCIRQ _IOW( GPIB_CODE, 22, unsigned int )
#define CFCDMA _IOW( GPIB_CODE, 23, unsigned int )
#define CFCBOARDTYPE _IOW( GPIB_CODE, 24, board_type_ioctl_t )



