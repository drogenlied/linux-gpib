#include "board.h"

// XXX
IBLCL ssize_t nec7210_command(uint8_t *buffer, size_t length)
{
	size_t count = 0;

	while(count < length)
	{
		GPIBout(CDOR, buffer[count]);
//		bdWaitOut(); XXX
		count++;
	}

	GPIBout(AUXMR, AUX_GTS);	/* go to standby */

	if (!noTimo) {
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	return count;
}









