
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

void internal_ibeot( ibConf_t *conf, int send_eoi )
{
	if(send_eoi)
		conf->send_eoi = 1;
	else
		conf->send_eoi = 0;
}

int ibeot( int ud, int send_eoi )
{
	ibConf_t *conf;

	conf = general_enter_library( ud, 1, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	internal_ibeot( conf, send_eoi );

	return exit_library( ud, 0 );
}
