
#include <ib.h>
#include <ibP.h>

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

FILE* errp=NULL;


PRIVATE void ibOpenErrlog(char *name)
{
  
  if(( errp=fopen(name,"w"))==NULL){
    fprintf(stderr,"Warning: can't open Errlog: %s",name);
    errp = stderr;
  }
  /*fprintf(stderr,"open %s",name);*/
  ibPutErrlog(-1,"IbOpenErrlog");
}


PRIVATE void ibCloseErrlog(void)
{
  if( errp != stderr )
    fclose(errp);
}


char *routptr[]= {
    "ibrd",
    "ibwrt",
    "ibcmd",
    "ibwait",
    "ibrpp",
    "ibonl",
    "ibsic",
    "ibsre",
    "ibgts",
    "ibcac",
    "ibpoke",
    "iblines",
    "ibpad",
    "ibsad",
    "ibtmo",
    "ibeot",
    "ibeos",
    "ibrsv",  /*17*/

    "dvtrg",
    "dvclr",
    "dvrsp",
    "dvrd",
    "dvwrt", /*22*/

    "cfcbase",
    "cfcirq",
    "cfcdma", /*25*/

    "unknown"
  };



PRIVATE char *ibVerbCode(int code)
{

  if( code >=0 && code <=17 )    return routptr[code];
  else if( code >=100 && code <=104 ) return routptr[18+(code-100)];
  else if( code >=200 && code <=202 ) return routptr[23+(code-200)];
  else return routptr[26];
}

#if 0
int ibPutMsg(char *msg)
{

  time_t tm;
  char strtime[60];

    time(&tm);
    strftime(strtime,59,"%c",gmtime(&tm));

    fprintf(errp,"gpib-message   :[%s] %s \n",strtime,msg);

}
#endif

PRIVATE void ibPutMsg (char *format,...) 
{

va_list ap;

  time_t tm;
  char strtime[60];


    va_start(ap,format);
    time(&tm);
    strftime(strtime,59,"%c",gmtime(&tm));

    fprintf(errp,"\n gpib-message   :[%s] ",strtime);
    vfprintf(errp,format, ap);
    va_end(ap);

}


PRIVATE void ibPutErrlog(int ud,char *routine)
{

time_t tm;
char strtime[60];

    if( ibsta & ERR ) {
    time(&tm);
    strftime(strtime,59,"%c",gmtime(&tm));

    if(ud>=0) fprintf(errp,"\n %-15s:[%s](%s)< ",routine,strtime,CONF(ud,name));
    else      fprintf(errp,"\n %-15s:[%s](-)< " ,routine,strtime);

    if ( ibsta & ERR )  fprintf(errp," ERR");
    if ( ibsta & TIMO ) fprintf(errp," TIMO");
    if ( ibsta & END )  fprintf(errp," END");
    if ( ibsta & SRQI ) fprintf(errp," SRQI");
    /*if ( ibsta & RQS )  fprintf(errp," RQS");*/
    if ( ibsta & CMPL ) fprintf(errp," CMPL");
    /*if ( ibsta & LOK )  fprintf(errp," LOK");*/
    /*if ( ibsta & REM )  fprintf(errp," REM");*/
    if ( ibsta & CIC )  fprintf(errp," CIC");
    if ( ibsta & ATN )  fprintf(errp," ATM");
    if ( ibsta & TACS ) fprintf(errp," TACS");
    if ( ibsta & LACS ) fprintf(errp," LACS");
    /*if ( ibsta & DTAS ) fprintf(errp," DATS");*/
    /*if ( ibsta & DCAS ) fprintf(errp," DCTS");*/

    fprintf(errp,"> ");

    if ( iberr == EDVR) fprintf(errp," EDVR <OS Error>\n");
    if ( iberr == ECIC) fprintf(errp," ECIC <Not CIC>\n");
    if ( iberr == ENOL) fprintf(errp," ENOL <No Listener>\n");
    if ( iberr == EADR) fprintf(errp," EADR <Adress Error>\n");
    if ( iberr == EARG) fprintf(errp," ECIC <Invalid Argument>\n");
    if ( iberr == ESAC) fprintf(errp," ESAC <No Sys Ctrlr>\n");
    if ( iberr == EABO) fprintf(errp," EABO <Operation Aborted>\n");
    if ( iberr == ENEB) fprintf(errp," ENEB <No Gpib Board>\n");
    if ( iberr == EOIP) fprintf(errp," EOIP <Async I/O in prg>\n");
    if ( iberr == ECAP) fprintf(errp," ECAP <No Capability>\n");
    if ( iberr == EFSO) fprintf(errp," EFSO <File sys. error>\n");
    if ( iberr == EBUS) fprintf(errp," EBUS <Command error>\n");
    if ( iberr == ESTB) fprintf(errp," ESTB <Status byte lost>\n");
    if ( iberr == ESRQ) fprintf(errp," ESRQ <SRQ stuck on>\n");
    if ( iberr == ETAB) fprintf(errp," ETAB <Table Overflow>\n");
    if ( iberr == EPAR) fprintf(errp," EPAR <Parse Error in Config>\n");
    if ( iberr == ECFG) fprintf(errp," ECFG <Can't open Config>\n");
    if ( iberr == ETAB) fprintf(errp," ETAB <Device Table Overflow>\n");
    if ( iberr == ENSD) fprintf(errp," ENSD <Configuration Error>\n");
    if ( iberr == ENWE) fprintf(errp," ENWE <Network Errror>\n");
    if ( iberr == ENTF) fprintf(errp," ENTF <Network Table Overflow>\n");

    if ( iberr == EDVR && ibcnt != 0) {
      fprintf(errp,"               -- errno=%d (%s)\n",ibcnt,strerror(ibcnt));
    }
    fflush(errp);
  }
}






