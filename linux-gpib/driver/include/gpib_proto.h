
#ifndef GPIB_PROTO_INCLUDED
#define GPIB_PROTO_INCLUDED

#include <board_proto.h>
#include <sys_proto.h>
#include <protocol_proto.h>
#include <linux/kernel.h>

// board-specific initialization
int board_attach(void);
// board-specific release of resources
void board_detach(void);

#endif /* GPIB_PROTO_INCLUDED */
