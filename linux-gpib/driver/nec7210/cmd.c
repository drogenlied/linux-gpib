#include "board.h"

// XXX
IBLCL ssize_t nec7210_command(uint8_t *buffer, size_t length)
{
	size_t count = 0;

	// enable command out interrupt
	imr2_bits |= HR_COIE;
	GPIBout(IMR2, imr2_bits);

	while(count < length)
	{
		if(wait_event_interruptible(nec7210_command_wait, test_and_clear_bit(0, &command_out_ready)))
		{
			printk("gpib command wait interrupted\n");
			break;
		}
		GPIBout(CDOR, buffer[count]);
		count++;
	}

	// disable command out interrupt
	imr2_bits |= HR_COIE;
	GPIBout(IMR2, imr2_bits);

	if (!noTimo) {
		board.status |= (ERR | TIMO);
		iberr = EABO;
	}
	return count;
}









