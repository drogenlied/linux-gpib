#include <board.h>
/*
 * BDLINES
 * Poll the GPIB control lines and return their status in buf.
 *
 *      LSB (bits 0-7)  -  VALID lines mask (lines that can be monitored).
 * Next LSB (bits 8-15) - STATUS lines mask (lines that are currently set).
 *
 */
IBLCL int bdlines(void)
{
	int	bus;			/* bus control line status */
	int	state;			/* current bus state vector */

	DBGin("bdlines");
	bus = GPIBin(gpibc);
	DBGprint(DBG_DATA, ("gpibc=0x%x  ", bus));
/*
 *	On the AT-GPIB, all control lines are valid and readable at all
 *	times using the 'gpibc' control register.
 *
 *	+=====+=====+===================================================+
 *	| SAC | CIC | Lines that can be monitored...                    |
 *	+=====+=====+===================================================+
 *	|     |     |                                                   |
 *	|     |     | EOI   ATN   SRQ   REN   IFC   NRFD   NDAC   DAV   |
 *	+-----+-----+---------------------------------------------------+
 *	|     |     |                                                   |
 *	| yes |     | EOI   ATN   SRQ   REN   IFC   NRFD   NDAC   DAV   |
 *	+-----+-----+---------------------------------------------------+
 *	|     |     |                                                   |
 *	|     | yes | EOI   ATN   SRQ   REN   IFC   NRFD   NDAC   DAV   |
 *	+-----+-----+---------------------------------------------------+
 *	|     |     |                                                   |
 *	| yes | yes | EOI   ATN   SRQ   REN   IFC   NRFD   NDAC   DAV   |
 *	+=====+=====+===================================================+
 */
	state = (BUS_EOI | BUS_ATN | BUS_SRQ | BUS_REN | BUS_IFC | BUS_NRFD | BUS_NDAC | BUS_DAV) >> 8;

	if (bus & CR_EOI)
		state |= BUS_EOI;
	if (bus & CR_ATN)
 		state |= BUS_ATN;
	if (bus & CR_SRQ)
		state |= BUS_SRQ;
	if (bus & CR_REN)
		state |= BUS_REN;
	if (bus & CR_IFC)
		state |= BUS_IFC;
	if (bus & CR_DAV)
		state |= BUS_DAV;
	if (bus & CR_NRFD)
		state |= BUS_NRFD;
	if (bus & CR_NDAC)
		state |= BUS_NDAC;

	DBGprint(DBG_DATA, ("state=0x%x  ", state));
	DBGout();
	return state;
}



