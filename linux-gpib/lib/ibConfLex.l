%option noyywrap

%{

#include "ibConfYacc.h"
YYLTYPE yylloc;

%}

%x COMMENT

FNAME [A-Za-z/_+-][A-Za-z0-9/_():.+-]*
NETD  [A-Za-z*_+-][A-Za-z0-9*_():.+-]*
%%

<COMMENT,INITIAL>\n {yylloc.first_line++;}

"/*"		 BEGIN(COMMENT);
<COMMENT>[^*\n]*
<COMMENT>"*"+[^*/i\n]*
<COMMENT>"*"+"/" BEGIN(INITIAL);

interface 	     { return (T_INTERFACE);}

base         { return (T_BASE);}
irq          { return (T_IRQ);}
dma          { return (T_DMA);}
pci_bus      { return (T_PCI_BUS);}
pci_slot      { return (T_PCI_SLOT);}

device	     { return(T_DEVICE);}

name         { return (T_NAME);}
board_type         { return (T_BOARD_TYPE);}
minor         { return (T_MINOR);}

pad          { return(T_PAD);}
sad          { return(T_SAD);}
eos          { return(T_EOSBYTE);}
timeout      |
timo	     { return(T_TIMO);}

set-reos      { return T_REOS;}
set-bin       { return T_BIN; }

init-string   { return T_INIT_S; }
init-flags    { return T_INIT_F; }
set-clear     { printf ("DCL not longer supported") ;return T_DCL;  }

LLO           { return T_LLO; }
DCL           { return T_DCL; }
EXCL          { return T_EXCL;}

controller    |
master        { return T_MASTER; }

autopoll      { return T_AUTOPOLL; }

yes         { gpib_yylval.bval=1; return(T_BOOL);}
no          { gpib_yylval.bval=0; return(T_BOOL);}


none         { gpib_yylval.ival=0 ; return(T_TIVAL);}
10us         { gpib_yylval.ival=10; return(T_TIVAL);}
30us         { gpib_yylval.ival=30; return(T_TIVAL);}
100us        { gpib_yylval.ival=100; return(T_TIVAL);}
300us        { gpib_yylval.ival=300; return(T_TIVAL);}
1ms          { gpib_yylval.ival=1000; return(T_TIVAL);}
3ms          { gpib_yylval.ival=3000; return(T_TIVAL);}
10ms         { gpib_yylval.ival=10000; return(T_TIVAL);}
30ms         { gpib_yylval.ival=30000; return(T_TIVAL);}
100ms        { gpib_yylval.ival=100000; return(T_TIVAL);}
300ms        { gpib_yylval.ival=300000; return(T_TIVAL);}
1s           { gpib_yylval.ival=1000000; return(T_TIVAL);}
3s           { gpib_yylval.ival=3000000; return(T_TIVAL);}
10s          { gpib_yylval.ival=1000000; return(T_TIVAL);}
30s          { gpib_yylval.ival=30000000; return(T_TIVAL);}
100s         { gpib_yylval.ival=100000000; return(T_TIVAL);}
300s         { gpib_yylval.ival=300000000; return(T_TIVAL);}
1000s        { gpib_yylval.ival=1000000000; return(T_TIVAL);}


[0-9]+  { sscanf(gpib_yytext,"%d",&gpib_yylval.ival); 
	  return( T_NUMBER );}
0x[0-9a-fA-F]+ { sscanf(gpib_yytext,"0x%x",&gpib_yylval.ival);
	  return( T_NUMBER );}

{FNAME}|{NETD} { gpib_yylval.sval=gpib_yytext; /*printf("<%s>\n",gpib_yytext);*/
		       return (T_STRING);}

[ \t]

.	{ return(gpib_yytext[0]);}


%%

