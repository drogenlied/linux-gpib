#include <ibprot.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
IBLCL int ibsre(int v)
{
	if( !(drvstat & DRV_REN || !v ) || (ib_opened <= 1) )
	{
		pgmstat |= PS_SAC;
		board.remote_enable(v);	/* set or clear REN */
		if( !v ) drvstat &= ~DRV_REN;
		else drvstat |= DRV_REN;
	}

	return 0;
}

