
/* Unit descriptor flag */

#define UD_REMOTE  (1<<16)            /* UD is a network handle */

 
extern ibBoard_t ibBoard[];
extern ibConf_t  ibConfigs[];
extern ibarg_t ibarg;

#include <errno.h>
#include <fcntl.h>

#define CONF(a,b) (((a) & UD_REMOTE) ? (a) : ibConfigs[a].b )
#define BOARD(a)  (ibConfigs[a].board)

#define MAX_BOARDS 16    /* maximal number of boards */
#define IB_MAXDEV 30    /* maximal number of devices */


#define PRIVATE   /* private functions */
#define PUBLIC    /* functions for prototyping */
#define _VARARGS  /* varargs prototyping */
#define VOID void

