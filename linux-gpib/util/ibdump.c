

#include <stdio.h>
#include <ib.h>
#include <stdlib.h>

#define DEFAULT_CONFIG_FILE "/etc/gpib.conf"

extern int ibDumpConfiguration(int format,char *filename);

void set_defaults(){
}

int main(int argc, char **argv){
        char *envptr;
	int format = IB_DUMP_CONFIG;
        

        /* first parse configuration */

        if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 ){
	  if(ibParseConfigFile(DEFAULT_CONFIG_FILE) < 0  ) {
	    fprintf(stderr,"Can't open Config File %s\n",DEFAULT_CONFIG_FILE);
	    set_defaults();
	  }
	}
        else{
	  if(ibParseConfigFile(envptr) < 0) {
	    fprintf(stderr,"Can't open Config File %s\n",envptr);
	    set_defaults();
	  }
	}




        if ( argc == 2 ){
	  if( ! strcmp(argv[1],"mkdev") ) format = IB_DUMP_MKDEV;
	  if( ! strcmp(argv[1],"config") ) format = IB_DUMP_CONFIG;
	  if( ! strcmp(argv[1],"tcl") ) format = IB_DUMP_TCL;
	}
	ibDumpConfiguration( format,   NULL);

	return 0;
}

