%{
#include <stdio.h>
#include <ib.h>
#undef EXTERN
#include <ibP.h>

static ibConf_t  temp;

ibConf_t  ibConfigs[IB_MAXDEV];


int bdid=-1;
%}

%union
{
int  ival;
char *sval;
char bval;
char cval;
}

%token T_CONFIG T_DEVICE T_NAME T_BASE T_IRQ T_DMA T_DMABUF
%token T_PAD T_SAD T_TIMO T_EOSBYTE T_DEBUG
%token T_REOS T_XEOS T_BIN T_ERRLOG T_INIT_S T_DCL T_IFC
%token T_MASTER T_LLO T_DCL T_EXCL T_INIT_F T_NETWORK T_SERVER T_AUTOPOLL 

%token T_NUMBER T_STRING T_BOOL T_TIVAL
%type <ival> T_NUMBER
%type <ival> T_TIVAL
%type <sval> T_STRING
%type <bval> T_BOOL

%%
	line:      /* empty */
	    | line configure 
            | line device { if(ibInstallConfigItem(&temp) < 0) return(-1); } 
	    | error       { return -1; }


	configure: preamble definition 
	
	preamble: T_CONFIG { if(bdid < MAX_BOARDS) bdid++;  
#if 0
	                     sprintf(ibBoard[bdid].device,"/dev/gpib%d",bdid) ; 
#else
                             sprintf(ibBoard[bdid].device,"/dev/gpib%d/master",bdid) ; 
#endif
                           }

	definition:  '{' parameter '}'

	parameter: statement
		 | parameter statement
	
        statement: T_PAD '=' T_NUMBER      { ibBoard[bdid].padsad |=  $3; }
		 | T_SAD '=' T_NUMBER      { ibBoard[bdid].padsad |= ($3<<8); }
                 | T_EOSBYTE '=' T_NUMBER  { ibBoard[bdid].eos = $3; }
		 | T_REOS T_BOOL           { ibBoard[bdid].eosflags |= $2 * REOS;}
	         | T_XEOS T_BOOL           { ibBoard[bdid].eosflags |= $2 * XEOS;}
                 | T_BIN  T_BOOL           { ibBoard[bdid].eosflags |= $2 * BIN; }
		 | T_IFC  T_BOOL           { ibBoard[bdid].ifc = $2 ; }
		 | T_TIMO '=' T_TIVAL      { ibBoard[bdid].timeout = $3; }
		 | T_BASE '=' T_NUMBER     { ibBoard[bdid].base = $3; }
		 | T_IRQ  '=' T_NUMBER     { ibBoard[bdid].irq = $3; }
		 | T_DMA  '=' T_NUMBER     { ibBoard[bdid].dma = $3; }
                 | T_DEBUG '=' T_NUMBER    { ibBoard[bdid].debug = $3; }
                 | T_DMABUF '=' T_NUMBER   { ibBoard[bdid].dmabuf = $3; }
		 | T_ERRLOG '=' T_STRING   { strncpy(ibBoard[bdid].errlog,$3,60);} 
		 |  error {return -1; } 

	device: T_DEVICE '{' arg '}' 

	arg: name
             | name option
	     | error { return -1;}

        name: T_NAME '=' T_STRING { strncpy(temp.name,$3,30); 
			            temp.padsad=0; 
				    temp.board=bdid; 
				    temp.init_string[0]='\0';
				    /*temp.dcl=0;*/
                                    temp.eos=0;
                                    temp.eosflags=0;
			            temp.flags = 0;
				    temp.networkdb = NULL;
                                  }
	     
        option: assign      
	        | option assign 

	assign:  
		T_PAD '=' T_NUMBER { temp.padsad  |= $3; }
                |  T_SAD '=' T_NUMBER { temp.padsad |= ($3<<8); }
		|  T_INIT_S '=' T_STRING { strncpy(temp.init_string,$3,60); }
		|  T_NETWORK T_STRING { temp.networkdb = malloc(strlen($2)+1 );
				        strcpy(temp.networkdb,$2); }
		/*|  T_DCL T_BOOL { temp.dcl=$2; }*/ 
                |  T_EOSBYTE '=' T_NUMBER  { temp.eos = $3; }
		|  T_REOS T_BOOL           { temp.eosflags |= $2 * REOS;}
	        |  T_XEOS T_BOOL           { temp.eosflags |= $2 * XEOS;}
                |  T_BIN  T_BOOL           { temp.eosflags |= $2 * BIN; }
                |  T_MASTER                { temp.flags |= CN_ISCNTL; }
                |  T_AUTOPOLL              { temp.flags |= CN_AUTOPOLL; }
		|  T_INIT_F '=' flags
		|  error {return -1;}
			
	flags: oneflag 
		| oneflag ',' flags
	        | oneflag flags
		| error {return -1;}

	oneflag:
		| T_LLO       { temp.flags |= CN_SLLO; }
		| T_DCL       { temp.flags |= CN_SDCL; }
		| T_EXCL      { temp.flags |= CN_EXCLUSIVE; }
	        | error  {return -1;}

%%



PRIVATE void gpib_yyerror(char *s)
{
	ibsta |= ERR;
	iberr = EPAR;
}



