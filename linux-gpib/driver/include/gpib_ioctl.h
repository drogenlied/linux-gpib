
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


/* Standard functions. */
#define IBRD _IOWR( GPIB_CODE, 0, read_write_ioctl_t )
#define IBWRT _IOWR( GPIB_CODE, 1, read_write_ioctl_t )
#define IBCMD _IOWR( GPIB_CODE, 2, read_write_ioctl_t )
#define IBOPENDEV _IOW( GPIB_CODE, 3, open_close_dev_ioctl_t )
#define IBCLOSEDEV _IOW( GPIB_CODE, 4, open_close_dev_ioctl_t )
#define IBWAIT _IOR( GPIB_CODE, 5, unsigned int )
#define IBRPP _IOWR( GPIB_CODE, 6, uint8_t )
#define IBAPE _IOW( GPIB_CODE, 7, int )
#define IBONL		5
#define IBSIC		6
#define IBSRE		7
#define IBGTS		8
#define IBCAC		9

#define IBSTATUS _IOR(GPIB_CODE, 10, int)

#define IBLINES		11
#define IBPAD		12
#define IBSAD		13
#define IBTMO _IOW( GPIB_CODE, 14, unsigned int )
#define IBEOS		16
#define IBRSV		17
#define IBRSP		_IOWR( GPIB_CODE, 18, serial_poll_ioctl_t )

#define DEVFCN		100

#define DVRSP		(DEVFCN+2)
#define DVRD		(DEVFCN+3)
#define DVWRT		(DEVFCN+4)

#define CFCIO           200             /* Special Functions.. */

#define CFCBASE         (CFCIO+0)
#define CFCIRQ          (CFCIO+1)
#define CFCDMA          (CFCIO+2)
#define CFCDMABUFFER	(CFCIO+3)

#define IBSDBG          (CFCIO+4)
#define CFCBOARDTYPE    (CFCIO+5)

#define APIO            300
#define IBAPWAIT        (APIO+0)
#define IBAPRSP         (APIO+1)

