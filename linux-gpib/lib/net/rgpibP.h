

#define MAXHANDLES 50

typedef struct ibRemoteHandle {
  int r_ud;                    /* remote unit descriptor */
  CLIENT *r_client;            /* client stub */
} r_handle_t ;

