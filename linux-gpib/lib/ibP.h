
/* Unit descriptor flag */

#define FIND_CONFIGS_LENGTH 64	// max number of devices we can read from config file

extern ibBoard_t ibBoard[];
extern ibConf_t *ibConfigs[];
extern ibConf_t ibFindConfigs[ FIND_CONFIGS_LENGTH ];

#include <errno.h>
#include <fcntl.h>

#define CONF(a,b) (ibConfigs[a]->b)
#define BOARD(a)  (ibConfigs[a]->board)

#define MAX_BOARDS 16    /* maximal number of boards */
#define IB_MAXDEV 31    /* maximal number of devices */
#define NUM_CONFIGS 0x1000	// max number of device descriptors (length of ibConfigs array)

static const int sad_offset = 0x60;
static const int gpib_addr_max = 30;	// max address for primary/secondary gpib addresses

// deal with stupid pad/sad packing scheme
extern inline int padsad(int pad, int sad)
{
	int padsad = pad & 0xff;
	if(sad >= 0 && sad <= gpib_addr_max )
		padsad |= (sad + sad_offset);
	return padsad;
}

