
#include "ib_internal.h"
#include <ibP.h>

/*
 *-----------------------------------------------------------------
 * Function:  ibconfig
 * Purpose:   Change the software configuration input.
 * Input:     ud      - Board or device unit descriptor.
 *    	      option  - A parameter that selects the software
 *    	      	      	configuration item.
 *    	      value   - The value to shich the selected
 *    	      	      	configuration item is to be changed.
 * Output:    Returns the value of "ibsta".
 * Author:    Tobias Blomberg
 * Created:   1998-08-04
 * Remarks:   
 * Bugs:      
 *-----------------------------------------------------------------
 */
int ibconfig( int ud, int option, int value )
{

  ibsta = CMPL;
  return ibsta;	/* FIXME -- Implement me */
  
} /* ibconfig */

