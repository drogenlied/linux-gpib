
/* Unit descriptor flag */

#define FIND_CONFIGS_LENGTH 31	// max number of devices we can read from config file

extern ibBoard_t ibBoard[];
extern ibConf_t *ibConfigs[];
extern ibConf_t ibFindConfigs[FIND_CONFIGS_LENGTH];
extern ibarg_t ibarg;

#include <errno.h>
#include <fcntl.h>

#define CONF(a,b) (ibConfigs[a]->b)
#define BOARD(a)  (ibConfigs[a]->board)

#define MAX_BOARDS 16    /* maximal number of boards */
#define IB_MAXDEV 31    /* maximal number of devices */
#define NUM_CONFIGS 0x1000	// max number of device descriptors (length of ibConfigs array)

