#include "board.h"
#include <asm/dma.h>

/*
 *  BDREAD (DMA)
 *  This function performs a single DMA read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */

IBLCL void bdDMAread(ibio_op_t *rdop)
{
	bdPIOread(rdop);
}

/*
 *  BDREAD (PIO)
 *  This function performs a single Programmed I/O read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */
IBLCL void bdPIOread(ibio_op_t *rdop)
{
	faddr_t		buf;
	unsigned	cnt;
	gpib_char_t data;
	int ret;
	unsigned long flags;

	DBGin("bdread");

	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	if(cnt == 0) return;
	DBGprint(DBG_DATA, ("bdread: buf=0x%p cnt=%d  ", buf, cnt));
	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE );

#if DMAOP		// ISA DMA transfer
	flags = claim_dma_lock();
	disable_dma(ibdma);
	rdop->io_pbuf = virt_to_bus(rdop->io_vbuf);

	/* program dma controller */
	clear_dma_ff ( ibdma );
	// XXX what if io_cnt is too big?
	set_dma_count( ibdma, rdop->io_cnt );
	set_dma_addr ( ibdma, rdop->io_pbuf);
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
	ibcnt += rdop->io_cnt - get_dma_residue(ibdma);
	release_dma_lock(flags);

	set_bit(END_NUM, &ibsta);

#else	// PIO transfer

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

	// enable 'data in' interrupt
	imr1_bits |= HR_DIIE;
	GPIBout(IMR1, imr1_bits);

	while (ibcnt < cnt )
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
		buf[ibcnt++] = data.value;
		if(data.end)
		{
			set_bit(END_NUM, &ibsta);
			break;
		}
	}

	// disable 'data in' interrupt
	imr1_bits &= ~HR_DIIE;
	GPIBout(IMR1, imr1_bits);

	DBGprint(DBG_BRANCH, ("done  "));
#endif

	pgmstat |= PS_HELD;
	GPIBout(AUXMR, auxrabits | HR_HLDA);

	if (!noTimo)
	{
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}








