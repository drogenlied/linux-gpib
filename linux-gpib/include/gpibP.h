
#include <autoconf.h>
#include <gpib_board.h>


#include <gpib_config.h>
#include <gpib_debug.h>
#include <gpib_types.h>

#include <gpib_proto.h>




#include <gpib_registers.h>
#include <gpib_ioctl.h>
#include <gpib_user.h>

/* this are wrappers for the DMA/PIO functions */

#if DMAOP

#define bdread   bdDMAread
#define bdwrt    bdDMAwrt
#define bdAdjCnt bdDMAAdjCnt
#define osAdjCnt osDMAAdjCnt

#else

#define bdread   bdPIOread
#define bdwrt    bdPIOwrt
#define bdAdjCnt bdPIOAdjCnt
#define osAdjCnt osPIOAdjCnt

#endif

