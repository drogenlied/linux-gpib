
#include <ib.h>
#include <ibP.h>


/*
 *-----------------------------------------------------------------
 * Function:  ibdma
 * Purpose:   Enable or disable DMA.
 * Input:     ud  - A board descriptor.
 *    	      v   - Enable or disable the use of DMA.
 * Output:    Returns the value of "ibsta".
 * Author:    Tobias Blomberg
 * Created:   1998-08-04
 * Remarks:   Does not do anything at all right now...
 * Bugs:      
 *-----------------------------------------------------------------
 */
int ibdma( int ud, int v )
{
  if (ud < 0) {
    ibsta = CMPL | ERR;
    iberr = EDVR;
    return ibsta;
  }
  
  ibsta = CMPL;
  return ibsta;
} /* ibdma */
