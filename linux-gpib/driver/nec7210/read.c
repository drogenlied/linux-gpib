#include "board.h"
#include <asm/dma.h>

ssize_t nec7210_read(uint8_t *buffer, size_t length, uint8_t eos) // XXX eos broken
{
	size_t	count = 0;
	gpib_char_t data;
	int ret;
	unsigned long flags;

	if(length == 0) return 0;

	if (pgmstat & PS_HELD) {
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
/*
 *	holdoff on END
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE );

#if DMAOP		// ISA DMA transfer
	flags = claim_dma_lock();
	disable_dma(ibdma);

	/* program dma controller */
	clear_dma_ff ( ibdma );
	set_dma_count( ibdma, length );
	set_dma_addr ( ibdma, virt_to_bus(buffer));
	set_dma_mode( ibdma, DMA_MODE_READ );
	release_dma_lock(flags);

	enable_dma(ibdma);

	// enable nec7210 dma
	imr2_bits |= HR_DMAI;
	GPIBout(IMR2, imr2_bits);

	// wait for data to transfer
	wait_event_interruptible(nec7210_read_wait, test_and_clear_bit(0, &dma_transfer_complete));

	// disable nec7210 dma
	imr2_bits &= ~HR_DMAI;
	GPIBout(IMR2, imr2_bits);

	// record how many bytes we transferred
	flags = claim_dma_lock();
	clear_dma_ff ( ibdma );
	disable_dma(ibdma);
	count += length - get_dma_residue(ibdma);
	release_dma_lock(flags);

	set_bit(END_NUM, &ibsta);

#else	// PIO transfer

	// enable 'data in' interrupt
	imr1_bits |= HR_DIIE;
	GPIBout(IMR1, imr1_bits);

	while (count < length )
	{
		ret = gpib_buffer_get(read_buffer, &data);
		if(ret < 0)
		{
			if(wait_event_interruptible(nec7210_read_wait, atomic_read(&read_buffer->size) > 0))
			{
				printk("wait failed\n");
				// XXX
				break;
			};
			continue;
		}
		buffer[count++] = data.value;
		if(data.end)
		{
			set_bit(END_NUM, &ibsta);
			break;
		}
	}

	// disable 'data in' interrupt
	imr1_bits &= ~HR_DIIE;
	GPIBout(IMR1, imr1_bits);

#endif

	pgmstat |= PS_HELD;
	GPIBout(AUXMR, auxrabits | HR_HLDA);

	if (!noTimo)
	{
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}

	return count;
}








