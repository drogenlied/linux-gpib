
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

int ibeot(int ud, int send_eoi)
{
	ibConf_t *conf;

	conf = enter_library( ud, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	if(send_eoi)
		conf->send_eoi = 1;
	else
		conf->send_eoi = 0;

	return exit_library( ud, 0 );
}
