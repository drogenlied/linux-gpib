#include <autoconf.h>

#include "ib.h"
#include <gpib_types.h>
#include <gpib_registers.h>
#include <gpib_ioctl.h>
#include <unistd.h>

#include <ibConf.h>

#define NOADDR -1
#define IbcAUTOPOLL 0

/***** Private Functions ******/
extern int ibCheckDescriptor(int ud);
extern  int ibBdChrConfig(int ud);
extern  void ibBoardDefaultValues(void);
extern  int ibBoardOpen(int bd,int flags);
extern  int ibBoardClose(int bd);
extern  int ibBoardFunc (int bd, int code, ...);
extern  int ibGetNrBoards(void);
extern  void yyerror(char *s);
extern  int ibeos(int ud, int v);
extern  int iblcleos(int ud);
extern  char *ibVerbCode(int code);
extern  void ibPutMsg (char *format,...);
extern  void ibPutErrlog(int ud,char *routine);
extern  int ibParseConfigFile(char *filename);
extern  int ibGetDescriptor(ibConf_t conf);
extern  int ibFindDevIndex(char *name);
extern ssize_t __ibcmd(ibBoard_t *board, uint8_t *buffer, size_t length);
extern int __ibtmo(ibBoard_t *board, int timeout);
extern int config_parsed;
extern int send_setup(ibBoard_t *board, int pad, int sad);
extern ssize_t device_command(ibBoard_t *board, uint8_t *cmd, size_t length, int pad, int sad);
extern void init_ibconf( ibConf_t *conf );

#include <stdio.h>
int gpib_yyparse(void);
void gpib_yyrestart(FILE*);
int gpib_yylex(void);

