%{
#include <stdio.h>
#include <ib.h>
#undef EXTERN
#include <ibP.h>
#include <string.h>
#include <stdlib.h>

#define YYERROR_VERBOSE

static ibConf_t  temp;

ibConf_t  ibConfigs[IB_MAXDEV];
int bdid = 0;
%}

%union
{
int  ival;
char *sval;
char bval;
char cval;
}

%token T_INTERFACE T_DEVICE T_NAME T_MINOR T_BASE T_IRQ T_DMA 
%token T_PAD T_SAD T_TIMO T_EOSBYTE 
%token T_REOS T_BIN T_INIT_S T_DCL T_IFC
%token T_MASTER T_LLO T_DCL T_EXCL T_INIT_F T_AUTOPOLL

%token T_NUMBER T_STRING T_BOOL T_TIVAL
%type <ival> T_NUMBER
%type <ival> T_TIVAL
%type <sval> T_STRING
%type <bval> T_BOOL

%%

	input: /* empty */
		| device input
		| interface input
		| error
 			{	
 				fprintf(stderr, "input error on line %i of %s\n", @1.first_line, DEFAULT_CONFIG_FILE);
				return -1;
			}
		;

	interface: T_INTERFACE '{' minor parameter '}'
		;

	minor : T_MINOR '=' T_NUMBER {
				bdid = $3;
				if(bdid < MAX_BOARDS)  
					sprintf(ibBoard[bdid].device,"/dev/gpib%i", bdid);
				else
					return -1;
			}
		;

	parameter: /* empty */ 
		| statement parameter
		| error
 			{	
 				fprintf(stderr, "parameter error on line %i of %s\n", @1.first_line, DEFAULT_CONFIG_FILE);
				return -1;
			}
		;

	statement: T_PAD '=' T_NUMBER      { ibBoard[bdid].padsad |=  $3; }
		| T_SAD '=' T_NUMBER      { ibBoard[bdid].padsad |= ($3<<8); }
                 | T_EOSBYTE '=' T_NUMBER  { ibBoard[bdid].eos = $3; }
		| T_REOS T_BOOL           { ibBoard[bdid].eosflags |= $2 * REOS;}
                 | T_BIN  T_BOOL           { ibBoard[bdid].eosflags |= $2 * BIN; }
		| T_IFC  T_BOOL           { ibBoard[bdid].ifc = $2 ; }
		| T_TIMO '=' T_TIVAL      { ibBoard[bdid].timeout = $3; }
		| T_BASE '=' T_NUMBER     { ibBoard[bdid].base = $3; }
		| T_IRQ  '=' T_NUMBER     { ibBoard[bdid].irq = $3; }
		| T_DMA  '=' T_NUMBER     { ibBoard[bdid].dma = $3; }
        	| T_NAME '=' T_STRING	{ strncpy(ibBoard[bdid].name,$3,30);}
		;

	device: T_DEVICE '{' option '}'
			{
				if(ibInstallConfigItem(&temp) < 0)
				{
					fprintf(stderr, "ibInstallConfigItem() failed\n");
					return -1;
				}
				// reinit temp
				temp.padsad=0;
				temp.board=0;
				temp.init_string[0]='\0';
				temp.eos=0;
				temp.eosflags=0;
				temp.flags = 0;
				temp.networkdb = NULL;
			}
		;

        option: /* empty */
	        | assign option 
		| error 
 			{	
 				fprintf(stderr, "option error on line %i of %s\n", @1.first_line, DEFAULT_CONFIG_FILE);
				return -1;
			}
		;

	assign:
		T_PAD '=' T_NUMBER { temp.padsad  |= $3; }
                | T_SAD '=' T_NUMBER { temp.padsad |= ($3<<8); }
		| T_INIT_S '=' T_STRING { strncpy(temp.init_string,$3,60); }
                | T_EOSBYTE '=' T_NUMBER  { temp.eos = $3; }
		| T_REOS T_BOOL           { temp.eosflags |= $2 * REOS;}
                | T_BIN  T_BOOL           { temp.eosflags |= $2 * BIN; }
                | T_MASTER                { temp.flags |= CN_ISCNTL; }
                | T_AUTOPOLL              { temp.flags |= CN_AUTOPOLL; }
		| T_INIT_F '=' flags
        	| T_NAME '=' T_STRING	{ strncpy(temp.name,$3,30);}
		| T_MINOR '=' T_NUMBER	{ temp.board = $3;} 
		;
		
	flags: /* empty */
		| ',' flags 
	        | oneflag flags
		;

	oneflag: T_LLO       { temp.flags |= CN_SLLO; }
		| T_DCL       { temp.flags |= CN_SDCL; }
		| T_EXCL      { temp.flags |= CN_EXCLUSIVE; }
		;

%%



PRIVATE void yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
}



