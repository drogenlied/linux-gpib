#define DEBUG_SERVER 1
#if DEBUG_SERVER
#define DB(a) if(debug) a
#else
#define DB(a)
#endif

#define IGNORE_LOCAL 1

struct gpib_request {
	int request;
	int handle;
	int count;
	int arg;
	int ret;
	int ibsta;
	int iberr;
	int ibcnt;
	char *client;
	char *buf;
};

typedef struct gpib_request gpib_request;
bool_t xdr_gpib_request();
void init_gpib_request();

#define RGPIBPROG ((u_long)0x20001234)
#define RGPIBVERS ((u_long)1)
#define RGPIB_NULL ((u_long)0)
extern void *rgpib_null_1();
#define RGPIB_GETHANDLE ((u_long)1)
extern int *rgpib_gethandle_1();
#define RGPIB_DOREQUEST ((u_long)2)
extern gpib_request *rgpib_dorequest_1();

extern int debug;


