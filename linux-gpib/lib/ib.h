#include <autoconf.h>

#include <gpib_types.h>
#include <gpib_registers.h>
#include <gpib_ioctl.h>
#include <gpib_user.h>

#include <ibConf.h>

extern int ibsta,ibcnt,iberr;

typedef int Addr4882_t;
#define NOADDR -1
#define IbcAUTOPOLL 0 
typedef void (*ibEventHandler)(int); 

#include <ibProto.h>

#define IB_DUMP_CONFIG 0
#define IB_DUMP_MKDEV  1
#define IB_DUMP_TCL    2

#ifndef NULL
#define NULL 0
#endif

#include <stdio.h>
int gpib_yyparse(void);
void gpib_yyrestart(FILE*);
int gpib_yylex(void);
