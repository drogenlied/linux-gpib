#include <stdio.h>
#include "ibConfYacc.h"

int gpib_yyparse(void);
void gpib_yyrestart(FILE*);
int gpib_yylex(void);
YYLTYPE yylloc;

