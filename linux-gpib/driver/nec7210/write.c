#include "board.h"
#include <asm/dma.h>

/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */

IBLCL void bdDMAwrt(ibio_op_t *wrtop)
{
	bdPIOwrt(wrtop);
}

/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL void bdPIOwrt( ibio_op_t *wrtop)
{
	faddr_t		buf;
	unsigned	cnt;
	extern int eosmodes;
	uint8_t first_byte;
	gpib_char_t data;
	unsigned long flags;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;
	if(cnt == 0) return;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

	cnt-- ; /* save the last byte for sending EOI */

	// enable 'data out' interrupts
	imr1_bits |= HR_DOIE;
	GPIBout(IMR1, imr1_bits);

#if DMAOP	// isa dma transfer
	if(ibcnt < cnt)
	{
		flags = claim_dma_lock();
		disable_dma(ibdma);
		wrtop->io_pbuf = virt_to_bus(wrtop->io_vbuf);

		/* program dma controller */
		clear_dma_ff ( ibdma );
		// XXX what if io_cnt is too big?
		set_dma_count( ibdma, cnt );
		set_dma_addr ( ibdma, wrtop->io_pbuf);
		set_dma_mode( ibdma, DMA_MODE_WRITE );
		release_dma_lock(flags);
		ibcnt += cnt;
	}

#else	// PIO transfer

	data.end = 0;
	if(ibcnt < cnt)
	{
		// store first byte which we will have to send manually
		first_byte = buf[ibcnt];
		ibcnt++;
		// load rest of message into buffer
		while (ibcnt < cnt)
		{
			if(test_and_set_bit(0, &write_in_progress))
			{
				printk("gpib: bug? write already in progress");
				break;
			}
			data.value = buf[ibcnt];
			if(gpib_buffer_put(write_buffer, data))
			{
				printk("gpib: write buffer full!\n");
				// XXX
				break;
			}
			ibcnt++;
		}
		// send first byte and let interrupt handler do the rest
		GPIBout(CDOR, first_byte);
	}

#endif	// DMAOP

	// suspend until message is sent
	if(wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0))
	{
		printk("gpib write interrupted!\n");
		// XXX
	}

	DBGprint(DBG_BRANCH, ("send EOI  "));
        /*send EOI */

	if( eosmodes & XEOS )
	{
		DBGprint(DBG_BRANCH, ("send EOS with EOI  "));
		// XXX check for failure
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		GPIBout(CDOR, buf[ibcnt]);
		ibcnt++;
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		bdSendAuxCmd(AUX_SEOI);
		GPIBout(CDOR, bdGetEOS() );
	} else {
		DBGprint(DBG_BRANCH, ("send EOI with last byte "));
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		bdSendAuxCmd(AUX_SEOI);
		GPIBout(CDOR, buf[ibcnt]);
		ibcnt++;
	}
	wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);

	// disable 'data out' interrupts
	imr1_bits &= ~HR_DOIE;
	GPIBout(IMR1, imr1_bits);

	DBGprint(DBG_BRANCH, ("done  "));

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}

	DBGout();
}













