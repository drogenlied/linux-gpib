
%{

#include "y.tab.h"
%}

%x COMMENT

FNAME [A-Za-z/_+-][A-Za-z0-9/_():.+-]*
NETD  [A-Za-z*_+-][A-Za-z0-9*_():.+-]*
%%

"/*"		 BEGIN(COMMENT);
<COMMENT>[^*]*
<COMMENT>"*"+[^*/]*
<COMMENT>"*"+"/" BEGIN(INITIAL);

config 	     { return (T_CONFIG);}

base         { return (T_BASE);}
irq          { return (T_IRQ);}
dma          { return (T_DMA);}

device	     { return(T_DEVICE);}

name         { return (T_NAME);}

pad          { return(T_PAD);}
sad          { return(T_SAD);}
eos          { return(T_EOSBYTE);}
timeout      |
timo	     { return(T_TIMO);}

set-reos      { return T_REOS;}
set-bin       { return T_BIN; }
set-ifc       { return T_IFC; }


init-string   { return T_INIT_S; }
init-flags    { return T_INIT_F; }
set-clear     { printf ("DCL not longer supported") ;return T_DCL;  }

LLO           { return T_LLO; }
DCL           { return T_DCL; }
EXCL          { return T_EXCL;}

controller    |
master        { return T_MASTER; }

autopoll      { return T_AUTOPOLL; }

debug         { return T_DEBUG; }

dma-bufsize  { return T_DMABUF; }    

network       { return T_NETWORK; }
server                 { return T_SERVER; }

yes         { gpib_yylval.bval=1; return(T_BOOL);}
no          { gpib_yylval.bval=0; return(T_BOOL);}


none         { gpib_yylval.ival=0 ; return(T_TIVAL);}
10us         { gpib_yylval.ival=1 ; return(T_TIVAL);}
30us         { gpib_yylval.ival=2 ; return(T_TIVAL);}
100us        { gpib_yylval.ival=3 ; return(T_TIVAL);}
300us        { gpib_yylval.ival=4 ; return(T_TIVAL);}
1ms          { gpib_yylval.ival=5 ; return(T_TIVAL);}
3ms          { gpib_yylval.ival=6 ; return(T_TIVAL);}
10ms         { gpib_yylval.ival=7 ; return(T_TIVAL);}
30ms         { gpib_yylval.ival=8 ; return(T_TIVAL);}
100ms        { gpib_yylval.ival=9 ; return(T_TIVAL);}
300ms        { gpib_yylval.ival=10; return(T_TIVAL);}
1s           { gpib_yylval.ival=11; return(T_TIVAL);}
3s           { gpib_yylval.ival=12; return(T_TIVAL);}
10s          { gpib_yylval.ival=13; return(T_TIVAL);}
30s          { gpib_yylval.ival=14; return(T_TIVAL);}
100s         { gpib_yylval.ival=15; return(T_TIVAL);}
300s         { gpib_yylval.ival=16; return(T_TIVAL);}
1000s        { gpib_yylval.ival=17; return(T_TIVAL);}

errlog       { return T_ERRLOG; }




[0-9]+  { sscanf(gpib_yytext,"%d",&gpib_yylval.ival); 
	  return( T_NUMBER );}
0x[0-9a-fA-F]+ { sscanf(gpib_yytext,"0x%x",&gpib_yylval.ival);
	  return( T_NUMBER );}


{FNAME}|{NETD} { gpib_yylval.sval=gpib_yytext; /*printf("<%s>\n",gpib_yytext);*/
		       return (T_STRING);}




[ \t\n]

.	{ return(gpib_yytext[0]);}


%%












